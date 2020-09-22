type t = {
  id: string,
  name: string,
};

let id = t => t.id;

let name = t => t.name;

let decode = json => {
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
  };
};

let colors = [|
  "red",
  "orange",
  "yellow",
  "green",
  "teal",
  "blue",
  "indigo",
  "purple",
  "pink",
  "gray",
|];

let stringToInt = name => {
  let rec aux = (sum, remains) =>
    switch (remains) {
    | "" => sum
    | remains =>
      let firstCharacter = remains |> Js.String.slice(~from=0, ~to_=1);
      let remains = remains |> Js.String.sliceToEnd(~from=1);
      aux(sum +. (firstCharacter |> Js.String.charCodeAt(0)), remains);
    };

  aux(0.0, name) |> int_of_float;
};

let computeColors = name => {
  let index = (name |> stringToInt) mod 10;
  colors[index];
};

let color = t => {
  computeColors(t.name);
};
