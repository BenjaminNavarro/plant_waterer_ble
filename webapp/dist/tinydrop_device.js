/// <reference types="cordova-plugin-ble-central" />
var detail;
(function (detail) {
    let NotificationType;
    (function (NotificationType) {
        NotificationType[NotificationType["WateringState"] = 0] = "WateringState";
    })(NotificationType = detail.NotificationType || (detail.NotificationType = {}));
    detail.service_uuid = '0x00112233445566';
    detail.watering_state_uuid = '0x00112233445566';
})(detail || (detail = {}));
export class WateringProgram {
    enabled = false;
    duration = 0;
    period = 0;
    waterFlow = 0;
    startDate = 0;
}
export class TinyDropDevice {
    deviceName = '';
    deviceId = '';
    watering = false;
    statusBar;
    constructor(statusBar, name, id) {
        this.statusBar = statusBar;
        this.deviceName = name;
        this.deviceId = id;
    }
    name() {
        return this.deviceName;
    }
    id() {
        return this.deviceId;
    }
    connect(onSuccess, onFailure) {
        this.doConnect(() => {
            this.startNotifications();
            if (onSuccess !== undefined) {
                onSuccess();
            }
            this.statusBar.setBluetoothState(true);
            this.statusBar.setWateringState(this.wateringState());
        }, () => {
            if (onFailure !== undefined) {
                onFailure();
            }
            this.statusBar.setBluetoothState(false);
            this.statusBar.setWateringState(false);
        });
    }
    disconnect(onSuccess, onFailure) {
        this.stopNotifications();
        this.doDisconnect(() => {
            if (onSuccess !== undefined) {
                onSuccess();
            }
            this.statusBar.setBluetoothState(false);
            this.statusBar.setWateringState(false);
        }, () => {
            if (onFailure !== undefined) {
                onFailure();
            }
        });
    }
    wateringState() {
        return this.watering;
    }
    onNotification(type, data) {
        switch (type) {
            case detail.NotificationType.WateringState:
                if (data.byteLength != 1) {
                    this.log("incorrect watering state data length");
                    return;
                }
                const wateringData = new Uint8Array(data);
                this.watering = wateringData[0] == 1;
                this.statusBar.setWateringState(this.watering);
                break;
        }
    }
    log(value) {
        console.log(`[Device] ${value}`);
    }
}
export class TinyDropBLEDevice extends TinyDropDevice {
    doConnect(onSuccess, onFailure) {
        ble.connect(this.id(), function (services) {
            this.log("connected");
            this.log(services);
            if (onSuccess !== undefined) {
                onSuccess();
            }
        }, function (error) {
            this.log("disconnected");
            this.log(`${error.id} - ${error.name}: ${error.errorMessage}`);
            if (onFailure !== undefined) {
                onFailure();
            }
        });
    }
    doDisconnect(onSuccess, onFailure) {
        ble.disconnect(this.id(), () => {
            onSuccess();
        }, (error) => {
            this.log(error);
            onFailure();
        });
    }
    startNotifications() {
        ble.startNotification(this.id(), detail.service_uuid, detail.watering_state_uuid, (data) => {
            this.onNotification(detail.NotificationType.WateringState, data);
        }, (error) => {
            this.log("watering start notification error: " + error);
        });
    }
    stopNotifications() {
        ble.stopNotification(this.id(), detail.service_uuid, detail.watering_state_uuid, null, (error) => {
            this.log("watering stop notification error: " + error);
        });
    }
    startWatering(durationSec, flowSpeed) {
    }
    stopWatering() {
    }
    sendProgram(program) {
    }
    log(value) {
        console.log(`[BLEDevice] ${value}`);
    }
}
export class TinyDropFakeDevice extends TinyDropDevice {
    successRate = 0.5;
    deviceDelayMs = 500;
    notificationsStarted = false;
    stopTimeoutHandle = -1;
    wateringTimeoutHandle = -1;
    wateringIntervalHandle = -1;
    internalWateringState = false;
    doConnect(onSuccess, onFailure) {
        setTimeout(() => {
            if (Math.random() < this.successRate) {
                this.log("connected");
                if (onSuccess !== undefined) {
                    onSuccess();
                }
                let data = new Uint8Array(1);
                data[0] = this.internalWateringState ? 1 : 0;
                this.onNotification(detail.NotificationType.WateringState, data.buffer);
            }
            else {
                this.log("disconnected");
                if (onFailure !== undefined) {
                    onFailure();
                }
            }
        }, 1000);
    }
    doDisconnect(onSuccess, onFailure) {
        if (onSuccess !== undefined) {
            onSuccess();
        }
    }
    startNotifications() {
        this.notificationsStarted = true;
    }
    stopNotifications() {
        this.notificationsStarted = false;
    }
    startWatering(durationSec, flowSpeed) {
        if (this.wateringState()) {
            return;
        }
        setTimeout(() => {
            if (this.notificationsStarted) {
                let data = new Uint8Array(1);
                data[0] = 1;
                this.onNotification(detail.NotificationType.WateringState, data.buffer);
            }
            this.internalWateringState = true;
        }, this.deviceDelayMs);
        clearTimeout(this.stopTimeoutHandle);
        this.stopTimeoutHandle = setTimeout(() => {
            if (this.notificationsStarted) {
                let data = new Uint8Array(1);
                data[0] = 0;
                this.onNotification(detail.NotificationType.WateringState, data.buffer);
            }
            this.internalWateringState = false;
        }, durationSec * 1000 + this.deviceDelayMs);
    }
    stopWatering() {
        if (!this.wateringState()) {
            return;
        }
        clearTimeout(this.stopTimeoutHandle);
        setTimeout(() => {
            if (this.notificationsStarted) {
                let data = new Uint8Array(1);
                data[0] = 0;
                this.onNotification(detail.NotificationType.WateringState, data.buffer);
            }
            this.internalWateringState = false;
        }, this.deviceDelayMs);
    }
    sendProgram(program) {
        clearInterval(this.wateringIntervalHandle);
        clearTimeout(this.wateringTimeoutHandle);
        if (program.enabled) {
            this.wateringTimeoutHandle = setTimeout(() => {
                this.startWatering(program.duration, program.waterFlow);
                this.wateringIntervalHandle = setInterval(() => {
                    this.startWatering(program.duration, program.waterFlow);
                }, program.period * 1000);
            }, this.computeWateringStart(program) * 1000);
        }
    }
    computeWateringStart(program) {
        const now = Date.now() / 1000;
        if (program.period == 0) {
            return Infinity;
        }
        else if (program.startDate >= now) {
            // startDate in the future
            return program.startDate;
        }
        else {
            // startDate in the past
            const cycles = (now - program.startDate) / program.period;
            return program.startDate + cycles * program.period;
        }
    }
    log(value) {
        console.log(`[FakeDevice] ${value}`);
    }
}
