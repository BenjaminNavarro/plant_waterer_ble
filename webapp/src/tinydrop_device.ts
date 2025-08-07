/// <reference types="cordova-plugin-ble-central" />

// type ConnectSuccessCallback = (a: number, b: number) => number;

export interface TinyDropDevice {
    name: string
    id: string
    readonly connected: boolean

    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void
    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void
}

export class TinyDropBLEDevice implements TinyDropDevice {
    name: string
    id: string
    readonly connected: boolean

    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        ble.connect(this.id, function (services) {
            console.log("connected");
            console.log(services);

            if (onSuccess !== undefined) {
                onSuccess()
            }
        }, function (error: BLECentralPlugin.BLEError) {
            console.log("disconnected");
            console.log(`${error.id} - ${error.name}: ${error.errorMessage}`)

            if (onFailure !== undefined) {
                onFailure()
            }
        });
    }

    disconnect(onSuccess: CallableFunction, onFailure?: CallableFunction): void {

    }
}

export class TinyDropFakeDevice implements TinyDropDevice {
    name: string
    id: string
    readonly connected: boolean

    connect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        setTimeout(() => {
            if (Math.random() < 0.5) {
                console.log("connected");

                if (onSuccess !== undefined) {
                    onSuccess()
                }
            }
            else {
                console.log("disconnected");

                if (onFailure !== undefined) {
                    onFailure()
                }
            }
        }, 1000)
    }

    disconnect(onSuccess?: CallableFunction, onFailure?: CallableFunction): void {
        if (onSuccess !== undefined) {
            onSuccess()
        }
    }
}