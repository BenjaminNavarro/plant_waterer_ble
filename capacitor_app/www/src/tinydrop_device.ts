import { BleClient as ble, type BleService } from '@capacitor-community/bluetooth-le';
import { StatusBar } from "./status_bar.ts"
import type { timeoutId } from "./utils.ts"
import { logger } from './logger.ts';

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

export type programReadCallback = (program: WateringProgram) => void

export abstract class TinyDropDevice {

    connected = false

    protected deviceName: string = ''
    private deviceId: string = ''
    private wateringStateData = new WateringStateData()
    private statusBar: StatusBar
    private outputCount = 0

    constructor(statusBar: StatusBar, name: string, id: string, outputCount: number) {
        this.statusBar = statusBar
        this.deviceName = name
        this.deviceId = id
        this.outputCount = outputCount
    }

    name(): string {
        return this.deviceName
    }

    id(): string {
        return this.deviceId
    }

    outputs(): number {
        return this.outputCount
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

    abstract setName(new_name: string): void
    abstract startWatering(durationSec: number, flowSpeed: number): void
    abstract stopWatering(): void
    abstract sendProgram(program: WateringProgram): void
    abstract readProgram(onSuccess: programReadCallback): void

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
        logger.log(`[Device] ${value}`)
    }

    protected updateStatusBar() {
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
        const onConnect = () => {
            logger.log("connected")
            ble.getServices(this.id()).then((services: BleService[]) => {
                let uuids = []
                services.forEach(service => {
                    uuids.push(service.uuid)
                })

                // TODO check expected services
                logger.log('services: ' + uuids.toString())

                this.sendCurrentTime()

                if (onSuccess !== undefined) {
                    onSuccess()
                }
            })


        }

        const onDisconnect = () => {
            this.log("disconnected")

            this.connected = false
            this.updateStatusBar()
        }

        const onError = (reason: any) => {
            this.log("connection error")
            this.log(reason)
            if (onFailure !== undefined) {
                onFailure()
            }
        }

        this.log('connecting to ' + this.id())
        ble.connect(this.id(), onDisconnect).then(onConnect, onError)
    }

    override doDisconnect(onSuccess: CallableFunction, onFailure?: CallableFunction): void {
        ble.disconnect(this.id()).then(() => {
            onSuccess()
        }).catch((reason: any) => {
            this.log(reason)
            onFailure()
        })
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

    override setName(new_name: string): void {
        logger.log(`setName: ${new_name} (${new_name.length})`)
        let asciiKeys = new Array<number>();
        for (var i = 0; i < new_name.length; i++) {
            asciiKeys.push(new_name[i].charCodeAt(0));
        }
        const encoder = new TextEncoder()
        const array = encoder.encode(new_name)
        const data = new DataView(array.buffer, array.length - new_name.length, new_name.length)
        ble.write(this.id(), "4f736c21-2054-4786-93fe-a5c4b028dbef", "b8b4c3af-fa31-4de4-9fa1-a26ea5da7f0b", data).then(
            () => {
                logger.log(`Name ${new_name} sent to device`)
            }, (reason: any) => {
                logger.log(`Can't send name to device`)
                logger.log(reason)
            })
    }

    override startWatering(durationSec: number, flowSpeed: number): void {

    }

    override stopWatering(): void {

    }

    override sendProgram(program: WateringProgram): void {

    }

    override readProgram(onSuccess: programReadCallback): void {
        logger.log('Reading program from device...')
        ble.read(this.id(), '2f675585-e40a-c088-6941-b245883c4e3a', '3496dac7-7885-4c9c-8e54-0ffebd805485').then(
            (data: DataView) => {
                if (data.byteLength != 16) {
                    logger.log(`Invalid data size for program. Got ${data.byteLength}, expected 16`)
                    return
                }

                let program = new WateringProgram()
                program.startDate = Number(data.getBigUint64(0, true))
                program.period = data.getUint32(8, true)
                program.duration = data.getUint16(8 + 4, true)
                program.waterFlow = data.getUint8(8 + 4 + 2)
                program.enabled = data.getUint8(8 + 4 + 2 + 1) != 0
                logger.log('Program received: ' + JSON.stringify(program))
                onSuccess(program)
            }, (reason: any) => {
                logger.log(`Can't read program from device`)
                logger.log(reason)
            })
    }

    private sendCurrentTime(): void {
        const now = Math.round(Date.now() / 1000)
        const buffer = new ArrayBuffer(8)
        const dataView = new DataView(buffer)
        const nowBitInt = BigInt(now)
        dataView.setBigInt64(0, nowBitInt, true)
        ble.write(this.id(), "87f4d02e-698f-4c46-91f2-5f714c877b0a", "21bc4af5-44f0-4a7b-aa36-a110a0ac0ad2", dataView).then(
            () => {
                logger.log(`Time ${now} sent to device`)
            }, (reason: any) => {
                logger.log(`Can't send time to device`)
                logger.log(reason)
            })
    }

    protected log(value: any): void {
        logger.log(`[BLEDevice] ${value}`)
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

    constructor(statusBar: StatusBar, name: string, id: string, outputCount: number) {
        super(statusBar, name, id, outputCount)

        this.readProgram((program: WateringProgram) => {
            if (program.enabled) {
                this.sendProgram(program)
            }
        })

        let savedName = localStorage.getItem('deviceName')
        if (savedName != null) {
            this.deviceName = savedName
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

    override setName(new_name: string): void {
        this.deviceName = new_name
        localStorage.setItem('deviceName', new_name)
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

    override readProgram(onSuccess: programReadCallback): void {
        const savedProgram = localStorage.getItem('program')
        if (savedProgram != null) {
            const savedProgramJS = JSON.parse(savedProgram)
            let program = new WateringProgram()
            program.enabled = savedProgramJS['enabled']
            program.duration = savedProgramJS['duration']
            program.period = savedProgramJS['period']
            program.waterFlow = savedProgramJS['waterFlow']
            program.startDate = savedProgramJS['startDate']

            onSuccess(program)
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
        logger.log(`[FakeDevice] ${value}`)
    }
}