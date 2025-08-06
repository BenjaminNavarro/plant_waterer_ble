/// <reference types="cordova-plugin-ble-central" />

import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from "./tinydrop_device.js";

type onDeviceFoundCallback = (device: TinyDropDevice) => void
type onScanErrorCallback = (error: string) => void

export interface TinyDropScanner {
    scanning: boolean

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void
    stop(): void
}

export class TinyDropBLEScanner implements TinyDropScanner {
    scanning: boolean = false

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void {
        if (this.scanning) {
            console.log('BLE scan already in progress');
            return
        }

        ble.startScan(
            [],
            (device: BLECentralPlugin.PeripheralData) => {
                let dev = new TinyDropFakeDevice()
                dev.name = device.name
                dev.id = device.id

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

export class TinyDropFakeScanner implements TinyDropScanner {
    scanning: boolean = false

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void {
        if (this.scanning) {
            console.log('Fake scan already in progress');
            return
        }

        this.timers = new Array<number>(3)

        for (let index = 0; index < 3; index++) {
            this.timers[index] = setTimeout(() => {
                let dev = new TinyDropFakeDevice()
                dev.name = "Device #" + (index + 1).toString()
                dev.id = index.toString()

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