import { TinyDropDevice } from "./tinydrop_device.js";
type onDeviceFoundCallback = (device: TinyDropDevice) => void;
type onScanErrorCallback = (error: string) => void;
export interface TinyDropScanner {
    scanning: boolean;
    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void;
    stop(): void;
}
export declare class TinyDropBLEScanner implements TinyDropScanner {
    scanning: boolean;
    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void;
    stop(): void;
}
export declare class TinyDropFakeScanner implements TinyDropScanner {
    scanning: boolean;
    scan(onDeviceFound: onDeviceFoundCallback, onError: onScanErrorCallback, timeoutMs: number): void;
    stop(): void;
    private timers;
}
export {};
