open SchoolCommunities__IndexTypes;

let str = React.string;

type state = {
  categoryName: string,
  saving: bool,
  deleting: bool,
};

type action =
  | UpdateCategoryName(string)
  | StartSaving
  | FinishSaving(string)
  | FailSaving
  | StartDeleting
  | FailDeleting;

let reducer = (state, action) => {
  switch (action) {
  | UpdateCategoryName(categoryName) => {...state, categoryName}
  | StartSaving => {...state, saving: true}
  | FinishSaving(categoryName) => {...state, saving: false, categoryName}
  | FailSaving => {...state, saving: false}
  | StartDeleting => {...state, deleting: true}
  | FailDeleting => {...state, deleting: false}
  };
};

module CreateCategoryQuery = [%graphql
  {|
  mutation CreateCategoryMutation($name: String!, $communityId: ID!) {
    createTopicCategory(name: $name, communityId: $communityId ) {
      id
    }
  }
|}
];

module DeleteCategoryQuery = [%graphql
  {|
  mutation DeleteCategoryMutation($id: ID!) {
    deleteTopicCategory(id: $id ) {
      success
    }
  }
|}
];

module UpdateCategoryQuery = [%graphql
  {|
  mutation UpdateCategoryMutation($name: String!, $id: ID!) {
    updateTopicCategory(id: $id, name: $name ) {
      success
    }
  }
|}
];

let topicsCountPillClass = category => {
  let color = Category.color(category);
  "bg-" ++ color ++ "-200 text-" ++ color ++ "-900";
};

let deleteCategory = (categoryId, deleteCategoryCB, send, event) => {
  ReactEvent.Mouse.preventDefault(event);

  send(StartDeleting);

  DeleteCategoryQuery.make(~id=categoryId, ())
  |> GraphqlQuery.sendQuery
  |> Js.Promise.then_(response => {
       response##deleteTopicCategory##success
         ? deleteCategoryCB(categoryId) : send(FailDeleting);
       Js.Promise.resolve();
     })
  |> Js.Promise.catch(error => {
       Js.log(error);
       send(FailDeleting);
       Notification.error(
         "Unexpected Error!",
         "Please reload the page and try again.",
       );
       Js.Promise.resolve();
     })
  |> ignore;
};

let updateCategory = (category, newName, updateCategoryCB, send, event) => {
  ReactEvent.Mouse.preventDefault(event);

  send(StartSaving);

  UpdateCategoryQuery.make(~id=Category.id(category), ~name=newName, ())
  |> GraphqlQuery.sendQuery
  |> Js.Promise.then_(response => {
       response##updateTopicCategory##success
         ? {
           updateCategoryCB(Category.updateName(newName, category));
           send(FinishSaving(newName));
         }
         : send(FailSaving);
       Js.Promise.resolve();
     })
  |> Js.Promise.catch(error => {
       send(FailSaving);
       Js.log(error);
       Notification.error(
         "Unexpected Error!",
         "Please reload the page and try again.",
       );
       Js.Promise.resolve();
     })
  |> ignore;
};

let createCategory = (communityId, name, createCategoryCB, send, event) => {
  ReactEvent.Mouse.preventDefault(event);

  send(StartSaving);

  CreateCategoryQuery.make(~communityId, ~name, ())
  |> GraphqlQuery.sendQuery
  |> Js.Promise.then_(response => {
       switch (response##createTopicCategory##id) {
       | Some(id) =>
         let newCategory = Category.make(~id, ~name, ~topicsCount=0);
         createCategoryCB(newCategory);
         send(FinishSaving(""));
       | None => send(FailSaving)
       };

       Js.Promise.resolve();
     })
  |> Js.Promise.catch(error => {
       Js.log(error);
       send(FailSaving);
       Notification.error(
         "Unexpected Error!",
         "Please reload the page and try again.",
       );
       Js.Promise.resolve();
     })
  |> ignore;
};

let saveDisabled = (name, saving) => {
  String.trim(name) == "" || saving;
};

[@react.component]
let make =
    (
      ~category=?,
      ~communityId,
      ~deleteCategoryCB,
      ~createCategoryCB,
      ~updateCategoryCB,
    ) => {
  let (state, send) =
    React.useReducer(
      reducer,
      {
        categoryName:
          switch (category) {
          | Some(category) => Category.name(category)
          | None => ""
          },
        saving: false,
        deleting: false,
      },
    );
  switch (category) {
  | Some(category) =>
    let categoryId = Category.id(category);
    let presentCategoryName = Category.name(category);
    <div
      key=categoryId
      className="flex justify-between items-center bg-gray-100 border-gray-400 shadow rounded mt-3 px-2 py-1">
      <div className="flex-1 items-center mr-2">
        <input
          onChange={event => {
            let newName = ReactEvent.Form.target(event)##value;
            send(UpdateCategoryName(newName));
          }}
          value={state.categoryName}
          className="text-sm mr-1 font-semibold px-2 py-1 w-full outline-none"
        />
      </div>
      <div>
        {presentCategoryName == state.categoryName
           ? <span
               className={
                 "text-xs py-1 px-2 mr-2 " ++ topicsCountPillClass(category)
               }>
               {string_of_int(Category.topicsCount(category))
                ++ " topics"
                |> str}
             </span>
           : <button
               disabled={saveDisabled(state.categoryName, state.saving)}
               onClick={updateCategory(
                 category,
                 state.categoryName,
                 updateCategoryCB,
                 send,
               )}
               className="btn btn-success mr-2 text-xs">
               {"Update Category" |> str}
             </button>}
        <button
          onClick={deleteCategory(categoryId, deleteCategoryCB, send)}
          className="text-xs py-1 px-2 h-8 text-gray-700 hover:text-gray-900 hover:bg-gray-100 border-l border-gray-400">
          <FaIcon
            classes={
              state.deleting ? "fas fa-spinner fa-spin" : "fas fa-trash-alt"
            }
          />
        </button>
      </div>
    </div>;
  | None =>
    <div className="flex mt-2">
      <input
        onChange={event => {
          let name = ReactEvent.Form.target(event)##value;
          send(UpdateCategoryName(name));
        }}
        value={state.categoryName}
        placeholder="Add new category"
        className="appearance-none h-10 block w-full text-gray-700 border rounded border-gray-400 py-2 px-4 text-sm bg-gray-100 hover:bg-gray-200 focus:outline-none focus:bg-white focus:border-primary-400"
      />
      {let showButton = state.categoryName |> String.trim != "";
       showButton
         ? <button
             disabled={saveDisabled(state.categoryName, state.saving)}
             onClick={createCategory(
               communityId,
               state.categoryName,
               createCategoryCB,
               send,
             )}
             className="btn btn-success ml-2 text-sm">
             {"Save Category" |> str}
           </button>
         : React.null}
    </div>
  };
};
