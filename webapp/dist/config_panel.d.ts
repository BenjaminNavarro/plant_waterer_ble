import { ProgressUI } from './progress_ui.js';
export declare class ConfigPanel {
    programEnabled: HTMLInputElement;
    programDuration: HTMLInputElement;
    programDurationTimeType: HTMLInputElement;
    programPeriod: HTMLInputElement;
    programPeriodTimeType: HTMLInputElement;
    programWaterFlow: HTMLInputElement;
    programStart: HTMLInputElement;
    programTestButton: HTMLButtonElement;
    programApplyButton: HTMLButtonElement;
    programStopButton: HTMLButtonElement;
    constructor(progressUI: ProgressUI);
}
