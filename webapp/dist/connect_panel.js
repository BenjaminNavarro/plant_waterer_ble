import { qs, qsa, interpolate } from './utils.js';
export class ConnectPanel {
    searchButton = qs('#search_button');
    searchDurationMs = 5000;
    deviceList = qs('#device_list');
    deviceListEntryTemplate = qs('#device_list_entry_template');
    constructor(progressUI, scanner, onConnect, onDisconnect) {
        this.searchButton.addEventListener('click', () => {
            this.deviceList.innerHTML = '';
            progressUI.startAutoUpdate(this.searchDurationMs);
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
                    btn.setAttribute('connected', 'false');
                    btn.addEventListener('click', () => {
                        if (btn.getAttribute('connected') == 'false') {
                            progressUI.spin();
                            device.connect(() => {
                                btn.setAttribute('connected', 'true');
                                btn.innerHTML = 'Déconnexion';
                                progressUI.hide();
                                connectButtons.forEach(other_btn => {
                                    if (other_btn != btn) {
                                        other_btn.disabled = true;
                                    }
                                });
                                onConnect(device);
                            }, () => {
                                progressUI.hide();
                            });
                        }
                        else {
                            device.disconnect(() => {
                                btn.setAttribute('connected', 'false');
                                btn.innerHTML = 'Connexion';
                                connectButtons.forEach(other_btn => {
                                    if (other_btn != btn) {
                                        other_btn.disabled = false;
                                    }
                                });
                                onDisconnect();
                            });
                        }
                    });
                });
            }, (error) => {
                progressUI.hide();
                console.log(error);
            }, this.searchDurationMs);
        });
    }
}
