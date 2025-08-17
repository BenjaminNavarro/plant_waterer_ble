import { qs } from './utils.js'
import { SlTabGroup, SlTab, SlAlert, SlTabShowEvent } from 'shoelace/shoelace'

import { ProgressUI } from './progress_ui.js'
import { ConnectPanel } from './connect_panel.js'
import { ConfigPanel } from './config_panel.js'
import { ManualPanel } from './test_panel.js'

import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from './tinydrop_device.js'
import { TinyDropScanner, TinyDropFakeScanner, TinyDropBLEScanner } from './tinydrop_scanner.js'
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

    const scanner = isMobile() ? new TinyDropBLEScanner(statusBar) : new TinyDropFakeScanner(statusBar)

    const configPanel = new ConfigPanel(progressUI)
    const testPanel = new ManualPanel(progressUI)

    const connectPanel = new ConnectPanel(progressUI, statusBar, scanner, (device: TinyDropBLEDevice) => {
        configTab.disabled = false
        manualTab.disabled = false
        configPanel.setDevice(device)
        testPanel.setDevice(device)
        tabs.show('config')
    }, () => {
        configTab.disabled = true
        manualTab.disabled = true
        configPanel.setDevice(null)
        testPanel.setDevice(null)
        tabs.show('connect')
    })

    configTab.disabled = true
    manualTab.disabled = true

}

function isMobile() {
    var userAgent = navigator.userAgent

    const isAndroid = /android/i.test(userAgent)
    const isIOS = /iPad|iPhone|iPod/.test(userAgent)

    return isAndroid || isIOS
}

main()