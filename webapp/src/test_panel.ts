import { convertDuration, qs } from './utils.js'
import { ProgressUI, ResultType } from './progress_ui.js'
import { TinyDropDevice } from './tinydrop_device.js'

export class ManualPanel {
    device: TinyDropDevice

    private manualWaterFlow = qs<HTMLInputElement>('#manual_flow_speed')
    private manualStartButton = qs<HTMLButtonElement>('#manual_start_button')
    private manualStopButton = qs<HTMLButtonElement>('#manual_stop_button')
    private manualDuration = qs<HTMLInputElement>('#manual_duration')
    private manualDurationTimeType = qs<HTMLInputElement>('#manual_duration_time_type')

    private progressUI: ProgressUI
    private duration = 0
    private waterFlow = 0

    constructor(progressUI: ProgressUI) {
        this.progressUI = progressUI

        let log = (value: any) => {
            console.log(`[Manual] ${value}`)
        }

        this.manualDuration.addEventListener('sl-change', () => {
            let value = Number(this.manualDuration.value)
            value = Math.max(value, 0)
            this.duration = convertDuration(value, this.manualDurationTimeType.value)
            this.manualDuration.value = value.toString()
        })

        this.manualDurationTimeType.addEventListener('sl-change', () => {
            this.duration = convertDuration(Math.max(Number(this.manualDuration.value), 0), this.manualDurationTimeType.value)
        })

        this.manualWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.manualWaterFlow.value)
            this.waterFlow = Math.min(Math.max(value, 0), 100)
            this.manualWaterFlow.value = this.waterFlow.toString()
        })

        this.manualStartButton.addEventListener('click', () => {
            this.startWatering()
        })

        this.manualStopButton.addEventListener('click', () => {
            this.stopWatering()
        })
    }

    private startWatering() {
        if (this.device != null && !this.device.wateringState()) {
            this.progressUI.startAutoUpdate(this.duration * 1000)
            this.device.startWatering(this.duration, this.waterFlow)
        }
    }

    private stopWatering() {
        if (this.device != null && this.device.wateringState()) {
            this.progressUI.stop(ResultType.None)
            this.device.stopWatering()
        }
    }
}
