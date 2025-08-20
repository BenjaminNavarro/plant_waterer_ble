import 'bulma/css/versions/bulma-prefixed.min.css'
import '@shoelace-style/shoelace/dist/themes/dark.css'

import { qs } from './utils.ts'
import { SlTabGroup, SlTab, SlIcon } from '@shoelace-style/shoelace'
import type { SlTabShowEvent } from '@shoelace-style/shoelace'

import { ProgressUI } from './progress_ui.ts'
import { ConnectPanel } from './connect_panel.ts'
import { ConfigPanel } from './config_panel.ts'
import { ManualPanel } from './test_panel.ts'

import { TinyDropDevice, TinyDropBLEDevice, TinyDropFakeDevice } from './tinydrop_device.ts'
import { TinyDropScanner, TinyDropFakeScanner, TinyDropBLEScanner } from './tinydrop_scanner.ts'
import { StatusBar } from './status_bar.ts'
import { logger } from './logger.ts'

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

    const connectPanel = new ConnectPanel(progressUI, scanner,
        (device: TinyDropDevice) => {
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