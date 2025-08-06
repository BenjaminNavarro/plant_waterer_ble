/// <reference types="cordova-plugin-ble-central" />
export class TinyDropBLEDevice {
    name;
    id;
    connected;
    connect(onSuccess, onFailure) {
        ble.connect(this.id, function (services) {
            console.log("connected");
            console.log(services);
            if (onSuccess !== undefined) {
                onSuccess();
            }
        }, function (error) {
            console.log("disconnected");
            console.log(`${error.id} - ${error.name}: ${error.errorMessage}`);
            if (onFailure !== undefined) {
                onFailure();
            }
        });
    }
    disconnect(onSuccess, onFailure) {
    }
}
export class TinyDropFakeDevice {
    name;
    id;
    connected;
    connect(onSuccess, onFailure) {
        setTimeout(() => {
            if (Math.random() < 0.9) {
                console.log("connected");
                if (onSuccess !== undefined) {
                    onSuccess();
                }
            }
            else {
                console.log("disconnected");
                if (onFailure !== undefined) {
                    onFailure();
                }
            }
        }, 1000);
    }
    disconnect(onSuccess, onFailure) {
        if (onSuccess !== undefined) {
            onSuccess();
        }
    }
}
