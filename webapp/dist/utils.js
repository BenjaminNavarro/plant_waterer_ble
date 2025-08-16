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
/**
 * Converts a duration from a given unit into seconds
 *
 * @param durationInUnit  - A duration in the given unit
 * @param unit  - One of the possible unit types ('s' => seconds, 'm' => minutes, 'h' => hours, 'd' => days)
 * @return The duration converted into seconds
 */
export function convertDuration(durationInUnit, unit) {
    switch (unit) {
        case 's':
            return durationInUnit;
        case 'm':
            return durationInUnit * 60;
        case 'h':
            return durationInUnit * 60 * 60;
        case 'd':
            return durationInUnit * 60 * 60 * 24;
        default:
            return -1;
    }
}
