import { qs } from "./utils.js";
export class StatusBar {
    constructor() {
        this.setBluetoothState(false);
        this.setWateringState(false);
    }
    setBluetoothState(state) {
        if (state) {
            this.bluetoothStatus.classList.replace('bluetooth-disconnected', 'bluetooth-connected');
        }
        else {
            this.bluetoothStatus.classList.replace('bluetooth-connected', 'bluetooth-disconnected');
        }
    }
    setWateringState(state) {
        if (state) {
            this.wateringStatus.classList.replace('watering-off', 'watering-on');
        }
        else {
            this.wateringStatus.classList.replace('watering-on', 'watering-off');
        }
    }
    setWateringProgress(progress) {
        progress = Math.max(Math.min(progress, 100), 0);
        this.wateringProgress.value = progress;
    }
    startWateringProgressAutoUpdate(durationSec, startValue) {
        if (startValue !== undefined) {
            startValue = Math.max(Math.min(startValue, 100), 0);
        }
        else {
            startValue = 0;
        }
        durationSec = Math.max(durationSec, 0);
        let progress = startValue;
        this.stopWateringProgressAutoUpdate();
        this.setWateringProgress(progress);
        this.autoUpdateInterval = setInterval(() => {
            this.setWateringProgress(progress);
            progress += 100 / (durationSec * 1000 / this.updateRateMs);
            if (progress >= 100) {
                progress = 100;
                this.stopWateringProgressAutoUpdate();
                this.setWateringProgress(progress);
            }
        }, this.updateRateMs);
    }
    stopWateringProgressAutoUpdate() {
        clearInterval(this.autoUpdateInterval);
    }
    bluetoothStatus = qs('#bluetooth_status');
    wateringStatus = qs('#watering_status');
    wateringProgress = qs('#watering_progress');
    autoUpdateInterval = -1;
    updateRateMs = 100;
}
