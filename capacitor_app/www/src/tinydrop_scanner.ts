import { BleClient as ble } from '@capacitor-community/bluetooth-le';
import type { ScanResult } from '@capacitor-community/bluetooth-le';
import { StatusBar } from "./status_bar.ts";
import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from "./tinydrop_device.ts";
import type { timeoutId } from './utils.ts'

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
    constructor(statusBar: StatusBar) {
        super(statusBar)

    }

    async scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): Promise<void> {
        if (this.scanning) {
            console.log('BLE scan already in progress');
            return
        }

        // Ask for permissions if needed
        await ble.initialize()

        ble.requestLEScan(
            {
                services: ['2f675585-e40a-c088-6941-b245883c4e3a']
            },
            (result: ScanResult) => {
                console.log('received new scan result', result)
                console.log(result)

                let dev = new TinyDropBLEDevice(this.statusBar, result.localName, result.device.deviceId)
                onDeviceFound(dev)
            }
        )

        // ble.startScan(
        //     [],
        //     (device: BLECentralPlugin.PeripheralData) => {
        //         console.log('Device found:');
        //         console.log(device)

        //         let dev = new TinyDropBLEDevice(this.statusBar, device.name, device.id)

        //         onDeviceFound(dev)
        //     },
        //     (error: string) => {
        //         onError(error)
        //     }
        // )

        this.scanning = true

        setTimeout(
            this.stop,
            timeoutMs
        )
    }

    stop(): void {
        if (this.scanning) {
            ble.stopLEScan()
            this.scanning = false
        }
    }
}

export class TinyDropFakeScanner extends TinyDropScanner {

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void {
        if (this.scanning) {
            console.log('Fake scan already in progress');
            return
        }

        this.timers = new Array<timeoutId>(3)

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

    private timers: Array<timeoutId>
}