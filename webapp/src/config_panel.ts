import { convertDuration, qs } from './utils.js'
import { ProgressUI, ResultType } from './progress_ui.js'
import { TinyDropDevice, WateringProgram } from './tinydrop_device.js'
import { SlChangeEvent, SlSwitch } from 'shoelace/shoelace.js'

export class ConfigPanel {
    device: TinyDropDevice

    private programEnabled = qs<HTMLInputElement>('#program_enabled')
    private programDuration = qs<HTMLInputElement>('#program_duration')
    private programDurationTimeType = qs<HTMLInputElement>('#program_duration_time_type')
    private programPeriod = qs<HTMLInputElement>('#program_period')
    private programPeriodTimeType = qs<HTMLInputElement>('#program_period_time_type')
    private programWaterFlow = qs<HTMLInputElement>('#program_flow_speed')
    private programStart = qs<HTMLInputElement>('#program_start')
    private programTestButton = qs<HTMLButtonElement>('#program_test_button')
    private programApplyButton = qs<HTMLButtonElement>('#program_apply_button')
    private programStopButton = qs<HTMLButtonElement>('#program_stop_button')

    private progressUI: ProgressUI
    private program = new WateringProgram()

    constructor(progressUI: ProgressUI) {
        this.progressUI = progressUI

        let log = (value: any) => {
            console.log(`[Program] ${value}`)
        }

        this.programEnabled.addEventListener('sl-change', (ev: SlChangeEvent) => {
            this.program.enabled = qs<SlSwitch>('#program_enabled').checked
        })

        this.programDuration.addEventListener('sl-change', () => {
            let value = Number(this.programDuration.value)
            value = Math.max(value, 0)
            this.program.duration = convertDuration(value, this.programDurationTimeType.value)
            this.programDuration.value = value.toString()
        })

        this.programDurationTimeType.addEventListener('sl-change', () => {
            this.program.duration = convertDuration(Math.max(Number(this.programDuration.value), 0), this.programDurationTimeType.value)
        })

        this.programPeriod.addEventListener('sl-change', () => {
            let value = Number(this.programPeriod.value)
            value = Math.max(value, 0)
            this.program.period = convertDuration(value, this.programPeriodTimeType.value)
            this.programPeriod.value = value.toString()
        })

        this.programPeriodTimeType.addEventListener('sl-change', () => {
            this.program.period = convertDuration(Math.max(Number(this.programPeriod.value), 0), this.programPeriodTimeType.value)
        })

        this.programWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.programWaterFlow.value)
            this.program.waterFlow = Math.min(Math.max(value, 0), 100)
            this.programWaterFlow.value = this.program.waterFlow.toString()
        })

        this.programStart.addEventListener('sl-change', () => {
            log(this.programStart.value)
            this.program.startDate = Date.parse(this.programStart.value)
            log(this.program.startDate)

        })

        this.programTestButton.addEventListener('click', () => {
            this.startWatering()
        })

        this.programApplyButton.addEventListener('click', () => {
            this.sendProgram()
        })

        this.programStopButton.addEventListener('click', () => {
            this.stopWatering()
        })
    }

    private startWatering() {
        if (this.device != null && !this.device.wateringState()) {
            this.progressUI.startAutoUpdate(this.program.duration * 1000)
            this.device.startWatering(this.program.duration, this.program.waterFlow)
        }
    }

    private stopWatering() {
        if (this.device != null && this.device.wateringState()) {
            this.progressUI.stop(ResultType.None)
            this.device.stopWatering()
        }
    }

    private sendProgram() {
        this.device.sendProgram(this.program)
    }
}
