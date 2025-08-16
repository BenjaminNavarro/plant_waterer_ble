import { qs } from './utils.js'
import { ProgressUI } from './progress_ui.js'

export class ManualPanel {
    manualWaterFlow = qs<HTMLInputElement>('#manual_flow_speed')
    manualStartButton = qs<HTMLButtonElement>('#manual_start_button')
    manualStopButton = qs<HTMLButtonElement>('#manual_stop_button')
    manualDuration = qs<HTMLInputElement>('#manual_duration')
    manualDurationTimeType = qs<HTMLInputElement>('#manual_duration_time_type')

    constructor(progressUI: ProgressUI) {
        let log = (value: any) => {
            console.log(`[Manual] ${value}`)
        }

        this.manualDuration.addEventListener('sl-change', () => {
            let value = Number(this.manualDuration.value)
            value = Math.max(value, 0)
            this.manualDuration.value = value.toString()
            log(this.manualDuration.value)
        })

        this.manualDurationTimeType.addEventListener('sl-change', () => {
            log(this.manualDurationTimeType.value)
        })

        this.manualWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.manualWaterFlow.value)
            value = Math.min(Math.max(value, 0), 100)
            this.manualWaterFlow.value = value.toString()
            log(this.manualWaterFlow.value)
        })

        this.manualStartButton.addEventListener('click', () => {
            log('Start clicked')
        })

        this.manualStopButton.addEventListener('click', () => {
            log('Stop clicked')
        })
    }
}
