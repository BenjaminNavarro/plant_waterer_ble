import { qs } from './utils.js';
import { ProgressUI } from './progress_ui.js';
import { ConnectPanel } from './connect_panel.js';
import { ConfigPanel } from './config_panel.js';
import { TinyDropFakeScanner } from './tinydrop_scanner.js';
import { Alert } from './alert.js';
function main() {
    const tabs = qs('#tabs');
    const configTab = qs('#config_tab');
    const testTab = qs('#test_tab');
    const progressUI = new ProgressUI();
    const alert = new Alert();
    // const device = new TinyDropBLEDevice()
    const fake_scanner = new TinyDropFakeScanner();
    const connectPanel = new ConnectPanel(progressUI, fake_scanner, alert, () => {
        configTab.disabled = false;
        testTab.disabled = false;
        tabs.show('config');
    }, () => {
        configTab.disabled = true;
        testTab.disabled = true;
        tabs.show('connect');
    });
    const configPanel = new ConfigPanel(progressUI);
    configTab.disabled = true;
    // tabs.show('config')
    testTab.disabled = true;
}
main();
