import { StatusBar } from "./status_bar.js";
import { TinyDropDevice } from "./tinydrop_device.js";
type onDeviceFoundCallback = (device: TinyDropDevice) => void;
type onScanErrorCallback = (error: string) => void;
export declare abstract class TinyDropScanner {
    protected scanning: boolean;
    protected statusBar: StatusBar;
    constructor(statusBar: StatusBar);
    abstract scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void;
    abstract stop(): void;
    scanningState(): boolean;
}
export declare class TinyDropBLEScanner extends TinyDropScanner {
    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void;
    stop(): void;
}
export declare class TinyDropFakeScanner extends TinyDropScanner {
    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void;
    stop(): void;
    private timers;
}
export {};
