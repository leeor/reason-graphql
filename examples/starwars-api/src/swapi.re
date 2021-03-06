type film = {
  title: string,
  episodeID: int,
  openingCrawl: string,
  director: string,
  producers: list(string),
  releaseDate: string,
  characterUrls: list(string),
};

type person = {
  name: string,
  birthYear: string,
  eyeColor: string,
  gender: string,
  hairColor: string,
  height: option(int),
  mass: option(float),
  skinColor: string,
  filmUrls: list(string),
  speciesUrls: list(string),
  vehicleUrls: list(string),
  starshipUrls: list(string),
};

type planet = {
  name: string,
  diameter: option(int),
  rotationPeriod: option(int),
  orbitalPeriod: option(int),
  gravity: string,
  population: option(float),
  climates: list(string),
  terrains: list(string),
  surfaceWater: option(float),
};

type species = {
  name: string,
  classification: string,
  designation: string,
  averageHeight: option(float),
  averageLifespan: option(int),
  eyeColors: list(string),
  hairColors: list(string),
  skinColors: list(string),
  language: string,
};

type starship = {
  name: string,
  model: string,
  starshipClass: string,
  manufacturers: list(string),
  costInCredits: option(float),
  length: option(float),
  crew: string,
  passengers: string,
  maxAtmospheringSpeed: option(int),
  hyperdriveRating: option(float),
  mglt: option(int),
  cargoCapacity: option(float),
  consumables: string,
};

type vehicle = {
  name: string,
  model: string,
  vehicleClass: string,
  manufacturers: list(string),
  costInCredits: option(float),
  length: option(float),
  crew: string,
  passengers: string,
  maxAtmospheringSpeed: option(int),
  cargoCapacity: option(float),
  consumables: string,
};

[%raw "require('isomorphic-fetch')"];

let baseUrl = "https://swapi.co/api/";

exception RecordNotFound;

let removeGrouping = numberString =>
  Js.String.replaceByRe([%re "/,/"], "", numberString);

let parseFloat = n =>
  n |> Js.String.replaceByRe([%re "/,/"], "") |> Js.Float.fromString;
let parseFloatOpt =
  fun
  | "n/a"
  | "unknown" => None
  | n =>
    try (Some(n |> parseFloat)) {
    | Failure(_) => None
    };

let parseInt = n => {
  (n |> Js.String.replaceByRe([%re "/,/"], ""))->int_of_string;
};
let parseIntOpt =
  fun
  | "n/a"
  | "unknown" => None
  | n =>
    try (Some(n |> parseInt)) {
    | Failure(_) => None
    };

let rec futureAll =
  Future.(
    fun
    | [] => value([])
    | [future, ...rest] =>
      flatMap(future, value => map(futureAll(rest), acc => [value, ...acc]))
  );

let urlDataloader =
  Dataloader.make(urls =>
    urls
    |> List.map(url =>
         Js.Promise.(
           Fetch.fetch(url)
           |> then_(res =>
                Fetch.Response.status(res) == 404
                  ? reject(RecordNotFound) : resolve(res)
              )
           |> then_(Fetch.Response.json)
           |> FutureJs.fromPromise(_, Js.String.make)
         )
       )
    |> futureAll
  );

let getEntityByUrl = (decoder, url) =>
  Future.map(
    urlDataloader.load(url),
    fun
    | Ok(thing) => Belt.Result.Ok(Some(decoder(thing)))
    | Error(_) => Ok(None),
  );

let getEntityById = (path, decoder, id) =>
  getEntityByUrl(decoder, baseUrl ++ path ++ "/" ++ string_of_int(id));

let getAllEntitiesForType = (path, decoder, ()) => {
  let rec aux = (acc, nextUrl) => {
    urlDataloader.load(nextUrl)
    ->Future.flatMapOk(json => {
        let results = Json.Decode.(json |> field("results", list(decoder)));
        let nextUrl = Json.Decode.(json |> field("next", optional(string)));

        switch (nextUrl) {
        | Some(url) => aux(List.concat([acc, results]), url)
        | None => Future.value(Belt.Result.Ok(List.concat([acc, results])))
        };
      });
  };

  aux([], baseUrl ++ path);
};

let getEntitiesByUrls = (decoder, urls) => {
  urlDataloader.loadMany(urls)
  ->Future.map(
      List.fold_left(
        acc =>
          fun
          | Belt.Result.Ok(result) => [decoder(result), ...acc]
          | Error(_) => acc,
        [],
      ),
    )
  ->Future.map(list => Belt.Result.Ok(list));
};

let decodeFilm = (json: Js.Json.t): film =>
  Json.Decode.{
    title: json |> field("title", string),
    episodeID: json |> field("episode_id", int),
    openingCrawl: json |> field("opening_crawl", string),
    director: json |> field("director", string),
    producers:
      json
      |> field("producer", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    releaseDate: json |> field("release_date", string),
    characterUrls: json |> field("characters", list(string)),
  };

let getFilm = getEntityById("films", decodeFilm);
let getAllFilms = getAllEntitiesForType("films", decodeFilm);

let decodePerson = (json: Js.Json.t): person =>
  Json.Decode.{
    name: json |> field("name", string),
    birthYear: json |> field("birth_year", string),
    eyeColor: json |> field("eye_color", string),
    gender: json |> field("gender", string),
    hairColor: json |> field("hair_color", string),
    height: json |> field("height", string) |> parseIntOpt,
    mass: json |> field("mass", string) |> parseFloatOpt,
    skinColor: json |> field("skin_color", string),
    filmUrls: json |> field("films", list(string)),
    starshipUrls: json |> field("starships", list(string)),
    vehicleUrls: json |> field("vehicles", list(string)),
    speciesUrls: json |> field("species", list(string)),
  };

let getPerson = getEntityById("people", decodePerson);
let getAllPeople = getAllEntitiesForType("people", decodePerson);

let decodePlanet = (json: Js.Json.t): planet =>
  Json.Decode.{
    name: json |> field("name", string),
    diameter: json |> field("diameter", string) |> parseIntOpt,
    rotationPeriod: json |> field("rotation_period", string) |> parseIntOpt,
    orbitalPeriod: json |> field("orbital_period", string) |> parseIntOpt,
    gravity: json |> field("gravity", string),
    population: json |> field("population", string) |> parseFloatOpt,
    climates:
      json
      |> field("climate", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    terrains:
      json
      |> field("terrain", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    surfaceWater: json |> field("surface_water", string) |> parseFloatOpt,
  };

let getPlanet = getEntityById("planets", decodePlanet);
let getAllPlanets = getAllEntitiesForType("planets", decodePlanet);

let decodeSpecies = (json: Js.Json.t): species =>
  Json.Decode.{
    name: json |> field("name", string),
    classification: json |> field("classification", string),
    designation: json |> field("designation", string),
    averageHeight: json |> field("average_height", string) |> parseFloatOpt,
    averageLifespan: json |> field("average_lifespan", string) |> parseIntOpt,
    eyeColors:
      json
      |> field("eye_colors", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    hairColors:
      json
      |> field("hair_colors", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    skinColors:
      json
      |> field("skin_colors", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    language: json |> field("language", string),
  };

let getSpecies = getEntityById("species", decodeSpecies);
let getAllSpecies = getAllEntitiesForType("species", decodeSpecies);

let decodeStarship = (json: Js.Json.t): starship =>
  Json.Decode.{
    name: json |> field("name", string),
    model: json |> field("model", string),
    starshipClass: json |> field("starship_class", string),
    manufacturers:
      json
      |> field("manufacturer", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    costInCredits: json |> field("cost_in_credits", string) |> parseFloatOpt,
    length: json |> field("length", string) |> parseFloatOpt,
    crew: json |> field("crew", string),
    passengers: json |> field("passengers", string),
    maxAtmospheringSpeed:
      json |> field("max_atmosphering_speed", string) |> parseIntOpt,
    hyperdriveRating:
      json |> field("hyperdrive_rating", string) |> parseFloatOpt,
    mglt: json |> field("MGLT", string) |> parseIntOpt,
    cargoCapacity: json |> field("cargo_capacity", string) |> parseFloatOpt,
    consumables: json |> field("consumables", string),
  };

let getStarship = getEntityById("starships", decodeStarship);
let getAllStarships = getAllEntitiesForType("starships", decodeStarship);

let decodeVehicle = (json: Js.Json.t): vehicle =>
  Json.Decode.{
    name: json |> field("name", string),
    model: json |> field("model", string),
    vehicleClass: json |> field("vehicle_class", string),
    manufacturers:
      json
      |> field("manufacturer", string)
      |> Js.String.split(", ")
      |> Array.to_list,
    costInCredits: json |> field("cost_in_credits", string) |> parseFloatOpt,
    length: json |> field("length", string) |> parseFloatOpt,
    crew: json |> field("crew", string),
    passengers: json |> field("passengers", string),
    maxAtmospheringSpeed:
      json |> field("max_atmosphering_speed", string) |> parseIntOpt,
    cargoCapacity: json |> field("cargo_capacity", string) |> parseFloatOpt,
    consumables: json |> field("consumables", string),
  };

let getVehicle = getEntityById("vehicles", decodeVehicle);
let getAllVehicles = getAllEntitiesForType("vehicles", decodeVehicle);