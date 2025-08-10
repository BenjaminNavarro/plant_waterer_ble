import { qs } from './utils.js';
export class ConfigPanel {
    programEnabled = qs('#program_enabled');
    programDuration = qs('#program_duration');
    programDurationTimeType = qs('#program_duration_time_type');
    programPeriod = qs('#program_period');
    programPeriodTimeType = qs('#program_period_time_type');
    programWaterFlow = qs('#program_flow_speed');
    programStart = qs('#program_start');
    programTestButton = qs('#program_test_button');
    programApplyButton = qs('#program_apply_button');
    programStopButton = qs('#program_stop_button');
    constructor(progressUI) {
        this.programEnabled.addEventListener('sl-change', () => {
            console.log(this.programEnabled.checked);
        });
        this.programDuration.addEventListener('sl-change', () => {
            let value = Number(this.programDuration.value);
            value = Math.max(value, 0);
            this.programDuration.value = value.toString();
            console.log(this.programDuration.value);
        });
        this.programDurationTimeType.addEventListener('sl-change', () => {
            console.log(this.programDurationTimeType.value);
        });
        this.programPeriod.addEventListener('sl-change', () => {
            let value = Number(this.programPeriod.value);
            value = Math.max(value, 0);
            this.programPeriod.value = value.toString();
            console.log(this.programPeriod.value);
        });
        this.programPeriodTimeType.addEventListener('sl-change', () => {
            console.log(this.programPeriodTimeType.value);
        });
        this.programWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.programWaterFlow.value);
            value = Math.min(Math.max(value, 0), 100);
            this.programWaterFlow.value = value.toString();
            console.log(this.programWaterFlow.value);
        });
        this.programStart.addEventListener('sl-change', () => {
            console.log(this.programStart.value);
            console.log(Date.parse(this.programStart.value));
        });
        this.programTestButton.addEventListener('click', () => {
            console.log('Test clicked');
        });
        this.programApplyButton.addEventListener('click', () => {
            console.log('Apply clicked');
        });
        this.programStopButton.addEventListener('click', () => {
            console.log('Stop clicked');
        });
    }
}
