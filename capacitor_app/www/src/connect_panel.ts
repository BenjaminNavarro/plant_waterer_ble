import { qs, qsa, interpolate } from './utils.ts'
import { ProgressUI, ResultType } from './progress_ui.ts'
import { TinyDropScanner } from './tinydrop_scanner.ts'
import { TinyDropDevice } from './tinydrop_device.ts'
import type { SlTabShowEvent } from '@shoelace-style/shoelace'

export class ConnectPanel {
    connectedDevice: TinyDropDevice = null

    private searchDurationMs = 30000
    private searchButton = qs('#search_button')
    private deviceList = qs('#device_list')
    private deviceListEntryTemplate = qs('#device_list_entry_template')

    constructor(progressUI: ProgressUI, scanner: TinyDropScanner, onConnect: CallableFunction, onDisconnect: CallableFunction) {
        this.searchButton.addEventListener('click', () => {
            this.deviceList.innerHTML = ''

            progressUI.startAutoUpdate(this.searchDurationMs)

            let connecting = false

            let index = 0
            scanner.stop()
            scanner.scan((device: TinyDropDevice) => {
                let name = device.name()
                let id = device.id()
                const data = { index, name, id }
                const entryHTML = interpolate(this.deviceListEntryTemplate.innerHTML.toString().trim(), data)
                this.deviceList.innerHTML += entryHTML

                const connectButtons = qsa<HTMLButtonElement>('.connect-button')

                connectButtons.forEach(btn => {
                    if (connecting) {
                        btn.disabled = true
                    }
                    btn.setAttribute('connected', 'false')
                    btn.addEventListener('click', () => {
                        if (btn.getAttribute('connected') == 'false') {
                            progressUI.spin()
                            connectButtons.forEach(other_btn => {
                                other_btn.disabled = true
                            })
                            scanner.stop()
                            device.connect(
                                () => {
                                    progressUI.stop(ResultType.Success)

                                    btn.setAttribute('connected', 'true')
                                    btn.innerHTML = 'Déconnexion'
                                    btn.disabled = false

                                    this.connectedDevice = device

                                    onConnect(device)
                                    connecting = false

                                    console.log('Device connected');
                                },
                                () => {
                                    progressUI.stop(ResultType.Error)

                                    this.connectedDevice = null

                                    connectButtons.forEach(other_btn => {
                                        other_btn.disabled = false
                                    })
                                    connecting = false
                                    console.log('failed to connect');
                                },
                                () => {
                                    this.connectedDevice = null

                                    connectButtons.forEach(other_btn => {
                                        other_btn.disabled = false
                                    })

                                    btn.setAttribute('connected', 'false')
                                    btn.innerHTML = 'Connexion'
                                    connecting = false
                                    onDisconnect()
                                })
                            connecting = true
                        }
                        else {
                            progressUI.spin()

                            device.disconnect(() => {
                                progressUI.stop(ResultType.Success)

                                btn.setAttribute('connected', 'false')
                                btn.innerHTML = 'Connexion'
                                connectButtons.forEach(other_btn => {
                                    if (other_btn != btn) {
                                        other_btn.disabled = false
                                    }
                                })

                                onDisconnect()
                            }, () => {
                                progressUI.stop(ResultType.Error)
                            })
                        }
                    })
                })
            },
                (error: string) => {
                    progressUI.stop(ResultType.Error)
                    console.log(error)

                },
                this.searchDurationMs)

        })
    }

    disconnect(onDisconnect?: CallableFunction) {
        if (this.connectedDevice) {
            this.connectedDevice.disconnect(() => {
                this.connectedDevice = null

                const connectButtons = qsa<HTMLButtonElement>('.connect-button')
                connectButtons.forEach(btn => {
                    btn.disabled = false
                    btn.setAttribute('connected', 'false')
                    btn.innerHTML = 'Connexion'
                })

                if (onDisconnect) {
                    onDisconnect()
                }
            })
        }
    }
}