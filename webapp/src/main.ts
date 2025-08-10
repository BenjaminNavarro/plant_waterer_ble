import { qs } from './utils.js'
import { SlTabGroup, SlTab, SlAlert } from 'shoelace/shoelace'

import { ProgressUI } from './progress_ui.js'
import { ConnectPanel } from './connect_panel.js'
import { ConfigPanel } from './config_panel.js'
import { TestPanel } from './test_panel.js'

import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from './tinydrop_device.js'
import { TinyDropScanner, TinyDropFakeScanner } from './tinydrop_scanner.js'
import { Alert, AlertDuration, AlertType } from './alert.js'

function main() {
    const tabs = qs<SlTabGroup>('#tabs')
    const configTab = qs<SlTab>('#config_tab')
    const testTab = qs<SlTab>('#test_tab')

    const progressUI = new ProgressUI()
    const alert = new Alert()

    // const device = new TinyDropBLEDevice()
    const fake_scanner = new TinyDropFakeScanner()

    const connectPanel = new ConnectPanel(progressUI, fake_scanner, alert, () => {
        configTab.disabled = false
        testTab.disabled = false
        tabs.show('config')
    }, () => {
        configTab.disabled = true
        testTab.disabled = true
        tabs.show('connect')
    })

    const configPanel = new ConfigPanel(progressUI)
    const testPanel = new TestPanel(progressUI)

    configTab.disabled = true
    testTab.disabled = true
    // tabs.show('test')

}

main()