import { qs, qsa, interpolate } from './utils.js';
import { ResultType } from './progress_ui.js';
export class ConnectPanel {
    searchButton = qs('#search_button');
    searchDurationMs = 5000;
    deviceList = qs('#device_list');
    deviceListEntryTemplate = qs('#device_list_entry_template');
    constructor(progressUI, scanner, onConnect, onDisconnect) {
        this.searchButton.addEventListener('click', () => {
            this.deviceList.innerHTML = '';
            progressUI.startAutoUpdate(this.searchDurationMs);
            let connecting = false;
            let index = 0;
            scanner.stop();
            scanner.scan((device) => {
                let name = device.name;
                let id = device.id;
                const data = { index, name, id };
                const entryHTML = interpolate(this.deviceListEntryTemplate.innerHTML.toString().trim(), data);
                this.deviceList.innerHTML += entryHTML;
                const connectButtons = qsa('.connect-button');
                connectButtons.forEach(btn => {
                    if (connecting) {
                        btn.disabled = true;
                    }
                    btn.setAttribute('connected', 'false');
                    btn.addEventListener('click', () => {
                        if (btn.getAttribute('connected') == 'false') {
                            progressUI.spin();
                            connectButtons.forEach(other_btn => {
                                other_btn.disabled = true;
                            });
                            device.connect(() => {
                                btn.setAttribute('connected', 'true');
                                btn.innerHTML = 'Déconnexion';
                                btn.disabled = false;
                                progressUI.stop(ResultType.Success);
                                onConnect(device);
                                connecting = false;
                                console.log('Device connected');
                            }, () => {
                                progressUI.stop(ResultType.Error);
                                connectButtons.forEach(other_btn => {
                                    other_btn.disabled = false;
                                });
                                connecting = false;
                                console.log('failed to connect');
                            });
                            connecting = true;
                        }
                        else {
                            progressUI.spin();
                            device.disconnect(() => {
                                progressUI.stop(ResultType.Success);
                                btn.setAttribute('connected', 'false');
                                btn.innerHTML = 'Connexion';
                                connectButtons.forEach(other_btn => {
                                    if (other_btn != btn) {
                                        other_btn.disabled = false;
                                    }
                                });
                                onDisconnect();
                            }, () => {
                                progressUI.stop(ResultType.Error);
                            });
                        }
                    });
                });
            }, (error) => {
                progressUI.stop(ResultType.Error);
                console.log(error);
            }, this.searchDurationMs);
        });
    }
}
