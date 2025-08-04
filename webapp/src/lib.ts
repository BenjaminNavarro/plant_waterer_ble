import { SlTabGroup } from '@shoelace-style/shoelace'

function qs<Type = HTMLElement>(query: string): Type {
    return document.querySelector(query) as Type
}

function qsa<Type extends HTMLElement = HTMLElement>(query: string): NodeListOf<Type> {
    return document.querySelectorAll(query)
}

function interpolate(template: string, params: Object) {
    const replaceTags: Map<string, string> = new Map([
        ['&', '& amp;'], ['<', '& lt;'], ['>', '& gt;'], ['(', '% 28'], [')', '% 29']])

    const safeInnerHTML = text => text.toString()
        .replace(/[&<>()]/g, tag => replaceTags.get(tag) || tag)
    const keys = Object.keys(params)
    const keyVals = Object.values(params).map(safeInnerHTML)
    /* eslint no-new-func: "off" */
    return new Function(...keys, `return \`${template}\``)(...keyVals)
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
        if (!this.progressRing.classList.contains("is_hidden")) {
            this.progressRing.classList.add("is_hidden")
        }
    }

    show() {
        if (this.progressRing.classList.contains("is_hidden")) {
            this.progressRing.classList.remove("is_hidden")
        }
    }

    startAutoUpdate(durationMs: number) {
        let searchProgress = 0;
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
        this.autoUpdateInterval = -1
    }

    updateRateMs = 100

    private progressRing: HTMLInputElement
    private autoUpdateInterval: number = -1
}

class ProgressSpinner {
    constructor(id: string) {
        this.progressSpinner = qs<HTMLInputElement>(id)
        this.hide()
    }


    hide() {
        if (!this.progressSpinner.classList.contains("is_hidden")) {
            this.progressSpinner.classList.add("is_hidden")
        }
    }

    show() {
        if (this.progressSpinner.classList.contains("is_hidden")) {
            this.progressSpinner.classList.remove("is_hidden")
        }
    }

    private progressSpinner: HTMLInputElement

}

export function testFromTS() {
    const progressRing = new ProgressRing('#progress_ring')
    const progressSpinner = new ProgressSpinner('#progress_spinner')

    const tabs = qs('#tabs') as SlTabGroup

    const searchButton = qs('#search_button')
    const searchDurationMs = 5000

    const deviceList = qs('#device_list')
    const deviceListEntryTemplate = qs('#device_list_entry_template')
    searchButton.addEventListener('click', () => {
        progressRing.startAutoUpdate(searchDurationMs)

        for (let index = 0; index < 3; index++) {
            const name = "Device #" + (index + 1).toString()
            const id = index
            const data = { index, name, id }
            const entryHTML = interpolate(deviceListEntryTemplate.innerHTML.toString().trim(), data)
            deviceList.innerHTML += entryHTML
        }

        qsa<HTMLButtonElement>('.connect-button').forEach(btn => {
            btn.addEventListener('click', () => {
                progressRing.hide()
                progressSpinner.show()
                setTimeout(() => {
                    progressSpinner.hide()
                    tabs.show('program')
                }, 1000)
            })
        })
    })
}