import { BleClient as ble } from '@capacitor-community/bluetooth-le';
import type { ScanResult } from '@capacitor-community/bluetooth-le';
import { StatusBar } from "./status_bar.ts";
import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from "./tinydrop_device.ts";
import type { timeoutId } from './utils.ts'
import { logger } from './logger.ts';

type onDeviceFoundCallback = (device: TinyDropDevice) => void
type onScanErrorCallback = (error: string) => void

export abstract class TinyDropScanner {
    scannedDevices = new Array<TinyDropBLEDevice>()
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

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): Promise<void> {
        if (this.scanning) {
            logger.log('BLE scan already in progress');
            return
        }

        // Ask for permissions if needed
        logger.log('Initializing BLE')
        ble.initialize().then(() => {

            logger.log('BLE ready')

            logger.log('Requesting BLE scan')

            const mfgIDtoKey = (id: string): string => {
                if (id.length != 2) {
                    return ''
                }

                const charCode0 = id.charCodeAt(0)
                const charCode1 = id.charCodeAt(1)
                const charCode0Shift = (charCode0 & 0xFF) << 8
                const key = charCode0Shift | charCode1 & 0xFF

                return key.toString()
            }

            const mfgKey1 = mfgIDtoKey('TD')
            const mfgKey2 = mfgIDtoKey('HW')

            this.scannedDevices.forEach(device => {
                if (device.connected) {
                    device.disconnect()
                }
            })
            this.scannedDevices = new Array<TinyDropBLEDevice>()

            ble.requestLEScan(
                {},
                (result: ScanResult) => {
                    if (mfgKey1 in result.manufacturerData && mfgKey2 in result.manufacturerData) {
                        const decoder = new TextDecoder()
                        const userDefinedName = decoder.decode(result.manufacturerData[mfgKey1])

                        const outputCount = result.manufacturerData[mfgKey2].getUint8(0);

                        let dev = new TinyDropBLEDevice(this.statusBar, userDefinedName, result.device.deviceId, outputCount)
                        this.scannedDevices.push(dev)
                        onDeviceFound(dev)
                    }
                }
            )

            this.scanning = true

            setTimeout(
                () => {
                    this.stop()
                },
                timeoutMs
            )

        })
    }

    stop(): void {
        if (this.scanning) {
            logger.log('Stopping BLE scan')
            ble.stopLEScan()
            this.scanning = false
        }
    }
}

export class TinyDropFakeScanner extends TinyDropScanner {

    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void {
        if (this.scanning) {
            logger.log('Fake scan already in progress');
            return
        }

        this.timers = new Array<timeoutId>(3)

        for (let index = 0; index < 3; index++) {
            this.timers[index] = setTimeout(() => {
                const name = 'Device #' + (index + 1).toString()
                const id = index.toString()
                let dev = new TinyDropFakeDevice(this.statusBar, name, id, 1)

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