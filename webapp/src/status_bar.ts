import { qs } from "./utils.js"

export class StatusBar {
    constructor() {
        this.setBluetoothState(false)
        this.setWateringState(false)
    }

    setBluetoothState(state: boolean) {
        if (state) {
            this.bluetoothStatus.classList.replace('bluetooth-disconnected', 'bluetooth-connected')
        }
        else {
            this.bluetoothStatus.classList.replace('bluetooth-connected', 'bluetooth-disconnected')
        }
    }

    setWateringState(state: boolean) {
        if (state) {
            this.wateringStatus.classList.replace('watering-off', 'watering-on')
        }
        else {
            this.wateringStatus.classList.replace('watering-on', 'watering-off')
        }
    }

    private bluetoothStatus = qs('#bluetooth_status')
    private wateringStatus = qs('#watering_status')
}