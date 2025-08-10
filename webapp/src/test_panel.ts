import { qs } from './utils.js'
import { ProgressUI } from './progress_ui.js'

export class TestPanel {
    testWaterFlow = qs<HTMLInputElement>('#test_flow_speed')
    testTestButton = qs<HTMLButtonElement>('#test_test_button')
    testStopButton = qs<HTMLButtonElement>('#test_stop_button')

    constructor(progressUI: ProgressUI) {
        let log = (value: any) => {
            console.log(`[Test] ${value}`)
        }

        this.testWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.testWaterFlow.value)
            value = Math.min(Math.max(value, 0), 100)
            this.testWaterFlow.value = value.toString()
            log(this.testWaterFlow.value)
        })

        this.testTestButton.addEventListener('click', () => {
            log('Test clicked')
        })

        this.testStopButton.addEventListener('click', () => {
            log('Stop clicked')
        })
    }
}
