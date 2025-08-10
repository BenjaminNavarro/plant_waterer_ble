import { qs } from './utils.js';
export class TestPanel {
    testWaterFlow = qs('#test_flow_speed');
    testTestButton = qs('#test_test_button');
    testStopButton = qs('#test_stop_button');
    constructor(progressUI) {
        let log = (value) => {
            console.log(`[Test] ${value}`);
        };
        this.testWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.testWaterFlow.value);
            value = Math.min(Math.max(value, 0), 100);
            this.testWaterFlow.value = value.toString();
            log(this.testWaterFlow.value);
        });
        this.testTestButton.addEventListener('click', () => {
            log('Test clicked');
        });
        this.testStopButton.addEventListener('click', () => {
            log('Stop clicked');
        });
    }
}
