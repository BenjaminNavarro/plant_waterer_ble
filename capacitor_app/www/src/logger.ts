import { interpolate, qs } from "./utils.ts";

export class Logger {
    constructor() {
        this.logs.innerHTML = ''
    }

    log(data: any) {
        const date = new Date()
        const timestamp = date.toLocaleDateString() + ' ' + date.toLocaleTimeString()
        const message = data.toString()
        const template_data = { message, timestamp }
        const entryHTML = interpolate(this.logEntryTemplate.innerHTML.toString().trim(), template_data)
        this.logs.innerHTML = entryHTML + this.logs.innerHTML

        console.log(data)
    }

    private logs = qs('#logs')
    private logEntryTemplate = qs('#log_entry')
}

export let logger = new Logger()