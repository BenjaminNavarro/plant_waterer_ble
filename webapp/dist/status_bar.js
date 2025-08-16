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
    bluetoothStatus = qs('#bluetooth_status');
    wateringStatus = qs('#watering_status');
}
