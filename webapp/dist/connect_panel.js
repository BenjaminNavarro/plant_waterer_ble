import { qs, qsa, interpolate } from './utils.js';
import { AlertDuration, AlertType } from './alert.js';
export class ConnectPanel {
    searchButton = qs('#search_button');
    searchDurationMs = 5000;
    deviceList = qs('#device_list');
    deviceListEntryTemplate = qs('#device_list_entry_template');
    constructor(progressUI, scanner, alert, onConnect, onDisconnect) {
        this.searchButton.addEventListener('click', () => {
            alert.show('Recherche en cours', AlertType.Info, this.searchDurationMs);
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
                            alert.show('Connexion en cours', AlertType.Info, AlertDuration.Infinite);
                            progressUI.spin();
                            connectButtons.forEach(other_btn => {
                                other_btn.disabled = true;
                            });
                            device.connect(() => {
                                alert.show('Connecté', AlertType.Success, AlertDuration.Default);
                                btn.setAttribute('connected', 'true');
                                btn.innerHTML = 'Déconnexion';
                                btn.disabled = false;
                                progressUI.hide();
                                onConnect(device);
                                connecting = false;
                                console.log('Device connected');
                            }, () => {
                                alert.show('Connexion échouée', AlertType.Error, AlertDuration.Long);
                                progressUI.hide();
                                connectButtons.forEach(other_btn => {
                                    other_btn.disabled = false;
                                });
                                connecting = false;
                                console.log('failed to connect');
                            });
                            connecting = true;
                        }
                        else {
                            alert.show('Déconnexion en cours', AlertType.Info, AlertDuration.Infinite);
                            device.disconnect(() => {
                                alert.show('Déconnecté', AlertType.Success, AlertDuration.Default);
                                btn.setAttribute('connected', 'false');
                                btn.innerHTML = 'Connexion';
                                connectButtons.forEach(other_btn => {
                                    if (other_btn != btn) {
                                        other_btn.disabled = false;
                                    }
                                });
                                onDisconnect();
                            }, () => {
                                alert.show('Déconnexion échouée', AlertType.Error, AlertDuration.Long);
                            });
                        }
                    });
                });
            }, (error) => {
                alert.show('Recherche echouée: ' + error, AlertType.Info, AlertDuration.Long);
                progressUI.hide();
                console.log(error);
            }, this.searchDurationMs);
        });
    }
}
