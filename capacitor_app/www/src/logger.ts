import { interpolate, qs } from "./utils.ts";

export class Logger {
    constructor() {
        this.logs.innerHTML = ''

        this.logsTable.classList.add('is_hidden')
        qs('#logs_button').addEventListener('click', () => {
            this.logsTable.classList.toggle('is_hidden')
        })
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

    private logsTable = qs('#logs_table')
    private logs = qs('#logs')
    private logEntryTemplate = qs('#log_entry')
}

export let logger = new Logger()