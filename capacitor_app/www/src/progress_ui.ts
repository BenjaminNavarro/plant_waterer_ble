import { SlIcon } from "@shoelace-style/shoelace"
import { qs } from "./utils.ts"
import type { timeoutId } from "./utils.ts"

export class ProgressUI {
    stop(result: ResultType) {
        this.progressRing.hide()
        this.progressSpinner.hide()
        this.progressResult.show(result)
    }


    startAutoUpdate(durationMs: number) {
        this.progressSpinner.hide()
        this.progressRing.show()
        this.progressRing.startAutoUpdate(durationMs)
        this.hideResult()
    }

    spin() {
        this.progressRing.hide()
        this.progressSpinner.show()
        this.hideResult()
    }

    private hideResult() {
        this.progressResult.hide()
    }

    private progressRing = new ProgressRing('#progress_ring')
    private progressSpinner = new ProgressSpinner('#progress_spinner')
    private progressResult = new ProgressResult('#progress_result')
}


class ProgressRing {
    constructor(id: string) {
        this.progressRing = qs<HTMLInputElement>(id)
        this.hide()
    }

    setValue(value: number) {
        this.progressRing.value = value.toString()
    }

    getValue(): number {
        return Number(this.progressRing.value)
    }

    hide() {
        this.progressRing.style.display = 'none'

    }

    show() {
        this.progressRing.style.display = 'block'
    }

    startAutoUpdate(durationMs: number) {
        let searchProgress = 0;
        this.stopAutoUpdate()
        this.autoUpdateInterval = setInterval(() => {
            this.progressRing.value = searchProgress.toString();

            searchProgress += 100 / (durationMs / this.updateRateMs);
            if (searchProgress >= 100) {
                searchProgress = 100
                this.stopAutoUpdate()
                this.setValue(0)
                this.hide()
            }
        }, this.updateRateMs)


        this.setValue(0)
        this.show()
    }

    stopAutoUpdate() {
        clearInterval(this.autoUpdateInterval)
    }

    updateRateMs = 100

    private progressRing: HTMLInputElement
    private autoUpdateInterval: timeoutId = null
}

class ProgressSpinner {
    constructor(id: string) {
        this.progressSpinner = qs<HTMLInputElement>(id)
        this.hide()
    }


    hide() {
        this.progressSpinner.style.display = 'none'
    }

    show() {
        this.progressSpinner.style.display = 'block'
    }

    private progressSpinner: HTMLInputElement

}

export enum ResultType {
    None,
    Success,
    Warning,
    Error
}

export enum ResultDuration {
    Default = 2000,
    Medium = 5000,
    Long = 10000,
    Infinite = Infinity
}

class ProgressResult {
    constructor(id: string) {
        this.progressResult = qs<SlIcon>(id)
        this.progressResultElem = qs(id)
        this.hide()
    }

    hide() {
        this.progressResultElem.style.display = 'none'
    }

    show(type: ResultType) {
        if (this.progressResultElem.style.display == 'none') {
            this.progressResultElem.style.display = 'block'
            let durationMs = ResultDuration.Infinite
            switch (type) {
                case ResultType.None:
                    this.progressResult.name = ''
                    this.progressResultElem.style.color = 'var(--sl-color-primary-600)'
                    break
                case ResultType.Success:
                    this.progressResult.name = 'check2-circle'
                    this.progressResultElem.style.color = 'var(--sl-color-success-600)'
                    durationMs = ResultDuration.Default
                    break
                case ResultType.Warning:
                    this.progressResult.name = 'exclamation-triangle'
                    this.progressResultElem.style.color = 'var(--sl-color-warning-600)'
                    durationMs = ResultDuration.Medium
                    break
                case ResultType.Error:
                    this.progressResult.name = 'exclamation-octagon'
                    this.progressResultElem.style.color = 'var(--sl-color-danger-600)'
                    durationMs = ResultDuration.Long
                    break
            }

            if (durationMs != ResultDuration.Infinite) {
                clearTimeout(this.currentTimeoutId)
                this.currentTimeoutId = setTimeout(() => {
                    this.hide()
                }, durationMs)
            }
        }
    }

    private progressResult = qs<SlIcon>('#progress_result')
    private progressResultElem = qs('#progress_result')
    private currentTimeoutId: timeoutId = null
}