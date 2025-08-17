/// <reference types="cordova-plugin-ble-central" />

import { StatusBar } from "./status_bar.js"

namespace detail {
    export enum NotificationType {
        WateringState
    }

    export const service_uuid = '0x00112233445566'
    export const watering_state_uuid = '0x00112233445566'
}

export class WateringProgram {
    enabled = false
    duration = 0
    period = 0
    waterFlow = 0
    startDate = 0
}

export abstract class TinyDropDevice {

    private deviceName: string = ''
    private deviceId: string = ''
    private watering = false
    private statusBar: StatusBar

    constructor(statusBar: StatusBar, name: string, id: string) {
        this.statusBar = statusBar
        this.deviceName = name
        this.deviceId = id
    }

    name(): string {
        return this.deviceName
    }

    id(): string {
        return this.deviceId
    }

    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        this.doConnect(() => {
            this.startNotifications()
            if (onSuccess !== undefined) {
                onSuccess()
            }
            this.statusBar.setBluetoothState(true)
            this.statusBar.setWateringState(this.wateringState())
        }, () => {
            if (onFailure !== undefined) {
                onFailure()
            }
            this.statusBar.setBluetoothState(false)
            this.statusBar.setWateringState(false)
        })
    }

    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        this.stopNotifications()
        this.doDisconnect(() => {
            if (onSuccess !== undefined) {
                onSuccess()
            }
            this.statusBar.setBluetoothState(false)
            this.statusBar.setWateringState(false)
        }, () => {
            if (onFailure !== undefined) {
                onFailure()
            }
        })
    }

    wateringState(): boolean {
        return this.watering
    }

    abstract startWatering(durationSec: number, flowSpeed: number): void
    abstract stopWatering(): void
    abstract sendProgram(program: WateringProgram): void

    protected onNotification(type: detail.NotificationType, data: ArrayBuffer) {
        switch (type) {
            case detail.NotificationType.WateringState:
                if (data.byteLength != 1) {
                    this.log("incorrect watering state data length");
                    return
                }
                const wateringData = new Uint8Array(data)
                this.watering = wateringData[0] == 1
                this.statusBar.setWateringState(this.watering)
                break
        }
    }

    protected abstract doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void
    protected abstract doDisconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void
    protected abstract startNotifications(): void
    protected abstract stopNotifications(): void

    protected log(value: any): void {
        console.log(`[Device] ${value}`)
    }
}

export class TinyDropBLEDevice extends TinyDropDevice {

    override doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        ble.connect(this.id(), function (services) {
            this.log("connected");
            this.log(services);

            if (onSuccess !== undefined) {
                onSuccess()
            }

        }, function (error: BLECentralPlugin.BLEError) {
            this.log("disconnected");
            this.log(`${error.id} - ${error.name}: ${error.errorMessage}`)

            if (onFailure !== undefined) {
                onFailure()
            }
        })
    }

    override doDisconnect(onSuccess: CallableFunction, onFailure?: CallableFunction): void {
        ble.disconnect(this.id(), () => {
            onSuccess()
        }, (error: BLECentralPlugin.BLEError) => {
            this.log(error)
            onFailure()
        })
    }

    override startNotifications(): void {
        ble.startNotification(this.id(), detail.service_uuid, detail.watering_state_uuid, (data: ArrayBuffer) => {
            this.onNotification(detail.NotificationType.WateringState, data)
        }, (error: BLECentralPlugin.BLEError) => {
            this.log("watering start notification error: " + error)
        })
    }

    override stopNotifications(): void {
        ble.stopNotification(this.id(), detail.service_uuid, detail.watering_state_uuid, null, (error: BLECentralPlugin.BLEError) => {
            this.log("watering stop notification error: " + error)
        })
    }

    override startWatering(durationSec: number, flowSpeed: number): void {

    }

    override stopWatering(): void {

    }

    override sendProgram(program: WateringProgram): void {

    }

    protected log(value: any): void {
        console.log(`[BLEDevice] ${value}`)
    }
}

export class TinyDropFakeDevice extends TinyDropDevice {
    private successRate = 0.5
    private deviceDelayMs = 500
    private notificationsStarted = false
    private stopTimeoutHandle = -1
    private wateringTimeoutHandle = -1
    private wateringIntervalHandle = -1
    private internalWateringState = false

    override doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        setTimeout(() => {
            if (Math.random() < this.successRate) {
                this.log("connected");

                if (onSuccess !== undefined) {
                    onSuccess()
                }

                let data = new Uint8Array(1)
                data[0] = this.internalWateringState ? 1 : 0
                this.onNotification(detail.NotificationType.WateringState, data.buffer)
            }
            else {
                this.log("disconnected");

                if (onFailure !== undefined) {
                    onFailure()
                }
            }
        }, 1000)
    }

    override doDisconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        if (onSuccess !== undefined) {
            onSuccess()
        }
    }

    override startNotifications(): void {
        this.notificationsStarted = true
    }

    override stopNotifications(): void {
        this.notificationsStarted = false
    }

    override startWatering(durationSec: number, flowSpeed: number): void {
        if (this.wateringState()) {
            return
        }

        setTimeout(() => {
            if (this.notificationsStarted) {
                let data = new Uint8Array(1)
                data[0] = 1
                this.onNotification(detail.NotificationType.WateringState, data.buffer)
            }
            this.internalWateringState = true
        }, this.deviceDelayMs)

        clearTimeout(this.stopTimeoutHandle)
        this.stopTimeoutHandle = setTimeout(() => {
            if (this.notificationsStarted) {
                let data = new Uint8Array(1)
                data[0] = 0
                this.onNotification(detail.NotificationType.WateringState, data.buffer)
            }
            this.internalWateringState = false
        }, durationSec * 1000 + this.deviceDelayMs)
    }

    override stopWatering(): void {
        if (!this.wateringState()) {
            return
        }

        clearTimeout(this.stopTimeoutHandle)

        setTimeout(() => {
            if (this.notificationsStarted) {
                let data = new Uint8Array(1)
                data[0] = 0
                this.onNotification(detail.NotificationType.WateringState, data.buffer)
            }
            this.internalWateringState = false
        }, this.deviceDelayMs)
    }

    override sendProgram(program: WateringProgram): void {
        clearInterval(this.wateringIntervalHandle)

        clearTimeout(this.wateringTimeoutHandle)

        if (program.enabled) {
            this.wateringTimeoutHandle = setTimeout(() => {
                this.startWatering(program.duration, program.waterFlow)
                this.wateringIntervalHandle = setInterval(() => {
                    this.startWatering(program.duration, program.waterFlow)
                }, program.period * 1000)
            }, this.computeWateringStart(program) * 1000)
        }
    }

    private computeWateringStart(program: WateringProgram) {
        const now = Date.now() / 1000
        if (program.period == 0) {
            return Infinity
        } else if (program.startDate >= now) {
            // startDate in the future
            return program.startDate
        } else {
            // startDate in the past
            const cycles = (now - program.startDate) / program.period
            return program.startDate + cycles * program.period
        }
    }

    protected log(value: any): void {
        console.log(`[FakeDevice] ${value}`)
    }
}