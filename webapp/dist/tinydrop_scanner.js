/// <reference types="cordova-plugin-ble-central" />
import { TinyDropFakeDevice } from "./tinydrop_device.js";
export class TinyDropBLEScanner {
    scanning = false;
    scan(onDeviceFound, onError, timeoutMs) {
        if (this.scanning) {
            console.log('BLE scan already in progress');
            return;
        }
        ble.startScan([], (device) => {
            let dev = new TinyDropFakeDevice();
            dev.name = device.name;
            dev.id = device.id;
            onDeviceFound(dev);
        }, (error) => {
            onError(error);
        });
        this.scanning = true;
        setTimeout(this.stop, timeoutMs);
    }
    stop() {
        if (this.scanning) {
            ble.stopScan(() => {
                this.scanning = false;
            }, () => {
                console.log('Failed to stop BLE scan');
            });
        }
    }
}
export class TinyDropFakeScanner {
    scanning = false;
    scan(onDeviceFound, onError, timeoutMs) {
        if (this.scanning) {
            console.log('Fake scan already in progress');
            return;
        }
        this.timers = new Array(3);
        for (let index = 0; index < 3; index++) {
            this.timers[index] = setTimeout(() => {
                let dev = new TinyDropFakeDevice();
                dev.name = "Device #" + (index + 1).toString();
                dev.id = index.toString();
                onDeviceFound(dev);
            }, (index + 1) * 500);
        }
        this.scanning = true;
    }
    stop() {
        if (this.scanning) {
            this.timers.forEach(timer => {
                clearTimeout(timer);
            });
            this.scanning = false;
        }
    }
    timers;
}
