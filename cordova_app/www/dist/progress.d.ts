export declare class ProgressRing {
    constructor(id: string);
    setValue(value: number): void;
    getValue(): number;
    hide(): void;
    show(): void;
    startAutoUpdate(durationMs: number): void;
    stopAutoUpdate(): void;
    updateRateMs: number;
    private progressRing;
    private autoUpdateInterval;
}
export declare class ProgressSpinner {
    constructor(id: string);
    hide(): void;
    show(): void;
    private progressSpinner;
}
