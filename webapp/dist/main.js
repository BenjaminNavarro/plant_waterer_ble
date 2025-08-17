import { qs } from './utils.js';
import { ProgressUI } from './progress_ui.js';
import { ConnectPanel } from './connect_panel.js';
import { ConfigPanel } from './config_panel.js';
import { ManualPanel } from './test_panel.js';
import { TinyDropFakeScanner } from './tinydrop_scanner.js';
import { StatusBar } from './status_bar.js';
function main() {
    const tabs = qs('#tabs');
    const configTab = qs('#config_tab');
    const manualTab = qs('#manual_tab');
    qs('#tabs').addEventListener('sl-tab-show', (ev) => {
        console.log(`Tab ${ev.detail.name} is shown`);
    });
    const progressUI = new ProgressUI();
    const statusBar = new StatusBar();
    // const device = new TinyDropBLEDevice()
    const fake_scanner = new TinyDropFakeScanner(statusBar);
    const configPanel = new ConfigPanel(progressUI);
    const testPanel = new ManualPanel(progressUI);
    const connectPanel = new ConnectPanel(progressUI, statusBar, fake_scanner, (device) => {
        configTab.disabled = false;
        manualTab.disabled = false;
        configPanel.device = device;
        testPanel.device = device;
        tabs.show('config');
    }, () => {
        configTab.disabled = true;
        manualTab.disabled = true;
        configPanel.device = null;
        testPanel.device = null;
        tabs.show('connect');
    });
    configTab.disabled = true;
    manualTab.disabled = true;
    // tabs.show('test')
}
main();
