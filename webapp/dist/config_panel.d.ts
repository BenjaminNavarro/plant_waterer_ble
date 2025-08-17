import { ProgressUI } from './progress_ui.js';
import { TinyDropDevice } from './tinydrop_device.js';
export declare class ConfigPanel {
    device: TinyDropDevice;
    private programEnabled;
    private programDuration;
    private programDurationTimeType;
    private programPeriod;
    private programPeriodTimeType;
    private programWaterFlow;
    private programStart;
    private programTestButton;
    private programApplyButton;
    private programStopButton;
    private progressUI;
    private program;
    constructor(progressUI: ProgressUI);
    private startWatering;
    private stopWatering;
    private sendProgram;
}
