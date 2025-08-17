import { qs } from './utils.js'
import { SlTabGroup, SlTab, SlAlert, SlTabShowEvent } from 'shoelace/shoelace'

import { ProgressUI } from './progress_ui.js'
import { ConnectPanel } from './connect_panel.js'
import { ConfigPanel } from './config_panel.js'
import { ManualPanel } from './test_panel.js'

import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from './tinydrop_device.js'
import { TinyDropScanner, TinyDropFakeScanner } from './tinydrop_scanner.js'
import { StatusBar } from './status_bar.js'

function main() {
    const tabs = qs<SlTabGroup>('#tabs')
    const configTab = qs<SlTab>('#config_tab')
    const manualTab = qs<SlTab>('#manual_tab')

    qs('#tabs').addEventListener('sl-tab-show', (ev: SlTabShowEvent) => {
        console.log(`Tab ${ev.detail.name} is shown`);
    })

    const progressUI = new ProgressUI()
    const statusBar = new StatusBar()

    // const device = new TinyDropBLEDevice()
    const fake_scanner = new TinyDropFakeScanner(statusBar)

    const configPanel = new ConfigPanel(progressUI)
    const testPanel = new ManualPanel(progressUI)

    const connectPanel = new ConnectPanel(progressUI, statusBar, fake_scanner, (device: TinyDropBLEDevice) => {
        configTab.disabled = false
        manualTab.disabled = false
        configPanel.device = device
        testPanel.device = device
        tabs.show('config')
    }, () => {
        configTab.disabled = true
        manualTab.disabled = true
        configPanel.device = null
        testPanel.device = null
        tabs.show('connect')
    })

    configTab.disabled = true
    manualTab.disabled = true
    // tabs.show('test')

}

main()