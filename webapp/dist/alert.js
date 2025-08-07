import { qs } from "./utils.js";
export var AlertType;
(function (AlertType) {
    AlertType[AlertType["Info"] = 0] = "Info";
    AlertType[AlertType["Success"] = 1] = "Success";
    AlertType[AlertType["Warning"] = 2] = "Warning";
    AlertType[AlertType["Error"] = 3] = "Error";
})(AlertType || (AlertType = {}));
export var AlertDuration;
(function (AlertDuration) {
    AlertDuration[AlertDuration["Short"] = 3000] = "Short";
    AlertDuration[AlertDuration["Default"] = 5000] = "Default";
    AlertDuration[AlertDuration["Long"] = 10000] = "Long";
    AlertDuration[AlertDuration["Infinite"] = Infinity] = "Infinite";
})(AlertDuration || (AlertDuration = {}));
export class Alert {
    // private alertBox = qs<SlAlert>('#alert_box')
    alertBoxElem = qs('#alert_box');
    alertText = qs('#alert_text');
    alertIcon = qs('#alert_icon');
    alertIconElem = qs('#alert_icon');
    currentTimeoutId = -1;
    // TODO icon color = border color
    show(text, type, durationMs) {
        let setColor = (color) => {
            this.alertBoxElem.style.borderColor = `var(--sl-color-${color}-600)`;
            this.alertBoxElem.setAttribute('variant', color);
            this.alertIconElem.style.color = `var(--sl-color-${color}-600)`;
        };
        this.alertText.innerHTML = text;
        // this.alertBox.duration = durationMs
        switch (type) {
            case AlertType.Info:
                this.alertIcon.name = 'info-circle';
                setColor('primary');
                break;
            case AlertType.Success:
                setColor('success');
                this.alertIcon.name = 'check2-circle';
                break;
            case AlertType.Warning:
                setColor('warning');
                this.alertIcon.name = 'exclamation-triangle';
                break;
            case AlertType.Error:
                setColor('danger');
                this.alertIcon.name = 'exclamation-octagon';
                break;
        }
        // this.alertBox.toast()
        // this.alertBox.show()
        this.alertBoxElem.style.display = 'block';
        // TODO kill previous timeout
        if (durationMs != AlertDuration.Infinite) {
            if (this.currentTimeoutId != -1) {
                clearTimeout(this.currentTimeoutId);
            }
            this.currentTimeoutId = setTimeout(() => {
                this.alertBoxElem.style.display = 'none';
            }, durationMs);
        }
    }
}
