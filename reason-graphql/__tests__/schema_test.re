open Jest;
open Expect;
open Graphql.Language;

let schema = StarWarsSchema.schema;

module Schema = StarWarsSchema.Schema;

describe("Basic Queries", () => {
  testAsync(
    "Correctly identifies R2-D2 as the hero of the Star Wars Saga", assertion => {
    let query = {|
      query HeroNameQuery {
        hero {
          name
        }
      }
    |};

    let expected =
      Schema.okResponse(
        `Map([("hero", `Map([("name", `String("R2-D2"))]))]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  });

  testAsync("Allows us to query for the ID and friends of R2-D2", assertion => {
    let query = {|
        query HeroNameAndFriendsQuery {
          hero {
            id
            name
            friends {
              name
            }
          }
        }
    |};

    let expected =
      Schema.okResponse(
        `Map([
          (
            "hero",
            `Map([
              ("id", `Int(2001)),
              ("name", `String("R2-D2")),
              (
                "friends",
                `List([
                  `Map([("name", `String("Luke Skywalker"))]),
                  `Map([("name", `String("Han Solo"))]),
                  `Map([("name", `String("Leia Organa"))]),
                ]),
              ),
            ]),
          ),
        ]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  });
});

describe("Nested Queries", () =>
  testAsync(
    "Allows us to query for the friends of friends of R2-D2", assertion => {
    let query = {|
      query NestedQuery {
        hero {
          name
          friends {
            name
            appearsIn
            friends {
              name
            }
          }
        }
      }
    |};

    let expected =
      Schema.okResponse(
        `Map([
          (
            "hero",
            `Map([
              ("name", `String("R2-D2")),
              (
                "friends",
                `List([
                  `Map([
                    ("name", `String("Luke Skywalker")),
                    (
                      "appearsIn",
                      `List([
                        `String("NEWHOPE"),
                        `String("EMPIRE"),
                        `String("JEDI"),
                      ]),
                    ),
                    (
                      "friends",
                      `List([
                        `Map([("name", `String("Han Solo"))]),
                        `Map([("name", `String("Leia Organa"))]),
                        `Map([("name", `String("C-3PO"))]),
                        `Map([("name", `String("R2-D2"))]),
                      ]),
                    ),
                  ]),
                  `Map([
                    ("name", `String("Han Solo")),
                    (
                      "appearsIn",
                      `List([
                        `String("NEWHOPE"),
                        `String("EMPIRE"),
                        `String("JEDI"),
                      ]),
                    ),
                    (
                      "friends",
                      `List([
                        `Map([("name", `String("Luke Skywalker"))]),
                        `Map([("name", `String("Leia Organa"))]),
                        `Map([("name", `String("R2-D2"))]),
                      ]),
                    ),
                  ]),
                  `Map([
                    ("name", `String("Leia Organa")),
                    (
                      "appearsIn",
                      `List([
                        `String("NEWHOPE"),
                        `String("EMPIRE"),
                        `String("JEDI"),
                      ]),
                    ),
                    (
                      "friends",
                      `List([
                        `Map([("name", `String("Luke Skywalker"))]),
                        `Map([("name", `String("Han Solo"))]),
                        `Map([("name", `String("C-3PO"))]),
                        `Map([("name", `String("R2-D2"))]),
                      ]),
                    ),
                  ]),
                ]),
              ),
            ]),
          ),
        ]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  })
);

describe("Mutation operation", () => {
  open Expect;

  let mutation = {|
    mutation MyMutation($id: Int!, $name: String!){
      updateCharacterName(characterId: $id, name: $name) {
        character {
          id
          name
        }
        error
      }
    }
  |};

  let variables =
    "{\"id\": 1000, \"name\": \"Sikan Skywalker\"}"
    ->Js.Json.parseExn
    ->Graphql_Json.toVariables
    ->Belt.Result.getExn;

  let result =
    schema
    |> Schema.execute(
         _,
         ~document=Parser.parse(mutation)->Belt.Result.getExn,
         ~ctx=Some("userId"),
         ~variables,
       )
    |> Schema.resultToJson;

  testAsync("returns the right data", assertion => {
    let expected =
      Schema.okResponse(
        `Map([
          (
            "updateCharacterName",
            `Map([
              (
                "character",
                `Map([
                  ("id", `Int(1000)),
                  ("name", `String("Sikan Skywalker")),
                ]),
              ),
              ("error", `Null),
            ]),
          ),
        ]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    Schema.Io.map(result, res =>
      assertion(expect(res) |> toEqual(expected))
    )
    ->ignore;
  });
});

describe("Mutation operation (error)", () => {
  open Expect;

  let mutation = {|
    mutation MyMutation($id: Int!, $name: String!){
      updateCharacterName(characterId: $id, name: $name) {
        character {
          id
          name
        }
        error
      }
    }
  |};

  let variables =
    "{\"id\": 1000, \"name\": \"Sikan Skywalker\"}"
    ->Js.Json.parseExn
    ->Graphql_Json.toVariables
    ->Belt.Result.getExn;

  let result =
    schema
    |> Schema.execute(
         _,
         ~document=Parser.parse(mutation)->Belt.Result.getExn,
         ~ctx=None,
         ~variables,
       );

  testAsync("returns the right data", assertion => {
    let expected =
      Belt.Result.Error((StarWarsSchema.Error.Unauthenticated, []));

    Schema.Io.map(result, res =>
      assertion(expect(res) |> toEqual(expected))
    )
    ->ignore;
  });
});

describe("Using aliases to change the key in the response", () => {
  open Expect;

  testAsync(
    "Allows us to query for Luke, changing his key with an alias", assertion => {
    let query = {|
      query FetchLukeAliased {
        luke: human(id: 1000) {
          name
        }
      }
    |};

    let expected =
      Schema.okResponse(
        `Map([("luke", `Map([("name", `String("Luke Skywalker"))]))]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  });

  testAsync(
    "Allows us to query for both Luke and Leia, using two root fields and an alias",
    assertion => {
      let query = {|
      query FetchLukeAndLeiaAliased {
        luke: human(id: 1000) {
          name
        }
        leia: human(id: 1003) {
          name
        }
      }
    |};

      let expected =
        Schema.okResponse(
          `Map([
            ("luke", `Map([("name", `String("Luke Skywalker"))])),
            ("leia", `Map([("name", `String("Leia Organa"))])),
          ]),
        )
        ->Belt.Result.map(Graphql_Json.fromConstValue);

      schema
      ->Schema.execute(
          ~document=Parser.parse(query)->Belt.Result.getExn,
          ~ctx=Some("userId"),
        )
      ->Schema.resultToJson
      ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
      ->ignore;
    },
  );
});

describe("Uses fragments to express more complex queries", () => {
  open Expect;

  testAsync("Allows us to query using duplicated content", assertion => {
    let query = {|
      query DuplicateFields {
        luke: human(id: 1000) {
          name
          homePlanet
        }
        leia: human(id: 1003) {
          name
          homePlanet
        }
      }
    |};

    let expected =
      Schema.okResponse(
        `Map([
          (
            "luke",
            `Map([
              ("name", `String("Luke Skywalker")),
              ("homePlanet", `String("Tatooine")),
            ]),
          ),
          (
            "leia",
            `Map([
              ("name", `String("Leia Organa")),
              ("homePlanet", `String("Alderaan")),
            ]),
          ),
        ]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  });

  testAsync(
    "Allows us to use a fragment to avoid duplicating content", assertion => {
    let query = {|
    query UseFragment {
      luke: human(id: 1000) {
        ...HumanFragment
      }
      leia: human(id: 1003) {
        ...HumanFragment
      }
    }

    fragment HumanFragment on Human {
      name
      homePlanet
    }
  |};

    let schema = StarWarsSchema.schema;

    let expected =
      Schema.okResponse(
        `Map([
          (
            "luke",
            `Map([
              ("name", `String("Luke Skywalker")),
              ("homePlanet", `String("Tatooine")),
            ]),
          ),
          (
            "leia",
            `Map([
              ("name", `String("Leia Organa")),
              ("homePlanet", `String("Alderaan")),
            ]),
          ),
        ]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  });

  testAsync(
    "Allows us to use __typename to get a containing object's type name",
    assertion => {
    let query = {|
    query UseFragment {
      luke: human(id: 1000) {
        __typename
      }
    }
  |};

    let schema = StarWarsSchema.schema;

    let expected =
      Schema.okResponse(
        `Map([("luke", `Map([("__typename", `String("Human"))]))]),
      )
      ->Belt.Result.map(Graphql_Json.fromConstValue);

    schema
    ->Schema.execute(
        ~document=Parser.parse(query)->Belt.Result.getExn,
        ~ctx=Some("userId"),
      )
    ->Schema.resultToJson
    ->Schema.Io.map(res => assertion(expect(res) |> toEqual(expected)))
    ->ignore;
  });
});
