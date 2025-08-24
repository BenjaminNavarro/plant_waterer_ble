import 'bulma/css/versions/bulma-prefixed.min.css'
import '@shoelace-style/shoelace/dist/themes/dark.css'
import { EdgeToEdge } from '@capawesome/capacitor-android-edge-to-edge-support';

import { qs } from './utils.ts'
import { SlTabGroup, SlTab, SlIcon } from '@shoelace-style/shoelace'
import type { SlTabShowEvent } from '@shoelace-style/shoelace'

import { ProgressUI, ResultType } from './progress_ui.ts'
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

    const statusBar = new StatusBar()

    const scanner = isMobile() ? new TinyDropBLEScanner(statusBar) : new TinyDropFakeScanner(statusBar)

    const configPanel = new ConfigPanel(statusBar.progress)
    const testPanel = new ManualPanel(statusBar.progress)

    const onConnect = (device: TinyDropDevice) => {
        device.setProgressUI(statusBar.progress)
        configTab.disabled = false
        manualTab.disabled = false
        configPanel.setDevice(device)
        testPanel.setDevice(device)
        tabs.show('config')
    }

    const onDisconnect = () => {
        configTab.disabled = true
        manualTab.disabled = true
        configPanel.setDevice(null)
        testPanel.setDevice(null)
        tabs.show('connect')
    }

    const connectPanel = new ConnectPanel(statusBar.progress, scanner, onConnect, onDisconnect)

    configTab.disabled = true
    manualTab.disabled = true

    let visibilityTimeoutHandle = -1
    document.addEventListener("visibilitychange", function () {
        if (document.hidden) {
            clearTimeout(visibilityTimeoutHandle)
            visibilityTimeoutHandle = window.setTimeout(() => {
                connectPanel.disconnect(onDisconnect)
            }, 30000)
        }
        else {
            clearTimeout(visibilityTimeoutHandle)
        }
        logger.log(`visibility changed: hidden=${document.hidden}, state=${document.visibilityState}`)
    }, false);

    document.body.classList.remove('is_hidden')

    if (!window.matchMedia('(prefers-color-scheme: dark)').matches) {
        EdgeToEdge.setBackgroundColor({ color: '#ffffff' });
    }

    window.matchMedia('(prefers-color-scheme: dark)')
        .addEventListener('change', ({ matches }) => {
            if (matches) {
                EdgeToEdge.setBackgroundColor({ color: '#14161a' });
            } else {
                EdgeToEdge.setBackgroundColor({ color: '#ffffff' });
            }
        })
}

function isMobile() {
    var userAgent = navigator.userAgent

    const isAndroid = /android/i.test(userAgent)
    const isIOS = /iPad|iPhone|iPod/.test(userAgent)

    return isAndroid || isIOS
}

main()