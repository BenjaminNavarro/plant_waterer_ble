export declare class ProgressUI {
    stop(result: ResultType): void;
    startAutoUpdate(durationMs: number): void;
    spin(): void;
    private hideResult;
    private progressRing;
    private progressSpinner;
    private progressResult;
}
export declare enum ResultType {
    None = 0,
    Success = 1,
    Warning = 2,
    Error = 3
}
export declare enum ResultDuration {
    Default = 2000,
    Medium = 5000,
    Long = 10000,
    Infinite = Infinity
}
