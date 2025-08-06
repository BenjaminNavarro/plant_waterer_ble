import { qs, qsa, interpolate } from './utils.js'
import { ProgressUI } from './progress_ui.js'
import { TinyDropScanner } from './tinydrop_scanner.js'
import { TinyDropDevice } from './tinydrop_device.js'

export class ConnectPanel {
    searchButton = qs('#search_button')
    searchDurationMs = 5000

    deviceList = qs('#device_list')
    deviceListEntryTemplate = qs('#device_list_entry_template')

    constructor(progressUI: ProgressUI, scanner: TinyDropScanner, onConnect: CallableFunction, onDisconnect: CallableFunction) {
        this.searchButton.addEventListener('click', () => {
            this.deviceList.innerHTML = ''

            progressUI.startAutoUpdate(this.searchDurationMs)

            let index = 0
            scanner.stop()
            scanner.scan((device: TinyDropDevice) => {
                let name = device.name
                let id = device.id
                const data = { index, name, id }
                const entryHTML = interpolate(this.deviceListEntryTemplate.innerHTML.toString().trim(), data)
                this.deviceList.innerHTML += entryHTML

                const connectButtons = qsa<HTMLButtonElement>('.connect-button')

                connectButtons.forEach(btn => {
                    btn.setAttribute('connected', 'false')
                    btn.addEventListener('click', () => {
                        if (btn.getAttribute('connected') == 'false') {
                            progressUI.spin()
                            device.connect(
                                () => {
                                    btn.setAttribute('connected', 'true')
                                    btn.innerHTML = 'Déconnexion'
                                    progressUI.hide()
                                    connectButtons.forEach(other_btn => {
                                        if (other_btn != btn) {
                                            other_btn.disabled = true
                                        }
                                    })

                                    onConnect(device)
                                },
                                () => {
                                    progressUI.hide()
                                })
                        }
                        else {
                            device.disconnect(() => {
                                btn.setAttribute('connected', 'false')
                                btn.innerHTML = 'Connexion'
                                connectButtons.forEach(other_btn => {
                                    if (other_btn != btn) {
                                        other_btn.disabled = false
                                    }
                                })

                                onDisconnect()
                            })
                        }
                    })
                })
            },
                (error: string) => {
                    progressUI.hide()
                    console.log(error)

                },
                this.searchDurationMs)

        })
    }
}