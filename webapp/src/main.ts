import { qs } from './utils.js'
import { SlTabGroup, SlTab, SlAlert, SlTabShowEvent } from 'shoelace/shoelace'

import { ProgressUI } from './progress_ui.js'
import { ConnectPanel } from './connect_panel.js'
import { ConfigPanel } from './config_panel.js'
import { ManualPanel } from './test_panel.js'

import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from './tinydrop_device.js'
import { TinyDropScanner, TinyDropFakeScanner } from './tinydrop_scanner.js'

function main() {
    const tabs = qs<SlTabGroup>('#tabs')
    const configTab = qs<SlTab>('#config_tab')
    const manualTab = qs<SlTab>('#manual_tab')

    qs('#tabs').addEventListener('sl-tab-show', (ev: SlTabShowEvent) => {
        console.log(`Tab ${ev.detail.name} is shown`);
    })

    const progressUI = new ProgressUI()

    // const device = new TinyDropBLEDevice()
    const fake_scanner = new TinyDropFakeScanner()

    const connectPanel = new ConnectPanel(progressUI, fake_scanner, () => {
        configTab.disabled = false
        manualTab.disabled = false
        tabs.show('config')
    }, () => {
        configTab.disabled = true
        manualTab.disabled = true
        tabs.show('connect')
    })

    const configPanel = new ConfigPanel(progressUI)
    const testPanel = new ManualPanel(progressUI)

    configTab.disabled = true
    manualTab.disabled = true
    // tabs.show('test')

}

main()