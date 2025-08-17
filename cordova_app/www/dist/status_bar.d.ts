export declare class StatusBar {
    constructor();
    setBluetoothState(state: boolean): void;
    setWateringState(state: boolean): void;
    setWateringProgress(progress: number): void;
    startWateringProgressAutoUpdate(durationSec: number, startValue?: number): void;
    stopWateringProgressAutoUpdate(): void;
    private bluetoothStatus;
    private wateringStatus;
    private wateringProgress;
    private autoUpdateInterval;
    readonly updateRateMs = 100;
}
