import { convertDuration, qs } from './utils.js';
import { ResultType } from './progress_ui.js';
import { WateringProgram } from './tinydrop_device.js';
export class ConfigPanel {
    device;
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
    progressUI;
    program = new WateringProgram();
    constructor(progressUI) {
        this.progressUI = progressUI;
        let log = (value) => {
            console.log(`[Program] ${value}`);
        };
        this.programEnabled.addEventListener('sl-change', (ev) => {
            this.program.enabled = qs('#program_enabled').checked;
        });
        this.programDuration.addEventListener('sl-change', () => {
            let value = Number(this.programDuration.value);
            value = Math.max(value, 0);
            this.program.duration = convertDuration(value, this.programDurationTimeType.value);
            this.programDuration.value = value.toString();
        });
        this.programDurationTimeType.addEventListener('sl-change', () => {
            this.program.duration = convertDuration(Math.max(Number(this.programDuration.value), 0), this.programDurationTimeType.value);
        });
        this.programPeriod.addEventListener('sl-change', () => {
            let value = Number(this.programPeriod.value);
            value = Math.max(value, 0);
            this.program.period = convertDuration(value, this.programPeriodTimeType.value);
            this.programPeriod.value = value.toString();
        });
        this.programPeriodTimeType.addEventListener('sl-change', () => {
            this.program.period = convertDuration(Math.max(Number(this.programPeriod.value), 0), this.programPeriodTimeType.value);
        });
        this.programWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.programWaterFlow.value);
            this.program.waterFlow = Math.min(Math.max(value, 0), 100);
            this.programWaterFlow.value = this.program.waterFlow.toString();
        });
        this.programStart.addEventListener('sl-change', () => {
            log(this.programStart.value);
            this.program.startDate = Date.parse(this.programStart.value);
            log(this.program.startDate);
        });
        this.programTestButton.addEventListener('click', () => {
            this.startWatering();
        });
        this.programApplyButton.addEventListener('click', () => {
            this.sendProgram();
        });
        this.programStopButton.addEventListener('click', () => {
            this.stopWatering();
        });
    }
    startWatering() {
        if (this.device != null && !this.device.wateringState()) {
            this.progressUI.startAutoUpdate(this.program.duration * 1000);
            this.device.startWatering(this.program.duration, this.program.waterFlow);
        }
    }
    stopWatering() {
        if (this.device != null && this.device.wateringState()) {
            this.progressUI.stop(ResultType.None);
            this.device.stopWatering();
        }
    }
    sendProgram() {
        this.device.sendProgram(this.program);
    }
}
