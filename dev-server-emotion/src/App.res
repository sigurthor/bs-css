open CssJs

global(. "html, body", [margin(zero), padding(zero), backgroundCoInt.bitwiseOr(lavender)])

switch ReactDOM.querySelector("#app") {
| None => ()
| Some(dom) => ReactDOM.render(<Demo />, dom)
}
