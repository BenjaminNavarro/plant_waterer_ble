import { BleClient as ble } from '@capacitor-community/bluetooth-le';
import { StatusBar } from "./status_bar.js"
import type { timeoutId } from "./utils.js"

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


export class WateringStateData {
    watering = false
    duration = 0
    startDate = 0

    static readonly expectedBufferSize = 1 + 4 + 4

    static fromBuffer(buffer: ArrayBuffer): WateringStateData {
        let data = new WateringStateData()
        let view = new DataView(buffer)
        data.watering = view.getUint8(0) == 1
        data.duration = view.getUint32(0 + 1)
        data.startDate = view.getUint32(0 + 1 + 4)
        return data
    }

    toBuffer(): ArrayBuffer {
        let buffer = new ArrayBuffer(WateringStateData.expectedBufferSize)
        let view = new DataView(buffer)
        view.setUint8(0, this.watering ? 1 : 0)
        view.setUint32(0 + 1, this.duration)
        view.setUint32(0 + 1 + 4, this.startDate)
        return buffer
    }
}

export abstract class TinyDropDevice {

    connected = false

    private deviceName: string = ''
    private deviceId: string = ''
    private wateringStateData = new WateringStateData()
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

            this.connected = true
            this.updateStatusBar()
        }, () => {
            if (onFailure !== undefined) {
                onFailure()
            }

            this.connected = false
            this.updateStatusBar()
        })
    }

    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        this.stopNotifications()
        this.doDisconnect(() => {
            if (onSuccess !== undefined) {
                onSuccess()
            }

            this.connected = false
            this.updateStatusBar()
        }, () => {
            if (onFailure !== undefined) {
                onFailure()
            }

            this.connected = false
            this.updateStatusBar()
        })
    }

    wateringState(): WateringStateData {
        return this.wateringStateData
    }

    abstract startWatering(durationSec: number, flowSpeed: number): void
    abstract stopWatering(): void
    abstract sendProgram(program: WateringProgram): void
    abstract readProgram(): WateringProgram

    protected onNotification(type: detail.NotificationType, data: ArrayBuffer) {
        switch (type) {
            case detail.NotificationType.WateringState:
                if (data.byteLength != WateringStateData.expectedBufferSize) {
                    this.log(`incorrect watering state data length: got ${data.byteLength}, expected ${WateringStateData.expectedBufferSize}`)
                    return
                }
                this.wateringStateData = WateringStateData.fromBuffer(data)

                this.updateStatusBar()
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

    private updateStatusBar() {
        this.statusBar.setBluetoothState(this.connected)
        this.statusBar.setWateringState(this.wateringState().watering)

        const nowSec = Date.now() / 1000
        if (this.wateringStateData.duration > 0) {
            this.statusBar.startWateringProgressAutoUpdate(
                this.wateringStateData.duration,
                ((nowSec - this.wateringStateData.startDate) /
                    this.wateringStateData.duration) * 100)
        }
        else {
            this.statusBar.stopWateringProgressAutoUpdate()
            this.statusBar.setWateringProgress(0)
        }
    }
}

export class TinyDropBLEDevice extends TinyDropDevice {

    override doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        // ble.connect(this.id(), function (services) {
        //     this.log("connected");
        //     this.log(services);

        //     if (onSuccess !== undefined) {
        //         onSuccess()
        //     }

        // }, function (error: BLECentralPlugin.BLEError) {
        //     this.log("disconnected");
        //     this.log(`${error.id} - ${error.name}: ${error.errorMessage}`)

        //     if (onFailure !== undefined) {
        //         onFailure()
        //     }
        // })
    }

    override doDisconnect(onSuccess: CallableFunction, onFailure?: CallableFunction): void {
        // ble.disconnect(this.id(), () => {
        //     onSuccess()
        // }, (error: BLECentralPlugin.BLEError) => {
        //     this.log(error)
        //     onFailure()
        // })
    }

    override startNotifications(): void {
        // ble.startNotification(this.id(), detail.service_uuid, detail.watering_state_uuid, (data: ArrayBuffer) => {
        //     this.onNotification(detail.NotificationType.WateringState, data)
        // }, (error: BLECentralPlugin.BLEError) => {
        //     this.log("watering start notification error: " + error)
        // })
    }

    override stopNotifications(): void {
        // ble.stopNotification(this.id(), detail.service_uuid, detail.watering_state_uuid, null, (error: BLECentralPlugin.BLEError) => {
        //     this.log("watering stop notification error: " + error)
        // })
    }

    override startWatering(durationSec: number, flowSpeed: number): void {

    }

    override stopWatering(): void {

    }

    override sendProgram(program: WateringProgram): void {

    }

    override readProgram(): WateringProgram {
        let program = new WateringProgram()
        return program
    }

    protected log(value: any): void {
        console.log(`[BLEDevice] ${value}`)
    }
}

export class TinyDropFakeDevice extends TinyDropDevice {
    private successRate = 0.95
    private deviceDelayMs = 500
    private notificationsStarted = false
    private stopTimeoutHandle: timeoutId = null
    private wateringTimeoutHandle: timeoutId = null
    private wateringIntervalHandle: timeoutId = null
    private internalWateringState = new WateringStateData()

    constructor(statusBar: StatusBar, name: string, id: string) {
        super(statusBar, name, id)

        let program = this.readProgram()
        if (program != null && program.enabled) {
            this.sendProgram(program)
        }

        this.internalWateringState.startDate = Date.now() / 1000
    }

    override doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        setTimeout(() => {
            if (Math.random() < this.successRate) {
                this.log("connected");

                if (onSuccess !== undefined) {
                    onSuccess()
                }

                this.onNotification(detail.NotificationType.WateringState, this.internalWateringState.toBuffer())
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
        if (this.wateringState().watering) {
            return
        }

        this.sendWateringNotification(false)

        setTimeout(() => {
            this.internalWateringState.startDate = Date.now() / 1000
            this.sendWateringNotification(true, durationSec)
        }, this.deviceDelayMs)

        clearTimeout(this.stopTimeoutHandle)
        this.stopTimeoutHandle = setTimeout(() => {
            this.sendWateringNotification(false)
        }, durationSec * 1000 + this.deviceDelayMs)
    }

    override stopWatering(): void {
        if (!this.wateringState().watering) {
            return
        }

        clearTimeout(this.stopTimeoutHandle)

        setTimeout(() => {
            this.sendWateringNotification(false)
        }, this.deviceDelayMs)
    }

    override sendProgram(program: WateringProgram): void {
        clearInterval(this.wateringIntervalHandle)

        clearTimeout(this.wateringTimeoutHandle)

        localStorage.setItem('program', JSON.stringify(program))

        if (program.enabled) {
            const startDate = this.computeWateringStart(program)
            this.wateringTimeoutHandle = setTimeout(() => {
                this.startWatering(program.duration, program.waterFlow)
                this.wateringIntervalHandle = setInterval(() => {
                    this.startWatering(program.duration, program.waterFlow)
                }, program.period * 1000)
            }, (startDate * 1000 - Date.now()))
        }
    }

    override readProgram(): WateringProgram {
        const savedProgram = localStorage.getItem('program')
        if (savedProgram != null) {
            const savedProgramJS = JSON.parse(savedProgram)
            let program = new WateringProgram()
            program.enabled = savedProgramJS['enabled']
            program.duration = savedProgramJS['duration']
            program.period = savedProgramJS['period']
            program.waterFlow = savedProgramJS['waterFlow']
            program.startDate = savedProgramJS['startDate']

            return program
        }
        else {
            return null
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

    private sendWateringNotification(state: boolean, duration: number = 0) {
        this.internalWateringState.watering = state
        this.internalWateringState.duration = state ? duration : 0
        this.onNotification(detail.NotificationType.WateringState, this.internalWateringState.toBuffer())

    }

    protected log(value: any): void {
        console.log(`[FakeDevice] ${value}`)
    }
}