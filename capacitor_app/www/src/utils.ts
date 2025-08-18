export type timeoutId = null | ReturnType<typeof setTimeout>

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

/**
 * Converts a duration from a given unit into seconds
 *
 * @param durationInUnit  - A duration in the given unit
 * @param unit  - One of the possible unit types ('s' => seconds, 'm' => minutes, 'h' => hours, 'd' => days)
 * @return The duration converted into seconds
 */
export function convertDuration(durationInUnit: number, unit: string): number {
    switch (unit) {
        case 's':
            return durationInUnit
        case 'm':
            return durationInUnit * 60
        case 'h':
            return durationInUnit * 60 * 60
        case 'd':
            return durationInUnit * 60 * 60 * 24
        default:
            return -1
    }
}

export function findBestDurationUnit(durationSec: number): string {
    if (durationSec > 60 * 60 * 24) {
        return 'd'
    }
    else if (durationSec > 60 * 60) {
        return 'h'
    }
    else if (durationSec > 60) {
        return 'm'
    }
    else {
        return 's'
    }
}

export function convertDurationToBestUnit(durationSec): { duration: number, unit: string } {
    const unit = findBestDurationUnit(durationSec)
    let duration = durationSec

    switch (unit) {
        case 'm':
            duration /= 60
            break
        case 'h':
            duration /= (60 * 60)
            break
        case 'd':
            duration /= (60 * 60 * 24)
            break
    }

    return { duration: duration, unit: unit }
}