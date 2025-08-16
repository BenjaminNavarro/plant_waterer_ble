import { convertDuration, qs } from './utils.js';
import { ResultType } from './progress_ui.js';
export class ManualPanel {
    manualWaterFlow = qs('#manual_flow_speed');
    manualStartButton = qs('#manual_start_button');
    manualStopButton = qs('#manual_stop_button');
    manualDuration = qs('#manual_duration');
    manualDurationTimeType = qs('#manual_duration_time_type');
    constructor(progressUI) {
        let log = (value) => {
            console.log(`[Manual] ${value}`);
        };
        this.manualDuration.addEventListener('sl-change', () => {
            let value = Number(this.manualDuration.value);
            value = Math.max(value, 0);
            this.manualDuration.value = value.toString();
            log(this.manualDuration.value);
        });
        this.manualDurationTimeType.addEventListener('sl-change', () => {
            log(this.manualDurationTimeType.value);
        });
        this.manualWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.manualWaterFlow.value);
            value = Math.min(Math.max(value, 0), 100);
            this.manualWaterFlow.value = value.toString();
            log(this.manualWaterFlow.value);
        });
        this.manualStartButton.addEventListener('click', () => {
            log('Start clicked');
            progressUI.startAutoUpdate(convertDuration(Number(this.manualDuration.value), this.manualDurationTimeType.value) * 1000);
        });
        this.manualStopButton.addEventListener('click', () => {
            log('Stop clicked');
            progressUI.stop(ResultType.None);
        });
    }
}
