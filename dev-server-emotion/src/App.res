open CssJs

global(. "html, body", [margin(zero), padding(zero), backgroundInt.bitwiseOr(lavender)])

switch ReactDOM.querySelector("#app") {
| None => ()
| Some(dom) => ReactDOM.render(<Demo />, dom)
}
