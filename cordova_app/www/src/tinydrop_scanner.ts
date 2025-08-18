/// <reference types="cordova-plugin-ble-central" />

import { StatusBar } from "./status_bar.js";
import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from "./tinydrop_device.js";

type onDeviceFoundCallback = (device: TinyDropDevice) => void
type onScanErrorCallback = (error: string) => void

export abstract class TinyDropScanner {
    protected scanning: boolean
    protected statusBar: StatusBar

    constructor(statusBar: StatusBar) {
        this.statusBar = statusBar
    }

    abstract scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void
    abstract stop(): void

    scanningState(): boolean {
        return this.scanning
    }
}

export class TinyDropBLEScanner extends TinyDropScanner {
    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void {
        if (this.scanning) {
            console.log('BLE scan already in progress');
            return
        }

        ble.startScan(
            [],
            (device: BLECentralPlugin.PeripheralData) => {
                console.log('Device found:');
                console.log(device)

                let dev = new TinyDropBLEDevice(this.statusBar, device.name, device.id)

                onDeviceFound(dev)
            },
            (error: string) => {
                onError(error)
            }
        )

        this.scanning = true

        setTimeout(
            this.stop,
            timeoutMs
        )
    }

    stop(): void {
        if (this.scanning) {
            ble.stopScan(() => {
                this.scanning = false
            }, () => {
                console.log('Failed to stop BLE scan');
            })
        }
    }
}

export class TinyDropFakeScanner extends TinyDropScanner {

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void {
        if (this.scanning) {
            console.log('Fake scan already in progress');
            return
        }

        this.timers = new Array<number>(3)

        for (let index = 0; index < 3; index++) {
            this.timers[index] = setTimeout(() => {
                const name = 'Device #' + (index + 1).toString()
                const id = index.toString()
                let dev = new TinyDropFakeDevice(this.statusBar, name, id)

                onDeviceFound(dev)
            }, (index + 1) * 500)
        }

        this.scanning = true
    }

    stop(): void {
        if (this.scanning) {
            this.timers.forEach(timer => {
                clearTimeout(timer)
            })

            this.scanning = false
        }
    }

    private timers: Array<number>
}