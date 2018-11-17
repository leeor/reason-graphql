// Generated by BUCKLESCRIPT VERSION 4.0.5, PLEASE EDIT WITH CARE
'use strict';

var List = require("bs-platform/lib/js/list.js");
var Block = require("bs-platform/lib/js/block.js");
var Curry = require("bs-platform/lib/js/curry.js");
var Pervasives = require("bs-platform/lib/js/pervasives.js");
var Caml_format = require("bs-platform/lib/js/caml_format.js");
var Ast$ReasonGraphqlServer = require("./Ast.bs.js");
var Lexer$ReasonGraphqlServer = require("./Lexer.bs.js");

function peek(lexer, kind) {
  return lexer[/* token */2][/* kind */0] === kind;
}

function expect(lexer, kind) {
  var token = lexer[/* token */2];
  if (token[/* kind */0] === kind) {
    Lexer$ReasonGraphqlServer.advance(lexer);
    return token;
  } else {
    console.log(Lexer$ReasonGraphqlServer.strOfTokenKind(kind));
    return Pervasives.failwith("Expected" + (Lexer$ReasonGraphqlServer.strOfTokenKind(kind) + (", found " + Lexer$ReasonGraphqlServer.printToken(token))));
  }
}

function skip(lexer, kind) {
  if (lexer[/* token */2][/* kind */0] === kind) {
    Lexer$ReasonGraphqlServer.advance(lexer);
    return true;
  } else {
    return false;
  }
}

function skipKeyword(lexer, value) {
  var match = lexer[/* token */2];
  if (match[/* kind */0] === /* NAME */16 && match[/* value */3] === value) {
    Lexer$ReasonGraphqlServer.advance(lexer);
    return true;
  } else {
    return false;
  }
}

function expectKeyword(lexer, value) {
  if (skipKeyword(lexer, value)) {
    return 0;
  } else {
    return Pervasives.failwith("Expected " + (value + (", found " + Lexer$ReasonGraphqlServer.printToken(lexer[/* token */2]))));
  }
}

function unexpected(atToken, lexer) {
  var token = atToken !== undefined ? atToken : lexer[/* token */2];
  return Pervasives.failwith("Unexpected " + Lexer$ReasonGraphqlServer.printToken(token));
}

function any(lexer, openKind, parseFn, closeKind) {
  expect(lexer, openKind);
  var nodes = /* [] */0;
  while(!skip(lexer, closeKind)) {
    nodes = /* :: */Block.simpleVariant("::", [
        Curry._1(parseFn, lexer),
        nodes
      ]);
  };
  return List.rev(nodes);
}

function many(lexer, openKind, parseFn, closeKind) {
  expect(lexer, openKind);
  var nodes = /* :: */Block.simpleVariant("::", [
      Curry._1(parseFn, lexer),
      /* [] */0
    ]);
  while(!skip(lexer, closeKind)) {
    nodes = /* :: */Block.simpleVariant("::", [
        Curry._1(parseFn, lexer),
        nodes
      ]);
  };
  return List.rev(nodes);
}

function parseStringLiteral(lexer) {
  var token = lexer[/* token */2];
  Lexer$ReasonGraphqlServer.advance(lexer);
  return /* String */Block.variant("String", 3, [token[/* value */3]]);
}

function parseName(lexer) {
  return expect(lexer, /* NAME */16)[/* value */3];
}

function parseNamedType(lexer) {
  return /* NamedType */Block.variant("NamedType", 0, [expect(lexer, /* NAME */16)[/* value */3]]);
}

function parseVariable(lexer) {
  expect(lexer, /* DOLLAR */3);
  return /* Variable */Block.variant("Variable", 7, [expect(lexer, /* NAME */16)[/* value */3]]);
}

function parseValueLiteral(lexer, isConst) {
  var token = lexer[/* token */2];
  var match = token[/* kind */0];
  switch (match) {
    case 3 : 
        if (isConst) {
          return unexpected(undefined, lexer);
        } else {
          return parseVariable(lexer);
        }
    case 11 : 
        return parseList(lexer, isConst);
    case 13 : 
        return parseObject(lexer, isConst);
    case 16 : 
        var $$enum = token[/* value */3];
        switch ($$enum) {
          case "false" : 
              Lexer$ReasonGraphqlServer.advance(lexer);
              return /* Boolean */Block.variant("Boolean", 2, [false]);
          case "null" : 
              Lexer$ReasonGraphqlServer.advance(lexer);
              return /* Null */0;
          case "true" : 
              Lexer$ReasonGraphqlServer.advance(lexer);
              return /* Boolean */Block.variant("Boolean", 2, [true]);
          default:
            Lexer$ReasonGraphqlServer.advance(lexer);
            return /* Enum */Block.variant("Enum", 4, [$$enum]);
        }
    case 17 : 
        Lexer$ReasonGraphqlServer.advance(lexer);
        return /* Int */Block.variant("Int", 0, [Caml_format.caml_int_of_string(token[/* value */3])]);
    case 18 : 
        Lexer$ReasonGraphqlServer.advance(lexer);
        return /* Float */Block.variant("Float", 1, [Caml_format.caml_float_of_string(token[/* value */3])]);
    case 19 : 
        return parseStringLiteral(lexer);
    case 0 : 
    case 1 : 
    case 2 : 
    case 4 : 
    case 5 : 
    case 6 : 
    case 7 : 
    case 8 : 
    case 9 : 
    case 10 : 
    case 12 : 
    case 14 : 
    case 15 : 
    case 20 : 
        return unexpected(undefined, lexer);
    
  }
}

function parseList(lexer, isConst) {
  var parseFn = function (param) {
    return parseValueLiteral(param, isConst);
  };
  return /* List */Block.variant("List", 5, [any(lexer, /* BRACKET_L */11, parseFn, /* BRACKET_R */12)]);
}

function parseObject(lexer, isConst) {
  expect(lexer, /* BRACE_L */13);
  var fields = Ast$ReasonGraphqlServer.StringMap[/* empty */0];
  while(!skip(lexer, /* BRACE_R */15)) {
    var match = parseObjectField(lexer, isConst);
    fields = Curry._3(Ast$ReasonGraphqlServer.StringMap[/* add */3], match[0], match[1], fields);
  };
  return /* Object */Block.variant("Object", 6, [fields]);
}

function parseObjectField(lexer, isConst) {
  var name = expect(lexer, /* NAME */16)[/* value */3];
  expect(lexer, /* COLON */8);
  var value = parseValueLiteral(lexer, isConst);
  return /* tuple */[
          name,
          value
        ];
}

function parseTypeReference(lexer) {
  var typ;
  if (skip(lexer, /* BRACKET_L */11)) {
    var t = parseTypeReference(lexer);
    expect(lexer, /* BRACKET_R */12);
    typ = /* ListType */Block.variant("ListType", 1, [t]);
  } else {
    typ = /* NamedType */Block.variant("NamedType", 0, [expect(lexer, /* NAME */16)[/* value */3]]);
  }
  var match = skip(lexer, /* BANG */2);
  if (match) {
    return /* NonNullType */Block.variant("NonNullType", 2, [typ]);
  } else {
    return typ;
  }
}

function parseArgument(lexer) {
  var name = expect(lexer, /* NAME */16)[/* value */3];
  expect(lexer, /* COLON */8);
  return /* record */Block.record([
            "name",
            "value"
          ], [
            name,
            parseValueLiteral(lexer, false)
          ]);
}

function parseConstArgument(lexer) {
  var name = expect(lexer, /* NAME */16)[/* value */3];
  expect(lexer, /* COLON */8);
  return /* record */Block.record([
            "name",
            "value"
          ], [
            name,
            parseValueLiteral(lexer, true)
          ]);
}

function parseArguments(lexer, isConst) {
  var parseFn = isConst ? parseConstArgument : parseArgument;
  var match = peek(lexer, /* PAREN_L */5);
  if (match) {
    return many(lexer, /* PAREN_L */5, parseFn, /* PAREN_R */6);
  } else {
    return /* [] */0;
  }
}

function parseDirective(lexer, isConst) {
  expect(lexer, /* AT */10);
  return /* record */Block.record([
            "name",
            "arguments"
          ], [
            expect(lexer, /* NAME */16)[/* value */3],
            parseArguments(lexer, isConst)
          ]);
}

function parseDirectives(lexer, isConst) {
  var directives = /* [] */0;
  while(peek(lexer, /* AT */10)) {
    directives = /* :: */Block.simpleVariant("::", [
        parseDirective(lexer, isConst),
        directives
      ]);
  };
  return List.rev(directives);
}

function parseOperationType(lexer) {
  var operationToken = expect(lexer, /* NAME */16);
  var match = operationToken[/* value */3];
  switch (match) {
    case "mutation" : 
        return /* Mutation */1;
    case "query" : 
        return /* Query */0;
    case "subscription" : 
        return /* Subscription */2;
    default:
      return unexpected(operationToken, lexer);
  }
}

function parseVariableDefinition(lexer) {
  var variable = parseVariable(lexer);
  expect(lexer, /* COLON */8);
  var typ = parseTypeReference(lexer);
  return /* record */Block.record([
            "variable",
            "typ",
            "defaultValue",
            "directives"
          ], [
            variable,
            typ,
            undefined,
            parseDirectives(lexer, true)
          ]);
}

function parseVariableDefinitions(lexer) {
  var match = peek(lexer, /* PAREN_L */5);
  if (match) {
    return many(lexer, /* PAREN_L */5, parseVariableDefinition, /* PAREN_R */6);
  } else {
    return /* [] */0;
  }
}

function parseSelectionSet(lexer) {
  return many(lexer, /* BRACE_L */13, parseSelection, /* BRACE_R */15);
}

function parseSelection(lexer) {
  if (peek(lexer, /* SPREAD */7)) {
    return parseFragment(lexer);
  } else {
    return parseField(lexer);
  }
}

function parseFragmentName(lexer) {
  var token = lexer[/* token */2];
  if (token[/* value */3] === "on") {
    return unexpected(undefined, lexer);
  } else {
    return expect(lexer, /* NAME */16)[/* value */3];
  }
}

function parseFragment(lexer) {
  expect(lexer, /* SPREAD */7);
  var hasTypeCondition = skipKeyword(lexer, "on");
  if (!hasTypeCondition && peek(lexer, /* NAME */16)) {
    return /* FragmentSpread */Block.variant("FragmentSpread", 1, [/* record */Block.record([
                  "name",
                  "directives"
                ], [
                  parseFragmentName(lexer),
                  parseDirectives(lexer, false)
                ])]);
  } else {
    return /* InlineFragment */Block.variant("InlineFragment", 2, [/* record */Block.record([
                  "typeCondition",
                  "selectionSet",
                  "directives"
                ], [
                  expect(lexer, /* NAME */16)[/* value */3],
                  parseSelectionSet(lexer),
                  parseDirectives(lexer, false)
                ])]);
  }
}

function parseField(lexer) {
  var name = expect(lexer, /* NAME */16)[/* value */3];
  var match = skip(lexer, /* COLON */8) ? /* tuple */[
      name,
      expect(lexer, /* NAME */16)[/* value */3]
    ] : /* tuple */[
      undefined,
      name
    ];
  var $$arguments = parseArguments(lexer, false);
  var directives = parseDirectives(lexer, false);
  var match$1 = peek(lexer, /* BRACE_L */13);
  var selectionSet = match$1 ? parseSelectionSet(lexer) : /* [] */0;
  return /* Field */Block.variant("Field", 0, [/* record */Block.record([
                "alias",
                "name",
                "arguments",
                "selectionSet",
                "directives"
              ], [
                match[0],
                match[1],
                $$arguments,
                selectionSet,
                directives
              ])]);
}

function parseOperationDefintiion(lexer) {
  if (peek(lexer, /* BRACE_L */13)) {
    return /* OperationDefinition */Block.variant("OperationDefinition", 0, [/* record */Block.record([
                  "operationType",
                  "name",
                  "variableDefinition",
                  "directives",
                  "selectionSet"
                ], [
                  0,
                  undefined,
                  0,
                  0,
                  parseSelectionSet(lexer)
                ])]);
  } else {
    var operationType = parseOperationType(lexer);
    var match = peek(lexer, /* NAME */16);
    var name = match ? expect(lexer, /* NAME */16)[/* value */3] : undefined;
    var variableDefinition = parseVariableDefinitions(lexer);
    var directives = parseDirectives(lexer, false);
    var selectionSet = parseSelectionSet(lexer);
    return /* OperationDefinition */Block.variant("OperationDefinition", 0, [/* record */Block.record([
                  "operationType",
                  "name",
                  "variableDefinition",
                  "directives",
                  "selectionSet"
                ], [
                  operationType,
                  name,
                  variableDefinition,
                  directives,
                  selectionSet
                ])]);
  }
}

function parseFragmentDefinition(lexer) {
  expectKeyword(lexer, "fragment");
  var name = parseFragmentName(lexer);
  expectKeyword(lexer, "on");
  var typeCondition = expect(lexer, /* NAME */16)[/* value */3];
  var selectionSet = parseSelectionSet(lexer);
  var directives = parseDirectives(lexer, false);
  return /* FragmentDefinition */Block.variant("FragmentDefinition", 1, [/* record */Block.record([
                "name",
                "typeCondition",
                "selectionSet",
                "directives"
              ], [
                name,
                typeCondition,
                selectionSet,
                directives
              ])]);
}

function parseExecutableDefinition(lexer) {
  var token = lexer[/* token */2];
  if (peek(lexer, /* NAME */16)) {
    var match = token[/* value */3];
    switch (match) {
      case "fragment" : 
          return parseFragmentDefinition(lexer);
      case "mutation" : 
      case "query" : 
      case "subscription" : 
          return parseOperationDefintiion(lexer);
      default:
        return unexpected(undefined, lexer);
    }
  } else if (peek(lexer, /* BRACE_L */13)) {
    return parseOperationDefintiion(lexer);
  } else {
    return unexpected(undefined, lexer);
  }
}

function parseDefinition(lexer) {
  if (peek(lexer, /* NAME */16)) {
    var match = lexer[/* token */2][/* value */3];
    switch (match) {
      case "extend" : 
          return Pervasives.failwith("implmeent type system extension");
      case "fragment" : 
      case "mutation" : 
      case "query" : 
      case "subscription" : 
          return parseExecutableDefinition(lexer);
      case "directive" : 
      case "enum" : 
      case "input" : 
      case "interface" : 
      case "scalar" : 
      case "schema" : 
      case "type" : 
      case "union" : 
          return Pervasives.failwith("implement type system defintion");
      default:
        return unexpected(undefined, lexer);
    }
  } else if (peek(lexer, /* BRACE_L */13)) {
    return parseExecutableDefinition(lexer);
  } else {
    return Pervasives.failwith("implement");
  }
}

function parseDocument(lexer) {
  return /* record */Block.record(["definitions"], [many(lexer, /* SOF */0, parseDefinition, /* EOF */1)]);
}

function parse(body) {
  return parseDocument(Lexer$ReasonGraphqlServer.make(body));
}

exports.peek = peek;
exports.expect = expect;
exports.skip = skip;
exports.skipKeyword = skipKeyword;
exports.expectKeyword = expectKeyword;
exports.unexpected = unexpected;
exports.any = any;
exports.many = many;
exports.parseStringLiteral = parseStringLiteral;
exports.parseName = parseName;
exports.parseNamedType = parseNamedType;
exports.parseVariable = parseVariable;
exports.parseValueLiteral = parseValueLiteral;
exports.parseList = parseList;
exports.parseObject = parseObject;
exports.parseObjectField = parseObjectField;
exports.parseTypeReference = parseTypeReference;
exports.parseArgument = parseArgument;
exports.parseConstArgument = parseConstArgument;
exports.parseArguments = parseArguments;
exports.parseDirective = parseDirective;
exports.parseDirectives = parseDirectives;
exports.parseOperationType = parseOperationType;
exports.parseVariableDefinition = parseVariableDefinition;
exports.parseVariableDefinitions = parseVariableDefinitions;
exports.parseSelectionSet = parseSelectionSet;
exports.parseSelection = parseSelection;
exports.parseFragmentName = parseFragmentName;
exports.parseFragment = parseFragment;
exports.parseField = parseField;
exports.parseOperationDefintiion = parseOperationDefintiion;
exports.parseFragmentDefinition = parseFragmentDefinition;
exports.parseExecutableDefinition = parseExecutableDefinition;
exports.parseDefinition = parseDefinition;
exports.parseDocument = parseDocument;
exports.parse = parse;
/* Ast-ReasonGraphqlServer Not a pure module */