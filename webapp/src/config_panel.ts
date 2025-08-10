import { qs } from './utils.js'
import { ProgressUI } from './progress_ui.js'

export class ConfigPanel {
    programEnabled = qs<HTMLInputElement>('#program_enabled')
    programDuration = qs<HTMLInputElement>('#program_duration')
    programDurationTimeType = qs<HTMLInputElement>('#program_duration_time_type')
    programPeriod = qs<HTMLInputElement>('#program_period')
    programPeriodTimeType = qs<HTMLInputElement>('#program_period_time_type')
    programWaterFlow = qs<HTMLInputElement>('#program_flow_speed')
    programStart = qs<HTMLInputElement>('#program_start')
    programTestButton = qs<HTMLButtonElement>('#program_test_button')
    programApplyButton = qs<HTMLButtonElement>('#program_apply_button')
    programStopButton = qs<HTMLButtonElement>('#program_stop_button')

    constructor(progressUI: ProgressUI) {
        let log = (value: any) => {
            console.log(`[Program] ${value}`)
        }

        this.programEnabled.addEventListener('sl-change', () => {
            log(this.programEnabled.checked)
        })

        this.programDuration.addEventListener('sl-change', () => {
            let value = Number(this.programDuration.value)
            value = Math.max(value, 0)
            this.programDuration.value = value.toString()
            log(this.programDuration.value)
        })

        this.programDurationTimeType.addEventListener('sl-change', () => {
            log(this.programDurationTimeType.value)
        })

        this.programPeriod.addEventListener('sl-change', () => {
            let value = Number(this.programPeriod.value)
            value = Math.max(value, 0)
            this.programPeriod.value = value.toString()
            log(this.programPeriod.value)
        })

        this.programPeriodTimeType.addEventListener('sl-change', () => {
            log(this.programPeriodTimeType.value)
        })

        this.programWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.programWaterFlow.value)
            value = Math.min(Math.max(value, 0), 100)
            this.programWaterFlow.value = value.toString()
            log(this.programWaterFlow.value)
        })

        this.programStart.addEventListener('sl-change', () => {
            log(this.programStart.value)
            log(Date.parse(this.programStart.value))

        })

        this.programTestButton.addEventListener('click', () => {
            log('Test clicked')
        })

        this.programApplyButton.addEventListener('click', () => {
            log('Apply clicked')
        })

        this.programStopButton.addEventListener('click', () => {
            log('Stop clicked')
        })
    }
}
