import { convertDuration, convertDurationToBestUnit, qs } from './utils.js';
import { WateringProgram } from './tinydrop_device.js';
export class ConfigPanel {
    programEnabled = qs('#program_enabled');
    programEnabledElem = qs('#program_enabled');
    programDuration = qs('#program_duration');
    programDurationTimeType = qs('#program_duration_time_type');
    programPeriod = qs('#program_period');
    programPeriodTimeType = qs('#program_period_time_type');
    programWaterFlow = qs('#program_flow_speed');
    programStart = qs('#program_start');
    programTestButton = qs('#program_test_button');
    programApplyButton = qs('#program_apply_button');
    programStopButton = qs('#program_stop_button');
    device;
    progressUI;
    program = new WateringProgram();
    constructor(progressUI) {
        this.progressUI = progressUI;
        let log = (value) => {
            console.log(`[Program] ${value}`);
        };
        this.programEnabledElem.addEventListener('sl-change', (ev) => {
            this.program.enabled = this.programEnabled.checked;
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
            this.program.startDate = Date.parse(this.programStart.value) / 1000;
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
    setDevice(device) {
        this.device = device;
        if (this.device != null) {
            let savedProgram = this.device.readProgram();
            if (savedProgram != null) {
                this.program = savedProgram;
                this.program.enabled = this.program.enabled;
                this.programEnabled.checked = this.program.enabled;
                const durationConv = convertDurationToBestUnit(this.program.duration);
                this.programDuration.value = durationConv.duration.toString();
                this.programDurationTimeType.value = durationConv.unit;
                const periodConv = convertDurationToBestUnit(this.program.period);
                this.programPeriod.value = periodConv.duration.toString();
                this.programPeriodTimeType.value = periodConv.unit;
                this.programWaterFlow.value = this.program.waterFlow.toString();
                const date = new Date(this.program.startDate * 1000);
                const year = date.getFullYear();
                const month = date.getMonth() + 1;
                const monthPrefix = month < 10 ? '0' : '';
                const day = date.getDate();
                const hours = date.getHours();
                const minutes = date.getMinutes();
                const formattedDate = `${year}-${monthPrefix}${month}-${day}T${hours}:${minutes}`;
                this.programStart.value = formattedDate;
            }
        }
    }
    startWatering() {
        if (this.device != null && !this.device.wateringState().watering) {
            this.device.startWatering(this.program.duration, this.program.waterFlow);
        }
    }
    stopWatering() {
        if (this.device != null && this.device.wateringState().watering) {
            this.device.stopWatering();
        }
    }
    sendProgram() {
        this.device.sendProgram(this.program);
    }
}
