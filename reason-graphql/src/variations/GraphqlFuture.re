module Future = {
  include Future;
  let return = value;
  let bind = flatMap;
  let ok = x => value(Belt.Result.Ok(x));
};

module Make = (Error: Graphql.Schema.Error) => {
  include Graphql.Schema.Make(Future, Error);
};

module Language = Graphql.Language;
