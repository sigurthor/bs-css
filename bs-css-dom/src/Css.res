include Css_Legacy_Core
include Css_Colors

include Css_Legacy_Core.Make({
  type styleEncoding = ReactDOM.Style.t
  type renderer = Js.Json.t // not relevant
  exception NotImplemented

  external unsafeJsonCast: Js.Json.t => styleEncoding = "%identity"
  external unsafeJsCast: Js.t<'a> => styleEncoding = "%identity"

  let injectRaw = (. _) => throw(NotImplemented)
  let renderRaw = (. _, _) => throw(NotImplemented)

  let injectRules = (. _, _) => throw(NotImplemented)
  let renderRules = (. _, _, _) => throw(NotImplemented)

  let mergeStyles = (. styles) =>
    Belt.Array.reduce(styles, Js.Obj.empty(), (acc, item) =>
      Js.Obj.assign(acc, Obj.magic(item))
    )->unsafeJsCast

  let make = (. props) => props->unsafeJsonCast

  let makeKeyframes = (. _) => throw(NotImplemented)
  let renderKeyframes = (. _, _) => throw(NotImplemented)
})
