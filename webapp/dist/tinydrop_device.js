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
export class WateringStateData {
    watering = false;
    duration = 0;
    startDate = 0;
    static expectedBufferSize = 1 + 4 + 4;
    static fromBuffer(buffer) {
        let data = new WateringStateData();
        let view = new DataView(buffer);
        data.watering = view.getUint8(0) == 1;
        data.duration = view.getUint32(0 + 1);
        data.startDate = view.getUint32(0 + 1 + 4);
        return data;
    }
    toBuffer() {
        let buffer = new ArrayBuffer(WateringStateData.expectedBufferSize);
        let view = new DataView(buffer);
        view.setUint8(0, this.watering ? 1 : 0);
        view.setUint32(0 + 1, this.duration);
        view.setUint32(0 + 1 + 4, this.startDate);
        return buffer;
    }
}
export class TinyDropDevice {
    connected = false;
    deviceName = '';
    deviceId = '';
    wateringStateData = new WateringStateData();
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
            this.connected = true;
            this.updateStatusBar();
        }, () => {
            if (onFailure !== undefined) {
                onFailure();
            }
            this.connected = false;
            this.updateStatusBar();
        });
    }
    disconnect(onSuccess, onFailure) {
        this.stopNotifications();
        this.doDisconnect(() => {
            if (onSuccess !== undefined) {
                onSuccess();
            }
            this.connected = false;
            this.updateStatusBar();
        }, () => {
            if (onFailure !== undefined) {
                onFailure();
            }
            this.connected = false;
            this.updateStatusBar();
        });
    }
    wateringState() {
        return this.wateringStateData;
    }
    onNotification(type, data) {
        switch (type) {
            case detail.NotificationType.WateringState:
                if (data.byteLength != WateringStateData.expectedBufferSize) {
                    this.log(`incorrect watering state data length: got ${data.byteLength}, expected ${WateringStateData.expectedBufferSize}`);
                    return;
                }
                this.wateringStateData = WateringStateData.fromBuffer(data);
                this.updateStatusBar();
                break;
        }
    }
    log(value) {
        console.log(`[Device] ${value}`);
    }
    updateStatusBar() {
        this.statusBar.setBluetoothState(this.connected);
        this.statusBar.setWateringState(this.wateringState().watering);
        const nowSec = Date.now() / 1000;
        if (this.wateringStateData.duration > 0) {
            this.statusBar.startWateringProgressAutoUpdate(this.wateringStateData.duration, ((nowSec - this.wateringStateData.startDate) /
                this.wateringStateData.duration) * 100);
        }
        else {
            this.statusBar.stopWateringProgressAutoUpdate();
            this.statusBar.setWateringProgress(0);
        }
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
    internalWateringState = new WateringStateData();
    constructor(statusBar, name, id) {
        super(statusBar, name, id);
        this.internalWateringState.startDate = Date.now() / 1000;
    }
    doConnect(onSuccess, onFailure) {
        setTimeout(() => {
            if (Math.random() < this.successRate) {
                this.log("connected");
                if (onSuccess !== undefined) {
                    onSuccess();
                }
                this.onNotification(detail.NotificationType.WateringState, this.internalWateringState.toBuffer());
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
        if (this.wateringState().watering) {
            return;
        }
        this.sendWateringNotification(false);
        setTimeout(() => {
            this.internalWateringState.startDate = Date.now() / 1000;
            this.sendWateringNotification(true, durationSec);
        }, this.deviceDelayMs);
        clearTimeout(this.stopTimeoutHandle);
        this.stopTimeoutHandle = setTimeout(() => {
            this.sendWateringNotification(false);
        }, durationSec * 1000 + this.deviceDelayMs);
    }
    stopWatering() {
        if (!this.wateringState().watering) {
            return;
        }
        clearTimeout(this.stopTimeoutHandle);
        setTimeout(() => {
            this.sendWateringNotification(false);
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
    sendWateringNotification(state, duration = 0) {
        this.internalWateringState.watering = state;
        this.internalWateringState.duration = state ? duration : 0;
        this.onNotification(detail.NotificationType.WateringState, this.internalWateringState.toBuffer());
    }
    log(value) {
        console.log(`[FakeDevice] ${value}`);
    }
}
