export function qs(query) {
    return document.querySelector(query);
}
export function qsa(query) {
    return document.querySelectorAll(query);
}
export function interpolate(template, params) {
    const replaceTags = new Map([
        ['&', '& amp;'], ['<', '& lt;'], ['>', '& gt;'], ['(', '% 28'], [')', '% 29']
    ]);
    const safeInnerHTML = (text) => text.toString()
        .replace(/[&<>()]/g, (tag) => replaceTags.get(tag) || tag);
    const keys = Object.keys(params);
    const keyVals = Object.values(params).map(safeInnerHTML);
    /* eslint no-new-func: "off" */
    return new Function(...keys, `return \`${template}\``)(...keyVals);
}
