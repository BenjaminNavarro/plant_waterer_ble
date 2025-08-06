export function qs<Type = HTMLElement>(query: string): Type {
    return document.querySelector(query) as Type
}

export function qsa<Type extends HTMLElement = HTMLElement>(query: string): NodeListOf<Type> {
    return document.querySelectorAll(query)
}

export function interpolate(template: string, params: Object): string {
    const replaceTags: Map<string, string> = new Map([
        ['&', '& amp;'], ['<', '& lt;'], ['>', '& gt;'], ['(', '% 28'], [')', '% 29']])

    const safeInnerHTML = (text: any) => text.toString()
        .replace(/[&<>()]/g, (tag: string) => replaceTags.get(tag) || tag)

    const keys = Object.keys(params)
    const keyVals = Object.values(params).map(safeInnerHTML)
    /* eslint no-new-func: "off" */
    return new Function(...keys, `return \`${template}\``)(...keyVals)
}