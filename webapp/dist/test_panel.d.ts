import { ProgressUI } from './progress_ui.js';
import { TinyDropDevice } from './tinydrop_device.js';
export declare class ManualPanel {
    device: TinyDropDevice;
    private manualWaterFlow;
    private manualStartButton;
    private manualStopButton;
    private manualDuration;
    private manualDurationTimeType;
    private progressUI;
    private duration;
    private waterFlow;
    constructor(progressUI: ProgressUI);
    private startWatering;
    private stopWatering;
}
