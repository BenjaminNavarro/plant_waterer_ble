import { ProgressUI } from './progress_ui.js';
import { TinyDropScanner } from './tinydrop_scanner.js';
import { StatusBar } from './status_bar.js';
export declare class ConnectPanel {
    searchButton: HTMLElement;
    searchDurationMs: number;
    deviceList: HTMLElement;
    deviceListEntryTemplate: HTMLElement;
    constructor(progressUI: ProgressUI, statusBar: StatusBar, scanner: TinyDropScanner, onConnect: CallableFunction, onDisconnect: CallableFunction);
}
