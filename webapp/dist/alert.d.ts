export declare enum AlertType {
    Info = 0,
    Success = 1,
    Warning = 2,
    Error = 3
}
export declare enum AlertDuration {
    Short = 3000,
    Default = 5000,
    Long = 10000,
    Infinite = Infinity
}
export declare class Alert {
    private alertBoxElem;
    private alertText;
    private alertIcon;
    private alertIconElem;
    private currentTimeoutId;
    show(text: string, type: AlertType, durationMs: number): void;
}
