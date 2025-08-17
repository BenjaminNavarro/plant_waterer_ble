import { qs } from './utils.js';
import { ProgressUI } from './progress_ui.js';
import { ConnectPanel } from './connect_panel.js';
import { ConfigPanel } from './config_panel.js';
import { ManualPanel } from './test_panel.js';
import { TinyDropFakeScanner, TinyDropBLEScanner } from './tinydrop_scanner.js';
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
    const scanner = isMobile() ? new TinyDropBLEScanner(statusBar) : new TinyDropFakeScanner(statusBar);
    const configPanel = new ConfigPanel(progressUI);
    const testPanel = new ManualPanel(progressUI);
    const connectPanel = new ConnectPanel(progressUI, scanner, (device) => {
        configTab.disabled = false;
        manualTab.disabled = false;
        configPanel.setDevice(device);
        testPanel.setDevice(device);
        tabs.show('config');
    }, () => {
        configTab.disabled = true;
        manualTab.disabled = true;
        configPanel.setDevice(null);
        testPanel.setDevice(null);
        tabs.show('connect');
    });
    configTab.disabled = true;
    manualTab.disabled = true;
}
function isMobile() {
    var userAgent = navigator.userAgent;
    const isAndroid = /android/i.test(userAgent);
    const isIOS = /iPad|iPhone|iPod/.test(userAgent);
    return isAndroid || isIOS;
}
main();
