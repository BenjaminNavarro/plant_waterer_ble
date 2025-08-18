import { convertDuration, convertDurationToBestUnit, qs } from './utils.js'
import { ProgressUI } from './progress_ui.js'
import { TinyDropDevice, WateringProgram } from './tinydrop_device.js'
import { SlSwitch } from '@shoelace-style/shoelace'
import type { SlChangeEvent } from '@shoelace-style/shoelace'

export class ConfigPanel {

    private programEnabled = qs<SlSwitch>('#program_enabled')
    private programEnabledElem = qs<HTMLInputElement>('#program_enabled')
    private programDuration = qs<HTMLInputElement>('#program_duration')
    private programDurationTimeType = qs<HTMLInputElement>('#program_duration_time_type')
    private programPeriod = qs<HTMLInputElement>('#program_period')
    private programPeriodTimeType = qs<HTMLInputElement>('#program_period_time_type')
    private programWaterFlow = qs<HTMLInputElement>('#program_flow_speed')
    private programStart = qs<HTMLInputElement>('#program_start')
    private programTestButton = qs<HTMLButtonElement>('#program_test_button')
    private programApplyButton = qs<HTMLButtonElement>('#program_apply_button')
    private programStopButton = qs<HTMLButtonElement>('#program_stop_button')

    private device: TinyDropDevice
    private progressUI: ProgressUI
    private program = new WateringProgram()

    constructor(progressUI: ProgressUI) {
        this.progressUI = progressUI

        let log = (value: any) => {
            console.log(`[Program] ${value}`)
        }

        this.programEnabledElem.addEventListener('sl-change', (ev: SlChangeEvent) => {
            this.program.enabled = this.programEnabled.checked
        })

        this.programDuration.addEventListener('sl-change', () => {
            let value = Number(this.programDuration.value)
            value = Math.max(value, 0)
            this.program.duration = convertDuration(value, this.programDurationTimeType.value)
            this.programDuration.value = value.toString()
        })

        this.programDurationTimeType.addEventListener('sl-change', () => {
            this.program.duration = convertDuration(Math.max(Number(this.programDuration.value), 0), this.programDurationTimeType.value)
        })

        this.programPeriod.addEventListener('sl-change', () => {
            let value = Number(this.programPeriod.value)
            value = Math.max(value, 0)
            this.program.period = convertDuration(value, this.programPeriodTimeType.value)
            this.programPeriod.value = value.toString()
        })

        this.programPeriodTimeType.addEventListener('sl-change', () => {
            this.program.period = convertDuration(Math.max(Number(this.programPeriod.value), 0), this.programPeriodTimeType.value)
        })

        this.programWaterFlow.addEventListener('sl-change', () => {
            let value = Number(this.programWaterFlow.value)
            this.program.waterFlow = Math.min(Math.max(value, 0), 100)
            this.programWaterFlow.value = this.program.waterFlow.toString()
        })

        this.programStart.addEventListener('sl-change', () => {
            this.program.startDate = Date.parse(this.programStart.value) / 1000

        })

        this.programTestButton.addEventListener('click', () => {
            this.startWatering()
        })

        this.programApplyButton.addEventListener('click', () => {
            this.sendProgram()
        })

        this.programStopButton.addEventListener('click', () => {
            this.stopWatering()
        })
    }

    setDevice(device: TinyDropDevice) {
        this.device = device

        if (this.device != null) {
            let savedProgram = this.device.readProgram()
            if (savedProgram != null) {
                this.program = savedProgram

                this.program.enabled = this.program.enabled
                this.programEnabled.checked = this.program.enabled

                const durationConv = convertDurationToBestUnit(this.program.duration)
                this.programDuration.value = durationConv.duration.toString()
                this.programDurationTimeType.value = durationConv.unit

                const periodConv = convertDurationToBestUnit(this.program.period)
                this.programPeriod.value = periodConv.duration.toString()
                this.programPeriodTimeType.value = periodConv.unit

                this.programWaterFlow.value = this.program.waterFlow.toString()

                const date = new Date(this.program.startDate * 1000)

                const year = date.getFullYear()
                const month = date.getMonth() + 1
                const monthPrefix = month < 10 ? '0' : ''
                const day = date.getDate()
                const hours = date.getHours()
                const minutes = date.getMinutes()
                const formattedDate = `${year}-${monthPrefix}${month}-${day}T${hours}:${minutes}`

                this.programStart.value = formattedDate
            }
        }

    }

    private startWatering() {
        if (this.device != null && !this.device.wateringState().watering) {
            this.device.startWatering(this.program.duration, this.program.waterFlow)
        }
    }

    private stopWatering() {
        if (this.device != null && this.device.wateringState().watering) {
            this.device.stopWatering()
        }
    }

    private sendProgram() {
        this.device.sendProgram(this.program)
    }
}
