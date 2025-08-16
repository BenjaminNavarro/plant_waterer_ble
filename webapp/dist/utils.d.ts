export declare function qs<Type = HTMLElement>(query: string): Type;
export declare function qsa<Type extends HTMLElement = HTMLElement>(query: string): NodeListOf<Type>;
export declare function interpolate(template: string, params: Object): string;
/**
 * Converts a duration from a given unit into seconds
 *
 * @param durationInUnit  - A duration in the given unit
 * @param unit  - One of the possible unit types ('s' => seconds, 'm' => minutes, 'h' => hours, 'd' => days)
 * @return The duration converted into seconds
 */
export declare function convertDuration(durationInUnit: number, unit: string): number;
