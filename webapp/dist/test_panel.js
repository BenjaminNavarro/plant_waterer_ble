import { convertDuration, qs } from './utils.js';
export class ManualPanel {
    device;
    manualWaterFlow = qs('#manual_flow_speed');
    manualStartButton = qs('#manual_start_button');
    manualStopButton = qs('#manual_stop_button');
    manualDuration = qs('#manual_duration');
    manualDurationTimeType = qs('#manual_duration_time_type');
    progressUI;
    duration = 0;
    waterFlow = 0;
    constructor(progressUI) {
        this.progressUI = progressUI;
        let log = (value) => {
            console.log(`[Manual] ${value}`);
        };
        this.manualDuration.addEventListener('sl-change', () => {
            let value = Number(this.manualDuration.value);
            value = Math.max(value, 0);
            this.duration = convertDuration(value, this.manualDurationTimeType.value);
            this.manualDuration.value = value.toString();
        });
        this.manualDurationTimeType.addEventListener('sl-change', () => {
            this.duration = convertDuration(Math.max(Number(this.manualDuration.value), 0), this.manualDurationTimeType.value);
        });
        this.manualWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.manualWaterFlow.value);
            this.waterFlow = Math.min(Math.max(value, 0), 100);
            this.manualWaterFlow.value = this.waterFlow.toString();
        });
        this.manualStartButton.addEventListener('click', () => {
            this.startWatering();
        });
        this.manualStopButton.addEventListener('click', () => {
            this.stopWatering();
        });
    }
    startWatering() {
        if (this.device != null && !this.device.wateringState().watering) {
            this.device.startWatering(this.duration, this.waterFlow);
        }
    }
    stopWatering() {
        if (this.device != null && this.device.wateringState().watering) {
            this.device.stopWatering();
        }
    }
}
