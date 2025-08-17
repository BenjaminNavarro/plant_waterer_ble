import { ProgressUI } from './progress_ui.js';
import { TinyDropDevice } from './tinydrop_device.js';
export declare class ConfigPanel {
    private programEnabled;
    private programEnabledElem;
    private programDuration;
    private programDurationTimeType;
    private programPeriod;
    private programPeriodTimeType;
    private programWaterFlow;
    private programStart;
    private programTestButton;
    private programApplyButton;
    private programStopButton;
    private device;
    private progressUI;
    private program;
    constructor(progressUI: ProgressUI);
    setDevice(device: TinyDropDevice): void;
    private startWatering;
    private stopWatering;
    private sendProgram;
}
