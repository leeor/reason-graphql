type tokenKind =
  | SOF
  | EOF
  | BANG
  | DOLLAR
  | AMP
  | PAREN_L
  | PAREN_R
  | SPREAD
  | COLON
  | EQUALS
  | AT
  | BRACKET_L
  | BRACKET_R
  | BRACE_L
  | PIPE
  | BRACE_R
  | NAME
  | INT
  | FLOAT
  | STRING
  | COMMENT;

let strOfTokenKind =
  fun
  | SOF => "<SOF>"
  | EOF => "<EOF>"
  | BANG => "!"
  | DOLLAR => "$"
  | AMP => "$"
  | PAREN_L => "("
  | PAREN_R => ")"
  | SPREAD => "..."
  | COLON => ":"
  | EQUALS => "="
  | AT => "@"
  | BRACKET_L => "["
  | BRACKET_R => "]"
  | BRACE_L => "{"
  | PIPE => "|"
  | BRACE_R => "}"
  | NAME => "Name"
  | INT => "Int"
  | FLOAT => "Float"
  | STRING => "String"
  | COMMENT => "Comment";

type location = {
  start: int,
  end_: int,
  line: int,
  column: int,
};

type token = {
  kind: tokenKind,
  location,
  prev: option(token),
  value: string,
};

type t = {
  body: string,
  mutable lastToken: token,
  mutable token,
  mutable line: int,
  mutable lineStart: int,
};

let tok = (~value="", ~prev=None, kind, ~start, ~end_, ~line, ~column) => {
  kind,
  location: {
    start,
    end_,
    line,
    column,
  },
  prev,
  value,
};

exception SyntaxError(string);

/* (line:col) kind: <token.kind>, value: <token.value> */
let printToken = token => {
  let kindStr = strOfTokenKind(token.kind);
  let lineCol =
    "("
    ++ string_of_int(token.location.line)
    ++ ":"
    ++ string_of_int(token.location.column)
    ++ ")";

  lineCol
  ++ (
    switch (token.value) {
    | "" => " kind: '" ++ kindStr ++ "'"
    | v => " kind: '" ++ kindStr ++ "', value: " ++ v
    }
  );
};

let safeMatch = (body, position, char) =>
  position < String.length(body) && body.[position] == char;

/**
 * Reads from body starting at startPosition until it finds a non-whitespace
 * character, then returns the position of that character for lexing.
 */
let positionAfterWhitespace = (lexer, startPosition) => {
  let {body} = lexer;

  let rec aux = position =>
    if (position >= String.length(body)) {
      position;
    } else {
      switch (body.[position]) {
      | char when int_of_char(char) == 0xFEFF /* BOM */ => position + 1
      | ' '
      | ','
      | '\t' => aux(position + 1)
      | '\n' =>
        let newPosition = position + 1;
        lexer.line = lexer.line + 1;
        lexer.lineStart = newPosition;
        aux(newPosition);
      | '\r' =>
        let newPosition = safeMatch(body, position + 1, '\n') ? position + 2 : position + 1;

        lexer.line = lexer.line + 1;
        lexer.lineStart = newPosition;
        aux(newPosition);
      | _ => position
      };
    };

  aux(startPosition);
};

let isNameChar =
  fun
  | 'A'..'Z'
  | 'a'..'z'
  | '_' => true
  | _ => false;

/**
 * Reads an alphanumeric + underscore name from the source.
 *
 * [_A-Za-z][_0-9A-Za-z]*
 */
let readName = (body, start, line, column, prev): token => {
  let rec aux = position =>
    switch (body.[position]) {
    | 'A'..'Z'
    | 'a'..'z'
    | '_' => aux(position + 1)
    | _ => {
        kind: NAME,
        location: {
          start,
          line,
          end_: position,
          column,
        },
        prev,
        value: String.sub(body, start, position - start),
      }
    };

  aux(start);
};

/**
 * Reads a comment token from the source file.
 *
 * #[\u0009\u0020-\uFFFF]*
 */
let readComment = (body, start, line, column, prev): token => {
  let rec aux = position =>
    if (position > String.length(body)) {
      position;
    } else {
      switch (body.[position]) {
      | '\r'
      | '\n' => position
      | _ => aux(position + 1)
      };
    };

  let position = aux(start);

  {
    kind: COMMENT,
    location: {
      start,
      line,
      end_: position,
      column,
    },
    prev,
    value: String.sub(body, start, position - start),
  };
};

let readDigits = (body, startingPosition): int => {
  let rec aux = (body, pos) =>
    if (pos >= String.length(body)) {
      pos;
    } else {
      switch (body.[pos]) {
      | '0'..'9' => aux(body, pos + 1)
      | c when pos === startingPosition =>
        raise(SyntaxError("Invalid number, expected digit but got: " ++ String.make(1, c)))
      | _ => pos
      };
    };

  aux(body, startingPosition);
};

/**
 * Reads a number token from the source file, either a float
 * or an int depending on whether a decimal point appears.
 *
 * Int:   -?(0|[1-9][0-9]*)
 * Float: -?(0|[1-9][0-9]*)(\.[0-9]+)?((E|e)(+|-)?[0-9]+)?
 */
let readNumber = (body, start, line, column, prev) => {
  let kind = ref(INT);
  let position = ref(start);

  if (body.[start] == '-') {
    position := position^ + 1;
  };

  if (body.[position^] == '0') {
    position := position^ + 1;
    switch (body.[position^]) {
    | '0'..'9' as char =>
      raise(SyntaxError("Invalid number, unexpected digit after 0: " ++ String.make(1, char)))
    | _ => ()
    };
  } else {
    position := readDigits(body, position^);
  };

  if (safeMatch(body, position^, '.')) {
    kind := FLOAT;
    position := position^ + 1;
    position := readDigits(body, position^);
  };

  if (safeMatch(body, position^, 'E') || safeMatch(body, position^, 'e')) {
    kind := FLOAT;
    position := position^ + 1;

    if (safeMatch(body, position^, '+') || safeMatch(body, position^, '-')) {
      position := position^ + 1;
    };

    position := readDigits(body, position^);
  };

  {
    kind: kind^,
    location: {
      start,
      end_: position^,
      line,
      column,
    },
    prev,
    value: String.sub(body, start, position^ - start),
  };
};

/**
 * Reads a string token from the source file.
 *
 * "([^"\\\u000A\u000D]|(\\(u[0-9a-fA-F]{4}|["\\/bfnrt])))*"
 */
let readString = (body, start, line, column, prev) => failwith("Read String is Not Implemented");

/**
 * Gets the next token from the source starting at the given position.
 *
 * This skips over whitespace until it finds the next lexable token, then lexes
 * punctuators immediately or calls the appropriate helper function for more
 * complicated tokens.
 */
let readToken = (lexer, prevToken) => {
  let {body} = lexer;
  let length = String.length(body);
  let position = positionAfterWhitespace(lexer, prevToken.location.end_);
  let line = lexer.line;
  let column = 1 + position - lexer.lineStart;

  let prev = Some(prevToken);

  let location = {start: position, end_: position + 1, line, column};

  if (position >= length) {
    {kind: EOF, location, prev: Some(prevToken), value: ""};
  } else {
    switch (body.[position]) {
    | '!' => {kind: BANG, location, value: "", prev}
    | '$' => {kind: DOLLAR, location, value: "", prev}
    | '&' => {kind: AMP, location, value: "", prev}
    | '(' => {kind: PAREN_L, location, value: "", prev}
    | ')' => {kind: PAREN_R, location, value: "", prev}
    | '.' =>
      if (safeMatch(body, position + 1, '.') && safeMatch(body, position + 2, '.')) {
        {
          kind: SPREAD,
          location: {
            ...location,
            end_: position + 3,
          },
          value: "",
          prev,
        };
      } else if (position + 1 >= String.length(body)) {
        raise(SyntaxError("Unexpected End of File"));
      } else {
        raise(SyntaxError("Unexpected Character" ++ String.make(1, body.[position + 1])));
      }
    | ':' => {kind: COLON, location, value: "", prev}
    | '=' => {kind: EQUALS, location, value: "", prev}
    | '@' => {kind: AT, location, value: "", prev}
    | '[' => {kind: BRACKET_L, location, value: "", prev}
    | ']' => {kind: BRACKET_R, location, value: "", prev}
    | '{' => {kind: BRACE_L, location, value: "", prev}
    | '|' => {kind: PIPE, location, value: "", prev}
    | '}' => {kind: BRACE_R, location, value: "", prev}
    | 'A'..'Z'
    | 'a'..'z'
    | '_' => readName(body, position, line, column, prev)
    | '0'..'9'
    | '-' => readNumber(body, position, line, column, prev)
    | '"' => readString(body, position, line, column, prev)
    | '#' => readComment(body, position, line, column, prev)
    | char => raise(SyntaxError("Unexpected Character" ++ String.make(1, char)))
    };
  };
};

let sof = tok(SOF, ~start=0, ~end_=0, ~column=0, ~line=1);

let make = body => {body, lastToken: sof, token: sof, line: 1, lineStart: 0};

let lookahead = lexer => {
  let token = lexer.token;
  switch (token.kind) {
  | EOF => token
  | _ =>
    let tok = ref(readToken(lexer, token));
    while (tok^.kind == COMMENT) {
      tok := readToken(lexer, tok^);
    };
    tok^;
  };
};

let advance = lexer => {
  lexer.lastToken = lexer.token;
  lexer.token = lookahead(lexer);
};