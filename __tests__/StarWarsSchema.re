module StarWars = StarWarsData;

let episodeEnum =
  Schema.(
    makeEnum(
      "Episode",
      [
        enumValue("NEWHOPE", ~value=StarWars.NEWHOPE, ~description="Released in 1977"),
        enumValue("EMPIRE", ~value=StarWars.EMPIRE, ~description="Released in 1980"),
        enumValue("JEDI", ~value=StarWars.JEDI, ~description="Released in 1983"),
      ],
    )
  );

let humanType =
  Schema.(
    obj("Human", ~fields=humanType =>
      [
        field("id", string, ~args=[], (human: StarWars.human) => human.id),
        field("name", string, ~args=[], (human: StarWars.human) => human.StarWars.name),
        field("appearsIn", ~args=[], list(episodeEnum.fieldType), (human: StarWars.human) =>
          human.StarWars.appearsIn
        ),
        field("friends", ~args=[], list(humanType), (human: StarWars.human) =>
          StarWars.getFriends(human.StarWars.friends)
        ),
      ]
    )
  );

let queryType =
  Schema.(
    rootQuery([
      field("hero", humanType, ~args=Arg.[arg("id", int)], ((), id) =>
        StarWars.getHero(string_of_int(id))
      ),
      field("heroes", list(humanType), ~args=Arg.[arg("ids", list(int))], ((), ids) =>
        ids |> List.map(string_of_int) |> StarWars.getFriends
      ),
    ])
  );

let schema: Schema.t = Schema.create(~query=queryType);