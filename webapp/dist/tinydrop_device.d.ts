export interface TinyDropDevice {
    name: string;
    id: string;
    readonly connected: boolean;
    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
}
export declare class TinyDropBLEDevice implements TinyDropDevice {
    name: string;
    id: string;
    readonly connected: boolean;
    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    disconnect(onSuccess: CallableFunction, onFailure?: CallableFunction): void;
}
export declare class TinyDropFakeDevice implements TinyDropDevice {
    name: string;
    id: string;
    readonly connected: boolean;
    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void;
}
