include Css_Colors;
module Types = Css_Types;

type rule = [
  | `selector(string, list(rule))
  | `declaration(string, string)
  | `animation(string)
  | `transition(string)
  | `shadow(string)
  | `textShadow(string)
];

type selector = [ | `selector(string, list(rule))];
type declaration = [ | `declaration(string, string)];

module Emotion = {
  type stylename = string;
  type cache;
  [@bs.module "emotion"] external _make: Js.Json.t => stylename = "css";
  [@bs.module "emotion"] external cache: cache = "cache";
  [@bs.module "emotion"]
  external injectGlobal: Js.Json.t => unit = "injectGlobal";
  [@bs.module "emotion"]
  external rawInjectGlobal: string => unit = "injectGlobal";
  [@bs.module "emotion"]
  external makeKeyFrames: Js.Dict.t(Js.Json.t) => string = "keyframes";
  [@bs.module "emotion"] external cx: array(stylename) => stylename = "cx";
  let mergeStyles: list(stylename) => stylename =
    stylenames => stylenames |> Array.of_list |> cx;

  let rec ruleToJs = rule =>
    switch (rule) {
    | `declaration(name, value) when name == "content" => (
        name,
        Js.Json.string(value == "" ? "\"\"" : value),
      )
    | `declaration(name, value) => (name, Js.Json.string(value))
    | `selector(name, ruleset) => (name, makeJson(ruleset))
    | `shadow(value) => ("boxShadow", Js.Json.string(value))
    | `textShadow(value) => ("textShadow", Js.Json.string(value))
    | `transition(value) => ("transition", Js.Json.string(value))
    | `animation(value) => ("animation", Js.Json.string(value))
    }

  and makeJson = rules =>
    List.map(ruleToJs, rules)->Js.Dict.fromList->Js.Json.object_;

  let make = rules => rules->makeJson->_make;
};

let toJson = Emotion.makeJson;
let style = Emotion.make;

let join = (separator, strings) => {
  let rec run = (acc, strings) =>
    switch (strings) {
    | [] => acc
    | [x] => acc ++ x
    | [x, ...xs] => run(acc ++ x ++ separator, xs)
    };
  run("", strings);
};

let string_of_float = Js.Float.toString;
let string_of_int = Js.Int.toString;

module Converter = {
  let string_of_rgb = (r, g, b) =>
    "rgb("
    ++ string_of_int(r)
    ++ ", "
    ++ string_of_int(g)
    ++ ", "
    ++ string_of_int(b)
    ++ ")";

  let string_of_rgba = (r, g, b, a) =>
    "rgba("
    ++ string_of_int(r)
    ++ ", "
    ++ string_of_int(g)
    ++ ", "
    ++ string_of_int(b)
    ++ ", "
    ++ string_of_float(a)
    ++ ")";

  let string_of_percent =
    fun
    | `percent(x) => Js.Float.toString(x) ++ "%";

  let string_of_hsl = (h, s, l) =>
    "hsl("
    ++ Types.Angle.toString(h)
    ++ ", "
    ++ string_of_percent(s)
    ++ ", "
    ++ string_of_percent(l)
    ++ ")";

  let string_of_alpha =
    fun
    | `num(f) => Js.Float.toString(f)
    | `percent(p) => Js.Float.toString(p) ++ "%";

  let string_of_hsla = (h, s, l, a) =>
    "hsla("
    ++ Types.Angle.toString(h)
    ++ ", "
    ++ string_of_percent(s)
    ++ ", "
    ++ string_of_percent(l)
    ++ ", "
    ++ string_of_alpha(a)
    ++ ")";

  let string_of_color =
    fun
    | `rgb(r, g, b) => string_of_rgb(r, g, b)
    | `rgba(r, g, b, a) => string_of_rgba(r, g, b, a)
    | `hsl(h, s, l) => string_of_hsl(h, s, l)
    | `hsla(h, s, l, a) => string_of_hsla(h, s, l, a)
    | `hex(s) => "#" ++ s
    | `transparent => "transparent"
    | `currentColor => "currentColor";

  let string_of_stops = stops =>
    stops
    |> List.map(((l, c)) =>
         join(" ", [string_of_color(c), Types.Length.toString(l)])
       )
    |> join(", ");

  let string_of_linearGradient = (angle, stops) =>
    "linear-gradient("
    ++ Types.Angle.toString(angle)
    ++ ", "
    ++ string_of_stops(stops)
    ++ ")";

  let string_of_repeatingLinearGradient = (angle, stops) =>
    "repeating-linear-gradient("
    ++ Types.Angle.toString(angle)
    ++ ", "
    ++ string_of_stops(stops)
    ++ ")";

  let string_of_translate3d = (x, y, z) =>
    "translate3d("
    ++ Types.Length.toString(x)
    ++ ", "
    ++ Types.Length.toString(y)
    ++ ", "
    ++ Types.Length.toString(z)
    ++ ")";

  let string_of_scale = (x, y) =>
    "scale(" ++ string_of_float(x) ++ ", " ++ string_of_float(y) ++ ")";

  let string_of_time = t => string_of_int(t) ++ "ms";

  let string_of_overflow =
    fun
    | `auto => "auto"
    | `scroll => "scroll"
    | `hidden => "hidden"
    | `visible => "visible";

  let string_of_visibility =
    fun
    | `hidden => "hidden"
    | `visible => "visible";

  let string_of_background = bg =>
    switch (bg) {
    | `none => "none"
    | `url(url) => "url(" ++ url ++ ")"
    | `rgb(r, g, b) =>
      "rgb("
      ++ join(
           ", ",
           [string_of_int(r), string_of_int(g), string_of_int(b)],
         )
      ++ ")"
    | `rgba(r, g, b, a) =>
      "rgba("
      ++ join(
           ", ",
           [
             string_of_int(r),
             string_of_int(g),
             string_of_int(b),
             string_of_float(a),
           ],
         )
      ++ ")"
    | `hsl(h, s, l) => string_of_hsl(h, s, l)
    | `hsla(h, s, l, a) => string_of_hsla(h, s, l, a)
    | `hex(s) => "#" ++ s
    | `transparent => "transparent"
    | `currentColor => "currentColor"
    | `linearGradient(angle, stops) =>
      "linear-gradient("
      ++ Types.Angle.toString(angle)
      ++ ", "
      ++ string_of_stops(stops)
      ++ ")"
    | `repeatingLinearGradient(angle, stops) =>
      "repeating-linear-gradient("
      ++ Types.Angle.toString(angle)
      ++ ", "
      ++ string_of_stops(stops)
      ++ ")"
    | `radialGradient(stops) =>
      "radial-gradient(" ++ string_of_stops(stops) ++ ")"
    | `repeatingRadialGradient(stops) =>
      "repeating-radial-gradient(" ++ string_of_stops(stops) ++ ")"
    };

  let string_of_cursor = x =>
    switch (x) {
    | `auto => "auto"
    | `default => "default"
    | `none => "none"
    | `contextMenu => "context-menu"
    | `help => "help"
    | `pointer => "pointer"
    | `progress => "progress"
    | `wait => "wait"
    | `cell => "cell"
    | `crosshair => "crosshair"
    | `text => "text"
    | `verticalText => "vertical-text"
    | `alias => "alias"
    | `copy => "copy"
    | `move => "move"
    | `noDrop => "no-drop"
    | `notAllowed => "not-allowed"
    | `grab => "grab"
    | `grabbing => "grabbing"
    | `allScroll => "all-scroll"
    | `colResize => "col-resize"
    | `rowResize => "row-resize"
    | `nResize => "n-resize"
    | `eResize => "e-resize"
    | `sResize => "s-resize"
    | `wResize => "w-resize"
    | `neResize => "ne-resize"
    | `nwResize => "nw-resize"
    | `seResize => "se-resize"
    | `swResize => "sw-resize"
    | `ewResize => "ew-resize"
    | `nsResize => "ns-resize"
    | `neswResize => "nesw-resize"
    | `nwseResize => "nwse-resize"
    | `zoomIn => "zoom-in"
    | `zoomOut => "zoom-out"
    };

  let string_of_fontWeight = x =>
    switch (x) {
    | `num(n) => string_of_int(n)
    | `thin => "100"
    | `extraLight => "200"
    | `light => "300"
    | `normal => "400"
    | `medium => "500"
    | `semiBold => "600"
    | `bold => "700"
    | `extraBold => "800"
    | `black => "900"
    | `lighter => "lighter"
    | `bolder => "bolder"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    };

  let string_of_flex =
    fun
    | `auto => "auto"
    | `initial => "initial"
    | `none => "none"
    | `num(n) => string_of_float(n);
  let string_of_flexBasis =
    fun
    | `calc(`add, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " + "
      ++ Types.Length.toString(b)
      ++ ")"
    | `calc(`sub, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " - "
      ++ Types.Length.toString(b)
      ++ ")"
    | `ch(x) => string_of_float(x) ++ "ch"
    | `cm(x) => string_of_float(x) ++ "cm"
    | `em(x) => string_of_float(x) ++ "em"
    | `ex(x) => string_of_float(x) ++ "ex"
    | `mm(x) => string_of_float(x) ++ "mm"
    | `percent(x) => string_of_float(x) ++ "%"
    | `pt(x) => string_of_int(x) ++ "pt"
    | `px(x) => string_of_int(x) ++ "px"
    | `pxFloat(x) => string_of_float(x) ++ "px"
    | `rem(x) => string_of_float(x) ++ "rem"
    | `vh(x) => string_of_float(x) ++ "vh"
    | `vmax(x) => string_of_float(x) ++ "vmax"
    | `vmin(x) => string_of_float(x) ++ "vmin"
    | `vw(x) => string_of_float(x) ++ "vw"
    | `zero => "0"
    | `auto => "auto"
    | `fill => "fill"
    | `content => "content"
    | `maxContent => "max-content"
    | `minContent => "min-content"
    | `fitContent => "fit-content";
};
include Converter;

type cache = Emotion.cache;

let empty = [];
let cache = Emotion.cache;
let merge = Emotion.mergeStyles;
let global = (selector, rules: list(rule)) =>
  Emotion.injectGlobal(
    [(selector, Emotion.makeJson(rules))]
    ->Js.Dict.fromList
    ->Js.Json.object_,
  );

let insertRule = raw => Emotion.rawInjectGlobal(raw);

type animation = string;

let keyframes = frames => {
  let addStop = (dict, (stop, rules)) => {
    Js.Dict.set(dict, string_of_int(stop) ++ "%", Emotion.makeJson(rules));
    dict;
  };
  Emotion.makeKeyFrames @@ List.fold_left(addStop, Js.Dict.empty(), frames);
};

let important = v =>
  switch (v) {
  | `declaration(name, value) => `declaration((name, value ++ " !important"))
  | _ => v
  };

let label = label => `declaration(("label", label));

/* Properties */

let bottom = x =>
  `declaration((
    "bottom",
    switch (x) {
    | #Types.Length.t as l => Types.Length.toString(l)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let direction = x =>
  `declaration((
    "direction",
    switch (x) {
    | #Types.Direction.t as d => Types.Direction.toString(d)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let fontFamily = x => `declaration(("fontFamily", x));

let fontSize = x =>
  `declaration((
    "fontSize",
    switch (x) {
    | #Types.Length.t as l => Types.Length.toString(l)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let fontStyle = x =>
  `declaration((
    "fontStyle",
    switch (x) {
    | #Types.FontStyle.t as f => Types.FontStyle.toString(f)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let fontVariant = x =>
  `declaration((
    "fontVariant",
    switch (x) {
    | #Types.FontVariant.t as f => Types.FontVariant.toString(f)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let left = x =>
  `declaration((
    "left",
    switch (x) {
    | #Types.Length.t as l => Types.Length.toString(l)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let position = x =>
  `declaration((
    "position",
    switch (x) {
    | #Types.Position.t as p => Types.Position.toString(p)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let resize = x =>
  `declaration((
    "resize",
    switch (x) {
    | #Types.Resize.t as r => Types.Resize.toString(r)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let right = x =>
  `declaration((
    "right",
    switch (x) {
    | #Types.Length.t as l => Types.Length.toString(l)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let top = x =>
  `declaration((
    "top",
    switch (x) {
    | #Types.Length.t as l => Types.Length.toString(l)
    | #Types.Cascading.t as c => Types.Cascading.toString(c)
    },
  ));

let unsafe = (property, value) => `declaration((property, value));

let zIndex = x => `declaration(("zIndex", Js.Int.toString(x)));

/* Type aliasing */

type cascading = Types.Cascading.t;
type length = Types.Length.t;
type angle = Types.Angle.t;
type fontStyle = Types.FontStyle.t;

/* Constructor aliases */

let initial = Types.Cascading.initial;
let inherit_ = Types.Cascading.inherit_;
let unset = Types.Cascading.unset;

let ch = Types.Length.ch;
let cm = Types.Length.cm;
let em = Types.Length.em;
let ex = Types.Length.ex;
let mm = Types.Length.mm;
let pt = Types.Length.pt;
let px = Types.Length.px;
let pxFloat = Types.Length.pxFloat;
let rem = Types.Length.rem;
let vh = Types.Length.vh;
let vmin = Types.Length.vmin;
let vmax = Types.Length.vmax;
let zero = Types.Length.zero;

let deg = Types.Angle.deg;
let rad = Types.Angle.rad;
let grad = Types.Angle.grad;
let turn = Types.Angle.turn;

let ltr = Types.Direction.ltr;
let rtl = Types.Direction.rtl;

let absolute = Types.Position.absolute;
let relative = Types.Position.relative;
let static = Types.Position.static;
let fixed = `fixed;
let sticky = Types.Position.sticky;

//let none = Types.Resize.none;
//let both = Types.Resize.both;
let horizontal = Types.Resize.horizontal;
let vertical = Types.Resize.vertical;
//let block = Types.Resize.block;
//let inline = Types.Resize.inline;

let smallCaps = Types.FontVariant.smallCaps;

//let normal = `normal;
let italic = Types.FontStyle.italic;
let oblique = Types.FontStyle.oblique;

/********************************************************
 ************************ VALUES ************************
 ********************************************************/

let pct = x => `percent(x);

type color = [
  | `rgb(int, int, int)
  | `rgba(int, int, int, float)
  | `hsl(angle, [ | `percent(float)], [ | `percent(float)])
  | `hsla(
      angle,
      [ | `percent(float)],
      [ | `percent(float)],
      [ | `num(float) | `percent(float)],
    )
  | `hex(string)
  | `transparent
  | `currentColor
];

let rgb = (r, g, b) => `rgb((r, g, b));
let rgba = (r, g, b, a) => `rgba((r, g, b, a));
let hsl = (h, s, l) => `hsl((h, pct(s), pct(l)));
let hsla = (h, s, l, a) => `hsla((h, pct(s), pct(l), a));
let hex = x => `hex(x);

let currentColor = `currentColor;

type gradient = [
  | `linearGradient(angle, list((length, color)))
  | `repeatingLinearGradient(angle, list((length, color)))
  | `radialGradient(list((length, color)))
  | `repeatingRadialGradient(list((length, color)))
];

let linearGradient = (angle, stops) => `linearGradient((angle, stops));

let repeatingLinearGradient = (angle, stops) =>
  `repeatingLinearGradient((angle, stops));

let radialGradient = stops => `radialGradient(stops);

let repeatingRadialGradient = stops => `repeatingRadialGradient(stops);

/**
 * Units
 */

let vw = x => `vw(x);
let fr = x => `fr(x);

module Calc = {
  let (-) = (a, b) => `calc((`sub, a, b));
  let (+) = (a, b) => `calc((`add, a, b));
};
let size = (x, y) => `size((x, y));

/**
 * Misc
 */

let all = `all;
let auto = `auto;
let backwards = `backwards;
let baseline = `baseline;
let block = `block;
let borderBox = `borderBox;
let both = `both;
let center = `center;
let column = `column;
let columnReverse = `columnReverse;
let contain = `contain;
let contentBox = `contentBox;
let count = x => `count(x);
let cover = `cover;
let cubicBesier = (a, b, c, d) => `cubicBezier((a, b, c, d));
let dashed = `dashed;
let dotted = `dotted;
let ease = `ease;
let easeIn = `easeIn;
let easeInOut = `easeInOut;
let easeOut = `easeOut;
let flexBox = `flex;
let grid = `grid;
let inlineGrid = `inlineGrid;
let flexEnd = `flexEnd;
let flexStart = `flexStart;
let forwards = `forwards;
let hidden = `hidden;
let infinite = `infinite;

let inline = `inline;
let block = `block;
let contents = `contents;
let flexBox = `flex;
let grid = `grid;
let inlineBlock = `inlineBlock;
let inlineFlex = `inlineFlex;
let inlineGrid = `inlineGrid;
let inlineTable = `inlineTable;
let listItem = `listItem;
let runIn = `runIn;
let table = `table;
let tableCaption = `tableCaption;
let tableColumnGroup = `tableColumnGroup;
let tableHeaderGroup = `tableHeaderGroup;
let tableFooterGroup = `tableFooterGroup;
let tableRowGroup = `tableRowGroup;
let tableCell = `tableCell;
let tableColumn = `tableColumn;
let tableRow = `tableRow;

let linear = `linear;
let local = `local;
let localUrl = x => `localUrl(x);
let none = `none;
let noRepeat = `noRepeat;
let nowrap = `nowrap;
let paddingBox = `paddingBox;
let paused = `paused;
let repeat = `repeat;
let minmax = `minmax;
let repeatX = `repeatX;
let repeatY = `repeatY;
let rotate = a => `rotate(a);
let rotate3d = (x, y, z, a) => `rotate3d((x, y, z, a));
let rotateX = a => `rotateX(a);
let rotateY = a => `rotateY(a);
let rotateZ = a => `rotateZ(a);
let row = `row;
let rowReverse = `rowReverse;
let running = `running;
let scale = (x, y) => `scale((x, y));
let scale3d = (x, y, z) => `scale3d((x, y, z));
let scaleX = x => `scaleX(x);
let scaleY = x => `scaleY(x);
let scaleZ = x => `scaleZ(x);
let scroll = `scroll;
let skew = (x, y) => `skew((x, y));
let skewX = a => `skewX(a);
let skewY = a => `skewY(a);
let solid = `solid;
let spaceAround = `spaceAround;
let spaceBetween = `spaceBetween;
let spaceEvenly = `spaceEvenly;
let stepEnd = `stepEnd;
let steps = (i, dir) => `steps((i, dir));
let stepStart = `stepStart;
let stretch = `stretch;
let text = `text;
let translate = (x, y) => `translate((x, y));
let translate3d = (x, y, z) => `translate3d((x, y, z));
let translateX = x => `translateX(x);
let translateY = y => `translateY(y);
let translateZ = z => `translateZ(z);
let url = x => `url(x);
let visible = `visible;
let wrap = `wrap;
let wrapReverse = `wrapReverse;

let inside = `inside;
let outside = `outside;

let underline = `underline;
let overline = `overline;
let lineThrough = `lineThrough;
let clip = `clip;
let ellipsis = `ellipsis;

let wavy = `wavy;
let double = `double;

let uppercase = `uppercase;
let lowercase = `lowercase;
let capitalize = `capitalize;

let sub = `sub;
let super = `super;
let textTop = `textTop;
let textBottom = `textBottom;
let middle = `middle;

let normal = `normal;

let breakAll = `breakAll;
let keepAll = `keepAll;
let breakWord = `breakWord;

let reverse = `reverse;
let alternate = `alternate;
let alternateReverse = `alternateReverse;

let fill = `fill;
let content = `content;
let maxContent = `maxContent;
let minContent = `minContent;
let fitContent = `fitContent;

let round = `round;
let miter = `miter;
let bevel = `bevel;
let butt = `butt;
let square = `square;

/********************************************************
 ******************** PROPERTIES ************************
 ********************************************************/

/**
 * Layout
 */

let display = x =>
  `declaration((
    "display",
    switch (x) {
    | `unset => "unset"
    | `inline => "inline"
    | `block => "block"
    | `contents => "contents"
    | `flex => "flex"
    | `grid => "grid"
    | `inlineBlock => "inline-block"
    | `inlineFlex => "inline-flex"
    | `inlineGrid => "inline-grid"
    | `inlineTable => "inline-table"
    | `listItem => "list-item"
    | `runIn => "run-in"
    | `table => "table"
    | `tableCaption => "table-caption"
    | `tableColumnGroup => "table-column-group"
    | `tableHeaderGroup => "table-header-group"
    | `tableFooterGroup => "table-footer-group"
    | `tableRowGroup => "table-row-group"
    | `tableCell => "table-cell"
    | `tableColumn => "table-column"
    | `tableRow => "table-row"
    | `none => "none"
    | `initial => "initial"
    | `inherit_ => "inherit"
    },
  ));

let flex = x => `declaration(("flex", string_of_flex(x)));
let flex3 = (~grow, ~shrink, ~basis) =>
  `declaration((
    "flex",
    string_of_float(grow)
    ++ " "
    ++ string_of_float(shrink)
    ++ " "
    ++ string_of_flexBasis(basis),
  ));
let flexGrow = x => `declaration(("flexGrow", string_of_float(x)));
let flexShrink = x => `declaration(("flexShrink", string_of_float(x)));
let flexBasis = x => `declaration(("flexBasis", string_of_flexBasis(x)));

let flexDirection = x =>
  `declaration((
    "flexDirection",
    switch (x) {
    | `row => "row"
    | `column => "column"
    | `rowReverse => "row-reverse"
    | `columnReverse => "column-reverse"
    },
  ));

let flexWrap = x =>
  `declaration((
    "flexWrap",
    switch (x) {
    | `nowrap => "nowrap"
    | `wrap => "wrap"
    | `wrapReverse => "wrap-reverse"
    },
  ));

let order = x => `declaration(("order", string_of_int(x)));

let string_of_margin =
  fun
  | `calc(`add, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " + "
    ++ Types.Length.toString(b)
    ++ ")"
  | `calc(`sub, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " - "
    ++ Types.Length.toString(b)
    ++ ")"
  | `ch(x) => string_of_float(x) ++ "ch"
  | `cm(x) => string_of_float(x) ++ "cm"
  | `em(x) => string_of_float(x) ++ "em"
  | `ex(x) => string_of_float(x) ++ "ex"
  | `mm(x) => string_of_float(x) ++ "mm"
  | `percent(x) => string_of_float(x) ++ "%"
  | `pt(x) => string_of_int(x) ++ "pt"
  | `px(x) => string_of_int(x) ++ "px"
  | `pxFloat(x) => string_of_float(x) ++ "px"
  | `rem(x) => string_of_float(x) ++ "rem"
  | `vh(x) => string_of_float(x) ++ "vh"
  | `vmax(x) => string_of_float(x) ++ "vmax"
  | `vmin(x) => string_of_float(x) ++ "vmin"
  | `vw(x) => string_of_float(x) ++ "vw"
  | `zero => "0"
  | `auto => "auto";

let margin = x => `declaration(("margin", string_of_margin(x)));
let margin2 = (~v, ~h) =>
  `declaration((
    "margin",
    string_of_margin(v) ++ " " ++ string_of_margin(h),
  ));
let margin3 = (~top, ~h, ~bottom) =>
  `declaration((
    "margin",
    string_of_margin(top)
    ++ " "
    ++ string_of_margin(h)
    ++ " "
    ++ string_of_margin(bottom),
  ));
let margin4 = (~top, ~right, ~bottom, ~left) =>
  `declaration((
    "margin",
    string_of_margin(top)
    ++ " "
    ++ string_of_margin(right)
    ++ " "
    ++ string_of_margin(bottom)
    ++ " "
    ++ string_of_margin(left),
  ));
let marginLeft = x => `declaration(("marginLeft", string_of_margin(x)));
let marginRight = x => `declaration(("marginRight", string_of_margin(x)));
let marginTop = x => `declaration(("marginTop", string_of_margin(x)));
let marginBottom = x => `declaration(("marginBottom", string_of_margin(x)));

let padding = x => `declaration(("padding", Types.Length.toString(x)));
let padding2 = (~v, ~h) =>
  `declaration((
    "padding",
    Types.Length.toString(v) ++ " " ++ Types.Length.toString(h),
  ));
let padding3 = (~top, ~h, ~bottom) =>
  `declaration((
    "padding",
    Types.Length.toString(top)
    ++ " "
    ++ Types.Length.toString(h)
    ++ " "
    ++ Types.Length.toString(bottom),
  ));
let padding4 = (~top, ~right, ~bottom, ~left) =>
  `declaration((
    "padding",
    Types.Length.toString(top)
    ++ " "
    ++ Types.Length.toString(right)
    ++ " "
    ++ Types.Length.toString(bottom)
    ++ " "
    ++ Types.Length.toString(left),
  ));
let paddingLeft = x =>
  `declaration(("paddingLeft", Types.Length.toString(x)));
let paddingRight = x =>
  `declaration(("paddingRight", Types.Length.toString(x)));
let paddingTop = x => `declaration(("paddingTop", Types.Length.toString(x)));
let paddingBottom = x =>
  `declaration(("paddingBottom", Types.Length.toString(x)));

let string_of_minmax =
  fun
  | `auto => "auto"
  | `calc(`add, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " + "
    ++ Types.Length.toString(b)
    ++ ")"
  | `calc(`sub, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " - "
    ++ Types.Length.toString(b)
    ++ ")"
  | `ch(x) => string_of_float(x) ++ "ch"
  | `cm(x) => string_of_float(x) ++ "cm"
  | `em(x) => string_of_float(x) ++ "em"
  | `ex(x) => string_of_float(x) ++ "ex"
  | `mm(x) => string_of_float(x) ++ "mm"
  | `percent(x) => string_of_float(x) ++ "%"
  | `pt(x) => string_of_int(x) ++ "pt"
  | `px(x) => string_of_int(x) ++ "px"
  | `pxFloat(x) => string_of_float(x) ++ "px"
  | `rem(x) => string_of_float(x) ++ "rem"
  | `vh(x) => string_of_float(x) ++ "vh"
  | `vmax(x) => string_of_float(x) ++ "vmax"
  | `vmin(x) => string_of_float(x) ++ "vmin"
  | `vw(x) => string_of_float(x) ++ "vw"
  | `fr(x) => string_of_float(x) ++ "fr"
  | `zero => "0"
  | `minContent => "min-content"
  | `maxContent => "max-content";

let string_of_dimension =
  fun
  | `auto => "auto"
  | `none => "none"
  | `calc(`add, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " + "
    ++ Types.Length.toString(b)
    ++ ")"
  | `calc(`sub, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " - "
    ++ Types.Length.toString(b)
    ++ ")"
  | `ch(x) => string_of_float(x) ++ "ch"
  | `cm(x) => string_of_float(x) ++ "cm"
  | `em(x) => string_of_float(x) ++ "em"
  | `ex(x) => string_of_float(x) ++ "ex"
  | `mm(x) => string_of_float(x) ++ "mm"
  | `percent(x) => string_of_float(x) ++ "%"
  | `pt(x) => string_of_int(x) ++ "pt"
  | `px(x) => string_of_int(x) ++ "px"
  | `pxFloat(x) => string_of_float(x) ++ "px"
  | `rem(x) => string_of_float(x) ++ "rem"
  | `vh(x) => string_of_float(x) ++ "vh"
  | `vmax(x) => string_of_float(x) ++ "vmax"
  | `vmin(x) => string_of_float(x) ++ "vmin"
  | `vw(x) => string_of_float(x) ++ "vw"
  | `fr(x) => string_of_float(x) ++ "fr"
  | `zero => "0"
  | `fitContent => "fit-content"
  | `minContent => "min-content"
  | `maxContent => "max-content"
  | `minmax(a, b) =>
    "minmax(" ++ string_of_minmax(a) ++ "," ++ string_of_minmax(b) ++ ")";

let width = x => `declaration(("width", string_of_dimension(x)));
let height = x => `declaration(("height", string_of_dimension(x)));
let minWidth = x => `declaration(("minWidth", string_of_dimension(x)));
let maxWidth = x => `declaration(("maxWidth", string_of_dimension(x)));
let minHeight = x => `declaration(("minHeight", string_of_dimension(x)));
let maxHeight = x => `declaration(("maxHeight", string_of_dimension(x)));

type gridAutoDirection = [
  | `column
  | `row
  | `columnDense
  | `rowDense
  | `initial
  | cascading
];

let gridAutoDirectionToJs =
  fun
  | `column => "column"
  | `row => "row"
  | `columnDense => "column dense"
  | `rowDense => "row dense"
  | `initial => "initial"
  | `inherit_ => "inherit"
  | `unset => "unset";

let gridAutoFlow = direction =>
  `declaration(("gridAutoFlow", gridAutoDirectionToJs(direction)));

type repeatValue = [ | `autoFill | `autoFit | `num(int)];
let repeatValueToJs =
  fun
  | `autoFill => "auto-fill"
  | `autoFit => "auto-fit"
  | `num(x) => x->string_of_int;

type minmax = [ | `fr(float) | `minContent | `maxContent | `auto | length];

type trackLength = [
  length
  | `fr(float)
  | `minContent
  | `maxContent
  | `minmax(minmax, minmax)
];
type gridLength = [ trackLength | `repeat(repeatValue, trackLength)];

let gridLengthToJs =
  fun
  | `auto => "auto"
  | `calc(`add, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " + "
    ++ Types.Length.toString(b)
    ++ ")"
  | `calc(`sub, a, b) =>
    "calc("
    ++ Types.Length.toString(a)
    ++ " - "
    ++ Types.Length.toString(b)
    ++ ")"
  | `ch(x) => string_of_float(x) ++ "ch"
  | `cm(x) => string_of_float(x) ++ "cm"
  | `em(x) => string_of_float(x) ++ "em"
  | `ex(x) => string_of_float(x) ++ "ex"
  | `mm(x) => string_of_float(x) ++ "mm"
  | `percent(x) => string_of_float(x) ++ "%"
  | `pt(x) => string_of_int(x) ++ "pt"
  | `px(x) => string_of_int(x) ++ "px"
  | `pxFloat(x) => string_of_float(x) ++ "px"
  | `rem(x) => string_of_float(x) ++ "rem"
  | `vh(x) => string_of_float(x) ++ "vh"
  | `vmax(x) => string_of_float(x) ++ "vmax"
  | `vmin(x) => string_of_float(x) ++ "vmin"
  | `vw(x) => string_of_float(x) ++ "vw"
  | `fr(x) => string_of_float(x) ++ "fr"
  | `zero => "0"
  | `minContent => "min-content"
  | `maxContent => "max-content"
  | `repeat(n, x) =>
    "repeat(" ++ n->repeatValueToJs ++ ", " ++ string_of_dimension(x) ++ ")"
  | `minmax(a, b) =>
    "minmax(" ++ string_of_minmax(a) ++ "," ++ string_of_minmax(b) ++ ")";

let string_of_dimensions = dimensions =>
  dimensions |> List.map(gridLengthToJs) |> String.concat(" ");

let gridTemplateColumns = dimensions =>
  `declaration(("gridTemplateColumns", string_of_dimensions(dimensions)));

let gridTemplateRows = dimensions =>
  `declaration(("gridTemplateRows", string_of_dimensions(dimensions)));

let gridAutoColumns = dimensions =>
  `declaration(("gridAutoColumns", string_of_dimension(dimensions)));

let gridAutoRows = dimensions =>
  `declaration(("gridAutoRows", string_of_dimension(dimensions)));

let gridColumn = (start, end') =>
  `declaration((
    "gridColumn",
    string_of_int(start) ++ " / " ++ string_of_int(end'),
  ));

let gridRow = (start, end') =>
  `declaration((
    "gridRow",
    string_of_int(start) ++ " / " ++ string_of_int(end'),
  ));
let gridColumnStart = n =>
  `declaration(("gridColumnStart", string_of_int(n)));
let gridColumnEnd = n => `declaration(("gridColumnEnd", string_of_int(n)));
let gridRowStart = n => `declaration(("gridRowStart", string_of_int(n)));
let gridRowEnd = n => `declaration(("gridRowEnd", string_of_int(n)));
let gridColumnGap = n =>
  `declaration(("gridColumnGap", Types.Length.toString(n)));
let gridRowGap = n => `declaration(("gridRowGap", Types.Length.toString(n)));
let gridGap = n => `declaration(("gridGap", Types.Length.toString(n)));

let string_of_align =
  fun
  | `baseline => "baseline"
  | `flexStart => "flex-start"
  | `flexEnd => "flex-end"
  | `center => "center"
  | `auto => "auto"
  | `stretch => "stretch";
let alignItems = x => `declaration(("alignItems", string_of_align(x)));
let alignSelf = x => `declaration(("alignSelf", string_of_align(x)));

let string_of_justify =
  fun
  | `flexStart => "flex-start"
  | `flexEnd => "flex-end"
  | `center => "center"
  | `spaceAround => "space-around"
  | `spaceBetween => "space-between"
  | `spaceEvenly => "space-evenly"
  | `stretch => "stretch";
let justifyContent = x =>
  `declaration(("justifyContent", string_of_justify(x)));
let justifySelf = x => `declaration(("justifySelf", string_of_justify(x)));
let alignContent = x => `declaration(("alignContent", string_of_justify(x)));

let boxSizing = x =>
  `declaration((
    "boxSizing",
    switch (x) {
    | `contentBox => "content-box"
    | `borderBox => "border-box"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let float = x =>
  `declaration((
    "float",
    switch (x) {
    | `left => "left"
    | `right => "right"
    | `none => "none"
    },
  ));

let clear = x =>
  `declaration((
    "clear",
    switch (x) {
    | `left => "left"
    | `right => "right"
    | `both => "both"
    },
  ));

let overflow = x => `declaration(("overflow", string_of_overflow(x)));
let overflowX = x => `declaration(("overflowX", string_of_overflow(x)));
let overflowY = x => `declaration(("overflowY", string_of_overflow(x)));

let contentRule = x => `declaration(("content", {j|"$x"|j}));

let columnCount = x =>
  `declaration((
    "columnCount",
    switch (x) {
    | `auto => "auto"
    | `count(v) => string_of_int(v)
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let objectFit = x =>
  `declaration((
    "objectFit",
    switch (x) {
    | `fill => "fill"
    | `contain => "contain"
    | `cover => "cover"
    | `none => "none"
    | `scaleDown => "scale-down"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

type filter = [
  | `blur(length)
  | `brightness(float)
  | `contrast(float)
  | `dropShadow(length, length, length, color)
  | `grayscale(float)
  | `hueRotate(angle)
  | `invert(float)
  | `opacity(float)
  | `saturate(float)
  | `sepia(float)
  | `url(string)
  | `none
  | cascading
];

let string_of_filter =
  fun
  | `blur(v) => "blur(" ++ Types.Length.toString(v) ++ ")"
  | `brightness(v) => "brightness(" ++ string_of_float(v) ++ "%)"
  | `contrast(v) => "contrast(" ++ string_of_float(v) ++ "%)"
  | `dropShadow(a, b, c, d) =>
    "drop-shadow("
    ++ Types.Length.toString(a)
    ++ " "
    ++ Types.Length.toString(b)
    ++ " "
    ++ Types.Length.toString(c)
    ++ " "
    ++ Converter.string_of_color(d)
    ++ ")"
  | `grayscale(v) => "grayscale(" ++ string_of_float(v) ++ "%)"
  | `hueRotate(v) => "hue-rotate(" ++ Types.Angle.toString(v) ++ ")"
  | `invert(v) => "invert(" ++ string_of_float(v) ++ "%)"
  | `opacity(v) => "opacity(" ++ string_of_float(v) ++ "%)"
  | `saturate(v) => "saturate(" ++ string_of_float(v) ++ "%)"
  | `sepia(v) => "sepia(" ++ string_of_float(v) ++ "%)"
  | `url(v) => "url(" ++ v ++ ")"
  | `initial => "initial"
  | `inherit_ => "inherit"
  | `unset => "unset"
  | `none => "none";

let filter = x =>
  `declaration(("filter", x |> List.map(string_of_filter) |> join(" ")));
/**
 * Style
 */

let backfaceVisibility = x =>
  `declaration(("backfaceVisibility", string_of_visibility(x)));

let visibility = x => `declaration(("visibility", string_of_visibility(x)));

let boxShadow =
    (~x=zero, ~y=zero, ~blur=zero, ~spread=zero, ~inset=false, color) =>
  `shadow(
    Types.Length.toString(x)
    ++ " "
    ++ Types.Length.toString(y)
    ++ " "
    ++ Types.Length.toString(blur)
    ++ " "
    ++ Types.Length.toString(spread)
    ++ " "
    ++ string_of_color(color)
    ++ " "
    ++ (inset ? "inset" : ""),
  );

let string_of_shadow =
  fun
  | `shadow(s) => s;
let boxShadows = shadows =>
  `declaration((
    "boxShadow",
    shadows |> List.map(string_of_shadow) |> join(", "),
  ));

let string_of_borderstyle =
  fun
  | `solid => "solid"
  | `dashed => "dashed"
  | `dotted => "dotted"
  | `none => "none";

let border = (px, style, color) =>
  `declaration((
    "border",
    Types.Length.toString(px)
    ++ " "
    ++ string_of_borderstyle(style)
    ++ " "
    ++ string_of_color(color),
  ));
let borderWidth = x =>
  `declaration(("borderWidth", Types.Length.toString(x)));
let borderStyle = x =>
  `declaration(("borderStyle", string_of_borderstyle(x)));
let borderColor = x => `declaration(("borderColor", string_of_color(x)));

let borderLeft = (px, style, color) =>
  `declaration((
    "borderLeft",
    Types.Length.toString(px)
    ++ " "
    ++ string_of_borderstyle(style)
    ++ " "
    ++ string_of_color(color),
  ));
let borderLeftWidth = x =>
  `declaration(("borderLeftWidth", Types.Length.toString(x)));
let borderLeftStyle = x =>
  `declaration(("borderLeftStyle", string_of_borderstyle(x)));
let borderLeftColor = x =>
  `declaration(("borderLeftColor", string_of_color(x)));

let borderRight = (px, style, color) =>
  `declaration((
    "borderRight",
    Types.Length.toString(px)
    ++ " "
    ++ string_of_borderstyle(style)
    ++ " "
    ++ string_of_color(color),
  ));

let borderRightWidth = x =>
  `declaration(("borderRightWidth", Types.Length.toString(x)));
let borderRightColor = x =>
  `declaration(("borderRightColor", string_of_color(x)));
let borderRightStyle = x =>
  `declaration(("borderRightStyle", string_of_borderstyle(x)));
let borderTop = (px, style, color) =>
  `declaration((
    "borderTop",
    Types.Length.toString(px)
    ++ " "
    ++ string_of_borderstyle(style)
    ++ " "
    ++ string_of_color(color),
  ));

let borderTopWidth = x =>
  `declaration(("borderTopWidth", Types.Length.toString(x)));
let borderTopStyle = x =>
  `declaration(("borderTopStyle", string_of_borderstyle(x)));
let borderTopColor = x =>
  `declaration(("borderTopColor", string_of_color(x)));

let borderBottom = (px, style, color) =>
  `declaration((
    "borderBottom",
    Types.Length.toString(px)
    ++ " "
    ++ string_of_borderstyle(style)
    ++ " "
    ++ string_of_color(color),
  ));
let borderBottomWidth = x =>
  `declaration(("borderBottomWidth", Types.Length.toString(x)));
let borderBottomStyle = x =>
  `declaration(("borderBottomStyle", string_of_borderstyle(x)));
let borderBottomColor = x =>
  `declaration(("borderBottomColor", string_of_color(x)));

let borderRadius = i =>
  `declaration(("borderRadius", Types.Length.toString(i)));
let borderTopLeftRadius = i =>
  `declaration(("borderTopLeftRadius", Types.Length.toString(i)));
let borderTopRightRadius = i =>
  `declaration(("borderTopRightRadius", Types.Length.toString(i)));
let borderBottomLeftRadius = i =>
  `declaration(("borderBottomLeftRadius", Types.Length.toString(i)));
let borderBottomRightRadius = i =>
  `declaration(("borderBottomRightRadius", Types.Length.toString(i)));

let tableLayout = x =>
  `declaration((
    "tableLayout",
    switch (x) {
    | `auto => "auto"
    | `fixed => "fixed"
    },
  ));

let borderCollapse = x =>
  `declaration((
    "borderCollapse",
    switch (x) {
    | `collapse => "collapse"
    | `separate => "separate"
    },
  ));

let borderSpacing = i =>
  `declaration(("borderSpacing", Types.Length.toString(i)));

let background = x => `declaration(("background", string_of_background(x)));
let backgrounds = bg =>
  `declaration((
    "background",
    bg |> List.map(string_of_background) |> join(", "),
  ));
let backgroundColor = x =>
  `declaration(("backgroundColor", string_of_color(x)));
let backgroundImage = x =>
  `declaration((
    "backgroundImage",
    switch (x) {
    | `none => "none"
    | `url(url) => "url(" ++ url ++ ")"
    | `linearGradient(angle, stops) => string_of_linearGradient(angle, stops)
    | `repeatingLinearGradient(angle, stops) =>
      string_of_repeatingLinearGradient(angle, stops)
    | `radialGradient(stops) =>
      "radial-gradient(" ++ string_of_stops(stops) ++ ")"
    | `repeatingRadialGradient(stops) =>
      "repeating-radial-gradient(" ++ string_of_stops(stops) ++ ")"
    },
  ));

let backgroundAttachment = x =>
  `declaration((
    "backgroundAttachment",
    switch (x) {
    | `scroll => "scroll"
    | `fixed => "fixed"
    | `local => "local"
    },
  ));

let backgroundClip = x =>
  `declaration((
    "backgroundClip",
    switch (x) {
    | `borderBox => "border-box"
    | `contentBox => "content-box"
    | `paddingBox => "padding-box"
    },
  ));

let backgroundOrigin = x =>
  `declaration((
    "backgroundOrigin",
    switch (x) {
    | `borderBox => "border-box"
    | `contentBox => "content-box"
    | `paddingBox => "padding-box"
    },
  ));

let backgroundPosition = (x, y) =>
  `declaration((
    "backgroundPosition",
    Types.Length.toString(x) ++ " " ++ Types.Length.toString(y),
  ));

let backgroundRepeat = x =>
  `declaration((
    "backgroundRepeat",
    switch (x) {
    | `repeat => "repeat"
    | `noRepeat => "no-repeat"
    | `repeatX => "repeat-x"
    | `repeatY => "repeat-y"
    },
  ));

let backgroundSize = x =>
  `declaration((
    "backgroundSize",
    switch (x) {
    | `size(x, y) =>
      Types.Length.toString(x) ++ " " ++ Types.Length.toString(y)
    | `auto => "auto"
    | `cover => "cover"
    | `contain => "contain"
    },
  ));

let cursor = x => `declaration(("cursor", string_of_cursor(x)));

let clipPath = x =>
  `declaration((
    "clipPath",
    switch (x) {
    | `url(url) => "url(" ++ url ++ ")"
    },
  ));

type listStyleType = [
  | `disc
  | `circle
  | `square
  | `decimal
  | `lowerAlpha
  | `upperAlpha
  | `lowerGreek
  | `lowerLatin
  | `upperLatin
  | `lowerRoman
  | `upperRoman
  | `none
];

let string_of_listStyleType =
  fun
  | `disc => "disc"
  | `circle => "circle"
  | `square => "square"
  | `decimal => "decimal"
  | `lowerAlpha => "lower-alpha"
  | `upperAlpha => "upper-alpha"
  | `lowerGreek => "lower-greek"
  | `lowerLatin => "lower-latin"
  | `upperLatin => "upper-latin"
  | `lowerRoman => "lower-roman"
  | `upperRoman => "upper-roman"
  | `none => "none";

let string_of_listStylePosition =
  fun
  | `inside => "inside"
  | `outside => "outside";

let string_of_listStyleImage =
  fun
  | `none => "none"
  | `url(url) => "url(" ++ url ++ ")";

let listStyle = (style, pos, img) =>
  `declaration((
    "listStyle",
    string_of_listStyleType(style)
    ++ " "
    ++ string_of_listStylePosition(pos)
    ++ " "
    ++ string_of_listStyleImage(img),
  ));

let listStyleType = x =>
  `declaration(("listStyleType", string_of_listStyleType(x)));

let listStylePosition = x =>
  `declaration(("listStylePosition", string_of_listStylePosition(x)));

let listStyleImage = x =>
  `declaration(("listStyleImage", string_of_listStyleImage(x)));

let opacity = x => `declaration(("opacity", string_of_float(x)));

type outlineStyle = [
  | `none
  | `hidden
  | `dotted
  | `dashed
  | `solid
  | `double
  | `groove
  | `ridge
  | `inset
  | `outset
];

let string_of_outlineStyle =
  fun
  | `none => "none"
  | `hidden => "hidden"
  | `dotted => "dotted"
  | `dashed => "dashed"
  | `solid => "solid"
  | `double => "double"
  | `groove => "grove"
  | `ridge => "ridge"
  | `inset => "inset"
  | `outset => "outset";

let outline = (size, style, color) =>
  `declaration((
    "outline",
    Types.Length.toString(size)
    ++ " "
    ++ string_of_outlineStyle(style)
    ++ " "
    ++ string_of_color(color),
  ));

let outlineStyle = x =>
  `declaration(("outlineStyle", string_of_outlineStyle(x)));

let outlineWidth = x =>
  `declaration(("outlineWidth", Types.Length.toString(x)));

let outlineColor = x => `declaration(("outlineColor", string_of_color(x)));

let outlineOffset = x =>
  `declaration(("outlineOffset", Types.Length.toString(x)));

/**
 * Text
 */

/* see https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight#Common_weight_name_mapping */
type fontWeight = [
  | `num(int)
  | `thin
  | `extraLight
  | `light
  | `normal
  | `medium
  | `semiBold
  | `bold
  | `extraBold
  | `black
  | `lighter
  | `bolder
];

let thin = `thin;
let extraLight = `extraLight;
let light = `light;
let medium = `medium;
let semiBold = `semiBold;
let bold = `bold;
let extraBold = `extraBold;
let lighter = `lighter;
let bolder = `bolder;

let color = x => `declaration(("color", string_of_color(x)));

let fontWeight = x => `declaration(("fontWeight", string_of_fontWeight(x)));

let fontFace = (~fontFamily, ~src, ~fontStyle=?, ~fontWeight=?, ()) => {
  let fontStyle =
    Js.Option.map((. value) => Types.FontStyle.toString(value), fontStyle);
  let src =
    src
    |> List.map(
         fun
         | `localUrl(value) => {j|local("$(value)")|j}
         | `url(value) => {j|url("$(value)")|j},
       )
    |> String.concat(", ");

  let fontStyle =
    Belt.Option.mapWithDefault(fontStyle, "", s => "font-style: " ++ s);
  let fontWeight =
    Belt.Option.mapWithDefault(fontWeight, "", w =>
      "font-weight: " ++ string_of_fontWeight(w)
    );
  let asString = {j|@font-face {
    font-family: $fontFamily;
    src: $src;
    $(fontStyle);
    $(fontWeight);
}|j};

  Emotion.rawInjectGlobal(asString);

  fontFamily;
};

let lineHeight = x =>
  `declaration((
    "lineHeight",
    switch (x) {
    | `normal => "normal"
    | `abs(x) => string_of_float(x)
    | `calc(`add, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " + "
      ++ Types.Length.toString(b)
      ++ ")"
    | `calc(`sub, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " - "
      ++ Types.Length.toString(b)
      ++ ")"
    | `ch(x) => string_of_float(x) ++ "ch"
    | `cm(x) => string_of_float(x) ++ "cm"
    | `em(x) => string_of_float(x) ++ "em"
    | `ex(x) => string_of_float(x) ++ "ex"
    | `mm(x) => string_of_float(x) ++ "mm"
    | `percent(x) => string_of_float(x) ++ "%"
    | `pt(x) => string_of_int(x) ++ "pt"
    | `px(x) => string_of_int(x) ++ "px"
    | `pxFloat(x) => string_of_float(x) ++ "px"
    | `rem(x) => string_of_float(x) ++ "rem"
    | `vh(x) => string_of_float(x) ++ "vh"
    | `vmax(x) => string_of_float(x) ++ "vmax"
    | `vmin(x) => string_of_float(x) ++ "vmin"
    | `vw(x) => string_of_float(x) ++ "vw"
    | `auto => "auto"
    | `zero => "0"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let letterSpacing = x =>
  `declaration((
    "letterSpacing",
    switch (x) {
    | `normal => "normal"
    | `calc(`add, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " + "
      ++ Types.Length.toString(b)
      ++ ")"
    | `calc(`sub, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " - "
      ++ Types.Length.toString(b)
      ++ ")"
    | `ch(x) => string_of_float(x) ++ "ch"
    | `cm(x) => string_of_float(x) ++ "cm"
    | `em(x) => string_of_float(x) ++ "em"
    | `ex(x) => string_of_float(x) ++ "ex"
    | `mm(x) => string_of_float(x) ++ "mm"
    | `percent(x) => string_of_float(x) ++ "%"
    | `pt(x) => string_of_int(x) ++ "pt"
    | `px(x) => string_of_int(x) ++ "px"
    | `pxFloat(x) => string_of_float(x) ++ "px"
    | `rem(x) => string_of_float(x) ++ "rem"
    | `vh(x) => string_of_float(x) ++ "vh"
    | `vmax(x) => string_of_float(x) ++ "vmax"
    | `vmin(x) => string_of_float(x) ++ "vmin"
    | `vw(x) => string_of_float(x) ++ "vw"
    | `auto => "auto"
    | `zero => "0"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let textAlign = x =>
  `declaration((
    "textAlign",
    switch (x) {
    | `left => "left"
    | `right => "right"
    | `center => "center"
    | `justify => "justify"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let textDecoration = x =>
  `declaration((
    "textDecoration",
    switch (x) {
    | `none => "none"
    | `underline => "underline"
    | `overline => "overline"
    | `lineThrough => "line-through"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let textDecorationColor = x =>
  `declaration(("textDecorationColor", string_of_color(x)));

let textDecorationStyle = x =>
  `declaration((
    "textDecorationStyle",
    switch (x) {
    | `wavy => "wavy"
    | `solid => "solid"
    | `double => "double"
    | `dotted => "dotted"
    | `dashed => "dashed"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let textIndent = x => `declaration(("textIndent", Types.Length.toString(x)));

let textOverflow = x =>
  `declaration((
    "textOverflow",
    switch (x) {
    | `clip => "clip"
    | `ellipsis => "ellipsis"
    | `string(s) => s
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let textShadow = (~x=zero, ~y=zero, ~blur=zero, color) =>
  `textShadow(
    Types.Length.toString(x)
    ++ " "
    ++ Types.Length.toString(y)
    ++ " "
    ++ Types.Length.toString(blur)
    ++ " "
    ++ string_of_color(color),
  );

let string_of_textShadow =
  fun
  | `textShadow(s) => s;
let textShadows = textShadows =>
  `declaration((
    "textShadow",
    textShadows |> List.map(string_of_textShadow) |> join(", "),
  ));

let textTransform = x =>
  `declaration((
    "textTransform",
    switch (x) {
    | `uppercase => "uppercase"
    | `lowercase => "lowercase"
    | `capitalize => "capitalize"
    | `none => "none"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let userSelect = x =>
  `declaration((
    "userSelect",
    switch (x) {
    | `auto => "auto"
    | `all => "all"
    | `text => "text"
    | `none => "none"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let verticalAlign = x =>
  `declaration((
    "verticalAlign",
    switch (x) {
    | `baseline => "baseline"
    | `sub => "sub"
    | `super => "super"
    | `top => "top"
    | `textTop => "text-top"
    | `middle => "middle"
    | `bottom => "bottom"
    | `textBottom => "text-bottom"
    | `calc(`add, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " + "
      ++ Types.Length.toString(b)
      ++ ")"
    | `calc(`sub, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " - "
      ++ Types.Length.toString(b)
      ++ ")"
    | `ch(x) => string_of_float(x) ++ "ch"
    | `cm(x) => string_of_float(x) ++ "cm"
    | `em(x) => string_of_float(x) ++ "em"
    | `ex(x) => string_of_float(x) ++ "ex"
    | `mm(x) => string_of_float(x) ++ "mm"
    | `percent(x) => string_of_float(x) ++ "%"
    | `pt(x) => string_of_int(x) ++ "pt"
    | `px(x) => string_of_int(x) ++ "px"
    | `pxFloat(x) => string_of_float(x) ++ "px"
    | `rem(x) => string_of_float(x) ++ "rem"
    | `vh(x) => string_of_float(x) ++ "vh"
    | `vmax(x) => string_of_float(x) ++ "vmax"
    | `vmin(x) => string_of_float(x) ++ "vmin"
    | `vw(x) => string_of_float(x) ++ "vw"
    | `auto => "auto"
    | `zero => "0"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let whiteSpace = x =>
  `declaration((
    "whiteSpace",
    switch (x) {
    | `normal => "normal"
    | `nowrap => "nowrap"
    | `pre => "pre"
    | `preLine => "pre-line"
    | `preWrap => "pre-wrap"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let wordBreak = x =>
  `declaration((
    "wordBreak",
    switch (x) {
    | `breakAll => "break-all"
    | `keepAll => "keep-all"
    | `normal => "normal"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let wordSpacing = x =>
  `declaration((
    "wordSpacing",
    switch (x) {
    | `normal => "normal"
    | `calc(`add, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " + "
      ++ Types.Length.toString(b)
      ++ ")"
    | `calc(`sub, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " - "
      ++ Types.Length.toString(b)
      ++ ")"
    | `ch(x) => string_of_float(x) ++ "ch"
    | `cm(x) => string_of_float(x) ++ "cm"
    | `em(x) => string_of_float(x) ++ "em"
    | `ex(x) => string_of_float(x) ++ "ex"
    | `mm(x) => string_of_float(x) ++ "mm"
    | `percent(x) => string_of_float(x) ++ "%"
    | `pt(x) => string_of_int(x) ++ "pt"
    | `px(x) => string_of_int(x) ++ "px"
    | `pxFloat(x) => string_of_float(x) ++ "px"
    | `rem(x) => string_of_float(x) ++ "rem"
    | `vh(x) => string_of_float(x) ++ "vh"
    | `vmax(x) => string_of_float(x) ++ "vmax"
    | `vmin(x) => string_of_float(x) ++ "vmin"
    | `vw(x) => string_of_float(x) ++ "vw"
    | `auto => "auto"
    | `zero => "0"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let wordWrap = x =>
  `declaration((
    "wordWrap",
    switch (x) {
    | `normal => "normal"
    | `breakWord => "break-word"
    | `initial => "initial"
    | `inherit_ => "inherit"
    | `unset => "unset"
    },
  ));

let string_of_pointerEvents =
  fun
  | `auto => "auto"
  | `none => "none";

let pointerEvents = x =>
  `declaration(("pointerEvents", string_of_pointerEvents(x)));

/**
 * Transform
 */

type transform = [
  | `translate(length, length)
  | `translate3d(length, length, length)
  | `translateX(length)
  | `translateY(length)
  | `translateZ(length)
  | `scale(float, float)
  | `scale3d(float, float, float)
  | `scaleX(float)
  | `scaleY(float)
  | `scaleZ(float)
  | `rotate(angle)
  | `rotate3d(float, float, float, angle)
  | `rotateX(angle)
  | `rotateY(angle)
  | `rotateZ(angle)
  | `skew(angle, angle)
  | `skewX(angle)
  | `skewY(angle)
  | `perspective(int)
];

let string_of_transform =
  fun
  | `translate(x, y) =>
    "translate("
    ++ Types.Length.toString(x)
    ++ ", "
    ++ Types.Length.toString(y)
    ++ ")"
  | `translate3d(x, y, z) => string_of_translate3d(x, y, z)
  | `translateX(x) => "translateX(" ++ Types.Length.toString(x) ++ ")"
  | `translateY(y) => "translateY(" ++ Types.Length.toString(y) ++ ")"
  | `translateZ(z) => "translateZ(" ++ Types.Length.toString(z) ++ ")"
  | `scale(x, y) => string_of_scale(x, y)
  | `scale3d(x, y, z) =>
    "scale3d("
    ++ string_of_float(x)
    ++ ", "
    ++ string_of_float(y)
    ++ ", "
    ++ string_of_float(z)
    ++ ")"
  | `scaleX(x) => "scaleX(" ++ string_of_float(x) ++ ")"
  | `scaleY(y) => "scaleY(" ++ string_of_float(y) ++ ")"
  | `scaleZ(z) => "scaleZ(" ++ string_of_float(z) ++ ")"
  | `rotate(a) => "rotate(" ++ Types.Angle.toString(a) ++ ")"
  | `rotate3d(x, y, z, a) =>
    "rotate3d("
    ++ string_of_float(x)
    ++ ", "
    ++ string_of_float(y)
    ++ ", "
    ++ string_of_float(z)
    ++ ", "
    ++ Types.Angle.toString(a)
    ++ ")"
  | `rotateX(a) => "rotateX(" ++ Types.Angle.toString(a) ++ ")"
  | `rotateY(a) => "rotateY(" ++ Types.Angle.toString(a) ++ ")"
  | `rotateZ(a) => "rotateZ(" ++ Types.Angle.toString(a) ++ ")"
  | `skew(x, y) =>
    "skew("
    ++ Types.Angle.toString(x)
    ++ ", "
    ++ Types.Angle.toString(y)
    ++ ")"
  | `skewX(a) => "skewX(" ++ Types.Angle.toString(a) ++ ")"
  | `skewY(a) => "skewY(" ++ Types.Angle.toString(a) ++ ")"
  | `perspective(x) => "perspective(" ++ string_of_int(x) ++ ")";

let transform = x => `declaration(("transform", string_of_transform(x)));

let transforms = xs =>
  `declaration((
    "transform",
    xs |> List.map(string_of_transform) |> join(" "),
  ));

let transformOrigin = (x, y) =>
  `declaration((
    "transformOrigin",
    Types.Length.toString(x) ++ " " ++ Types.Length.toString(y),
  ));

let transformOrigin3d = (x, y, z) =>
  `declaration((
    "transformOrigin",
    Types.Length.toString(x)
    ++ " "
    ++ Types.Length.toString(y)
    ++ " "
    ++ Types.Length.toString(z)
    ++ " ",
  ));

let transformStyle = x =>
  `declaration((
    "transformStyle",
    switch (x) {
    | `preserve3d => "preserve-3d"
    | `flat => "flat"
    },
  ));

let perspective = x =>
  `declaration((
    "perspective",
    switch (x) {
    | `none => "none"
    | `calc(`add, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " + "
      ++ Types.Length.toString(b)
      ++ ")"
    | `calc(`sub, a, b) =>
      "calc("
      ++ Types.Length.toString(a)
      ++ " - "
      ++ Types.Length.toString(b)
      ++ ")"
    | `ch(x) => string_of_float(x) ++ "ch"
    | `cm(x) => string_of_float(x) ++ "cm"
    | `em(x) => string_of_float(x) ++ "em"
    | `ex(x) => string_of_float(x) ++ "ex"
    | `mm(x) => string_of_float(x) ++ "mm"
    | `percent(x) => string_of_float(x) ++ "%"
    | `pt(x) => string_of_int(x) ++ "pt"
    | `px(x) => string_of_int(x) ++ "px"
    | `pxFloat(x) => string_of_float(x) ++ "px"
    | `rem(x) => string_of_float(x) ++ "rem"
    | `vh(x) => string_of_float(x) ++ "vh"
    | `vmax(x) => string_of_float(x) ++ "vmax"
    | `vmin(x) => string_of_float(x) ++ "vmin"
    | `vw(x) => string_of_float(x) ++ "vw"
    | `zero => "0"
    },
  ));

/**
* Transition
*/
type timingFunction = [
  | `linear
  | `ease
  | `easeIn
  | `easeOut
  | `easeInOut
  | `stepStart
  | `stepEnd
  | `steps(int, [ | `start | `end_])
  | `cubicBezier(float, float, float, float)
];

let string_of_timingFunction =
  fun
  | `linear => "linear"
  | `ease => "ease"
  | `easeIn => "ease-out"
  | `easeOut => "ease-out"
  | `easeInOut => "ease-in-out"
  | `stepStart => "step-start"
  | `stepEnd => "step-end"
  | `steps(i, `start) => "steps(" ++ string_of_int(i) ++ ", start)"
  | `steps(i, `end_) => "steps(" ++ string_of_int(i) ++ ", end)"
  | `cubicBezier(a, b, c, d) =>
    "cubic-bezier("
    ++ string_of_float(a)
    ++ ", "
    ++ string_of_float(b)
    ++ ", "
    ++ string_of_float(c)
    ++ ", "
    ++ string_of_float(d)
    ++ ")";

let transition = (~duration=0, ~delay=0, ~timingFunction=`ease, property) =>
  `transition(
    string_of_time(duration)
    ++ " "
    ++ string_of_timingFunction(timingFunction)
    ++ " "
    ++ string_of_time(delay)
    ++ " "
    ++ property,
  );

let transitions = xs =>
  `declaration((
    "transition",
    xs
    |> List.map(
         fun
         | `transition(s) => s,
       )
    |> join(", "),
  ));

let transitionDelay = i =>
  `declaration(("transitionDelay", string_of_time(i)));

let transitionDuration = i =>
  `declaration(("transitionDuration", string_of_time(i)));

let transitionTimingFunction = x =>
  `declaration(("transitionTimingFunction", string_of_timingFunction(x)));

let transitionProperty = x => `declaration(("transitionProperty", x));

let perspectiveOrigin = (x, y) =>
  `declaration((
    "perspectiveOrigin",
    Types.Length.toString(x) ++ " " ++ Types.Length.toString(y),
  ));

/**
 * Animation
 */

type animationDirection = [
  | `normal
  | `reverse
  | `alternate
  | `alternateReverse
];

let string_of_animationDirection =
  fun
  | `normal => "normal"
  | `reverse => "reverse"
  | `alternate => "alternate"
  | `alternateReverse => "alternate-reverse";

type animationFillMode = [ | `none | `forwards | `backwards | `both];

let string_of_animationFillMode =
  fun
  | `none => "none"
  | `forwards => "forwards"
  | `backwards => "backwards"
  | `both => "both";

type animationIterationCount = [ | `infinite | `count(int)];

let string_of_animationIterationCount =
  fun
  | `infinite => "infinite"
  | `count(x) => string_of_int(x);

type animationPlayState = [ | `paused | `running];

let string_of_animationPlayState =
  fun
  | `paused => "paused"
  | `running => "running";

let animation =
    (
      ~duration=0,
      ~delay=0,
      ~direction=`normal,
      ~timingFunction=`ease,
      ~fillMode=`none,
      ~playState=`running,
      ~iterationCount=`count(1),
      name,
    ) =>
  `animation(
    name
    ++ " "
    ++ string_of_time(duration)
    ++ " "
    ++ string_of_timingFunction(timingFunction)
    ++ " "
    ++ string_of_time(delay)
    ++ " "
    ++ string_of_animationIterationCount(iterationCount)
    ++ " "
    ++ string_of_animationDirection(direction)
    ++ " "
    ++ string_of_animationFillMode(fillMode)
    ++ " "
    ++ string_of_animationPlayState(playState),
  );

let string_of_animation =
  fun
  | `animation(s) => s;
let animations = xs =>
  `declaration((
    "animation",
    xs |> List.map(string_of_animation) |> join(", "),
  ));

let animationDelay = x =>
  `declaration(("animationDelay", string_of_time(x)));
let animationDirection = x =>
  `declaration(("animationDirection", string_of_animationDirection(x)));
let animationDuration = x =>
  `declaration(("animationDuration", string_of_time(x)));
let animationFillMode = x =>
  `declaration(("animationFillMode", string_of_animationFillMode(x)));
let animationIterationCount = x =>
  `declaration((
    "animationIterationCount",
    string_of_animationIterationCount(x),
  ));
let animationName = x => `declaration(("animationName", x));
let animationPlayState = x =>
  `declaration(("animationPlayState", string_of_animationPlayState(x)));
let animationTimingFunction = x =>
  `declaration(("animationTimingFunction", string_of_timingFunction(x)));

/**
 * Selectors
 */

let selector = (selector, rules) => `selector((selector, rules));

/* MEDIA */

let active = selector(":active");
let after = selector("::after");
let before = selector("::before");
let checked = selector(":checked");
let children = selector(" > *");
let directSibling = selector(" + ");
let disabled = selector(":disabled");
let firstChild = selector(":first-child");
let firstOfType = selector(":first-of-type");
let focus = selector(":focus");
let hover = selector(":hover");
let lastChild = selector(":last-child");
let lastOfType = selector(":last-of-type");
let link = selector(":link");
let readOnly = selector(":read-only");
let required = selector(":required");
let visited = selector(":visited");
let enabled = selector(":enabled");
let noContent = selector(":empty");
let default = selector(":default");
let anyLink = selector(":any-link");
let onlyChild = selector(":only-child");
let onlyOfType = selector(":only-of-type");
let optional = selector(":optional");
let invalid = selector(":invalid");
let outOfRange = selector(":out-of-range");
let siblings = selector(" ~ ");
let target = selector(":target");
let firstLine = selector("::first-line");
let firstLetter = selector("::first-letter");
let selection = selector("::selection");
let placeholder = selector("::placeholder");

let media = (query, rules) => `selector(("@media " ++ query, rules));

/**
 * SVG
 */
module SVG = {
  let fill = color => `declaration(("fill", string_of_color(color)));
  let fillOpacity = opacity =>
    `declaration(("fillOpacity", string_of_float(opacity)));
  let fillRule = x =>
    `declaration((
      "fillRule",
      switch (x) {
      | `evenodd => "evenodd"
      | `nonzero => "nonzero"
      },
    ));
  let stroke = color => `declaration(("stroke", string_of_color(color)));
  let strokeWidth = length =>
    `declaration(("strokeWidth", Types.Length.toString(length)));
  let strokeOpacity = opacity =>
    `declaration(("strokeOpacity", string_of_float(opacity)));
  let strokeMiterlimit = x =>
    `declaration(("strokeMiterlimit", string_of_float(x)));
  let strokeLinecap = x =>
    `declaration((
      "strokeLinecap",
      switch (x) {
      | `butt => "butt"
      | `round => "round"
      | `square => "square"
      },
    ));

  let strokeLinejoin = x =>
    `declaration((
      "strokeLinejoin",
      switch (x) {
      | `miter => "miter"
      | `round => "round"
      | `bevel => "bevel"
      },
    ));
  let stopColor = c => `declaration(("stopColor", string_of_color(c)));
  let stopOpacity = o => `declaration(("stopOpacity", string_of_float(o)));
};
