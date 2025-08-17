import { ProgressUI } from './progress_ui.js';
import { TinyDropDevice } from './tinydrop_device.js';
export declare class ManualPanel {
    private manualWaterFlow;
    private manualStartButton;
    private manualStopButton;
    private manualDuration;
    private manualDurationTimeType;
    private device;
    private progressUI;
    private duration;
    private waterFlow;
    constructor(progressUI: ProgressUI);
    setDevice(device: TinyDropDevice): void;
    private startWatering;
    private stopWatering;
}
