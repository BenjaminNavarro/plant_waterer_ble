import { SlAlert, SlIcon } from "shoelace/shoelace";
import { qs } from "./utils.js";

export enum AlertType {
    Info,
    Success,
    Warning,
    Error
}

export enum AlertDuration {
    Short = 3000,
    Default = 5000,
    Long = 10000,
    Infinite = Infinity
}

export class Alert {
    // private alertBox = qs<SlAlert>('#alert_box')
    private alertBoxElem = qs('#alert_box')
    private alertText = qs('#alert_text')
    private alertIcon = qs<SlIcon>('#alert_icon')
    private alertIconElem = qs('#alert_icon')
    private currentTimeoutId = -1

    // TODO icon color = border color


    show(text: string, type: AlertType, durationMs: number) {
        let setColor = (color: string) => {
            this.alertBoxElem.style.borderColor = `var(--sl-color-${color}-600)`
            this.alertBoxElem.setAttribute('variant', color)
            this.alertIconElem.style.color = `var(--sl-color-${color}-600)`
        }

        this.alertText.innerHTML = text
        // this.alertBox.duration = durationMs
        switch (type) {
            case AlertType.Info:
                this.alertIcon.name = 'info-circle'
                setColor('primary')
                break
            case AlertType.Success:
                setColor('success')
                this.alertIcon.name = 'check2-circle'
                break
            case AlertType.Warning:
                setColor('warning')
                this.alertIcon.name = 'exclamation-triangle'
                break
            case AlertType.Error:
                setColor('danger')
                this.alertIcon.name = 'exclamation-octagon'
                break
        }
        // this.alertBox.toast()
        // this.alertBox.show()
        this.alertBoxElem.style.display = 'block'
        // TODO kill previous timeout
        if (durationMs != AlertDuration.Infinite) {
            if (this.currentTimeoutId != -1) {
                clearTimeout(this.currentTimeoutId)
            }
            this.currentTimeoutId = setTimeout(() => {
                this.alertBoxElem.style.display = 'none'
            }, durationMs)
        }
    }
}