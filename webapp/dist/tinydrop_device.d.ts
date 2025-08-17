import { StatusBar } from "./status_bar.js";
declare namespace detail {
    enum NotificationType {
        WateringState = 0
    }
    const service_uuid = "0x00112233445566";
    const watering_state_uuid = "0x00112233445566";
}
export declare class WateringProgram {
    enabled: boolean;
    duration: number;
    period: number;
    waterFlow: number;
    startDate: number;
}
export declare class WateringStateData {
    watering: boolean;
    duration: number;
    startDate: number;
    static readonly expectedBufferSize: number;
    static fromBuffer(buffer: ArrayBuffer): WateringStateData;
    toBuffer(): ArrayBuffer;
}
export declare abstract class TinyDropDevice {
    connected: boolean;
    private deviceName;
    private deviceId;
    private wateringStateData;
    private statusBar;
    constructor(statusBar: StatusBar, name: string, id: string);
    name(): string;
    id(): string;
    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    wateringState(): WateringStateData;
    abstract startWatering(durationSec: number, flowSpeed: number): void;
    abstract stopWatering(): void;
    abstract sendProgram(program: WateringProgram): void;
    protected onNotification(type: detail.NotificationType, data: ArrayBuffer): void;
    protected abstract doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    protected abstract doDisconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    protected abstract startNotifications(): void;
    protected abstract stopNotifications(): void;
    protected log(value: any): void;
    private updateStatusBar;
}
export declare class TinyDropBLEDevice extends TinyDropDevice {
    doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    doDisconnect(onSuccess: CallableFunction, onFailure?: CallableFunction): void;
    startNotifications(): void;
    stopNotifications(): void;
    startWatering(durationSec: number, flowSpeed: number): void;
    stopWatering(): void;
    sendProgram(program: WateringProgram): void;
    protected log(value: any): void;
}
export declare class TinyDropFakeDevice extends TinyDropDevice {
    private successRate;
    private deviceDelayMs;
    private notificationsStarted;
    private stopTimeoutHandle;
    private wateringTimeoutHandle;
    private wateringIntervalHandle;
    private internalWateringState;
    constructor(statusBar: StatusBar, name: string, id: string);
    doConnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    doDisconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    startNotifications(): void;
    stopNotifications(): void;
    startWatering(durationSec: number, flowSpeed: number): void;
    stopWatering(): void;
    sendProgram(program: WateringProgram): void;
    private computeWateringStart;
    private sendWateringNotification;
    protected log(value: any): void;
}
export {};
