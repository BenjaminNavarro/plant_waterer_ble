import { qs } from "./utils.js";
export class ProgressUI {
    stop(result) {
        this.progressRing.hide();
        this.progressSpinner.hide();
        this.progressResult.show(result);
    }
    startAutoUpdate(durationMs) {
        this.progressSpinner.hide();
        this.progressRing.show();
        this.progressRing.startAutoUpdate(durationMs);
        this.hideResult();
    }
    spin() {
        this.progressRing.hide();
        this.progressSpinner.show();
        this.hideResult();
    }
    hideResult() {
        this.progressResult.hide();
    }
    progressRing = new ProgressRing('#progress_ring');
    progressSpinner = new ProgressSpinner('#progress_spinner');
    progressResult = new ProgressResult('#progress_result');
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
export var ResultType;
(function (ResultType) {
    ResultType[ResultType["None"] = 0] = "None";
    ResultType[ResultType["Success"] = 1] = "Success";
    ResultType[ResultType["Warning"] = 2] = "Warning";
    ResultType[ResultType["Error"] = 3] = "Error";
})(ResultType || (ResultType = {}));
export var ResultDuration;
(function (ResultDuration) {
    ResultDuration[ResultDuration["Default"] = 2000] = "Default";
    ResultDuration[ResultDuration["Medium"] = 5000] = "Medium";
    ResultDuration[ResultDuration["Long"] = 10000] = "Long";
    ResultDuration[ResultDuration["Infinite"] = Infinity] = "Infinite";
})(ResultDuration || (ResultDuration = {}));
class ProgressResult {
    constructor(id) {
        this.progressResult = qs(id);
        this.progressResultElem = qs(id);
        this.hide();
    }
    hide() {
        if (!this.progressResultElem.classList.contains("is_hidden")) {
            this.progressResultElem.classList.add("is_hidden");
        }
    }
    show(type) {
        if (this.progressResultElem.classList.contains("is_hidden")) {
            this.progressResultElem.classList.remove("is_hidden");
            let durationMs = ResultDuration.Infinite;
            switch (type) {
                case ResultType.None:
                    this.progressResult.name = '';
                    this.progressResultElem.style.color = 'var(--sl-color-primary-600)';
                    break;
                case ResultType.Success:
                    this.progressResult.name = 'check2-circle';
                    this.progressResultElem.style.color = 'var(--sl-color-success-600)';
                    durationMs = ResultDuration.Default;
                    break;
                case ResultType.Warning:
                    this.progressResult.name = 'exclamation-triangle';
                    this.progressResultElem.style.color = 'var(--sl-color-warning-600)';
                    durationMs = ResultDuration.Medium;
                    break;
                case ResultType.Error:
                    this.progressResult.name = 'exclamation-octagon';
                    this.progressResultElem.style.color = 'var(--sl-color-danger-600)';
                    durationMs = ResultDuration.Long;
                    break;
            }
            if (durationMs != ResultDuration.Infinite) {
                if (this.currentTimeoutId != -1) {
                    clearTimeout(this.currentTimeoutId);
                }
                this.currentTimeoutId = setTimeout(() => {
                    this.hide();
                }, durationMs);
            }
        }
    }
    progressResult = qs('#progress_result');
    progressResultElem = qs('#progress_result');
    currentTimeoutId = -1;
}
