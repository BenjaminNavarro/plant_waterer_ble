import { qs } from "./utils.js";
export class ProgressUI {
    hide() {
        this.progressRing.hide();
        this.progressSpinner.hide();
    }
    startAutoUpdate(durationMs) {
        this.progressSpinner.hide();
        this.progressRing.show();
        this.progressRing.startAutoUpdate(durationMs);
    }
    spin() {
        this.progressRing.hide();
        this.progressSpinner.show();
    }
    progressRing = new ProgressRing('#progress_ring');
    progressSpinner = new ProgressSpinner('#progress_spinner');
}
class ProgressRing {
    constructor(id) {
        this.progressRing = qs(id);
        this.hide();
    }
    setValue(value) {
        this.progressRing.value = value.toString();
    }
    getValue() {
        return Number(this.progressRing.value);
    }
    hide() {
        if (!this.progressRing.classList.contains("is_hidden")) {
            this.progressRing.classList.add("is_hidden");
        }
    }
    show() {
        if (this.progressRing.classList.contains("is_hidden")) {
            this.progressRing.classList.remove("is_hidden");
        }
    }
    startAutoUpdate(durationMs) {
        let searchProgress = 0;
        this.stopAutoUpdate();
        this.autoUpdateInterval = setInterval(() => {
            this.progressRing.value = searchProgress.toString();
            searchProgress += 100 / (durationMs / this.updateRateMs);
            if (searchProgress >= 100) {
                searchProgress = 100;
                this.stopAutoUpdate();
                this.setValue(0);
                this.hide();
            }
        }, this.updateRateMs);
        this.setValue(0);
        this.show();
    }
    stopAutoUpdate() {
        if (this.autoUpdateInterval != -1) {
            clearInterval(this.autoUpdateInterval);
            this.autoUpdateInterval = -1;
        }
    }
    updateRateMs = 100;
    progressRing;
    autoUpdateInterval = -1;
}
class ProgressSpinner {
    constructor(id) {
        this.progressSpinner = qs(id);
        this.hide();
    }
    hide() {
        if (!this.progressSpinner.classList.contains("is_hidden")) {
            this.progressSpinner.classList.add("is_hidden");
        }
    }
    show() {
        if (this.progressSpinner.classList.contains("is_hidden")) {
            this.progressSpinner.classList.remove("is_hidden");
        }
    }
    progressSpinner;
}
