import { ProgressUI } from './progress_ui.js';
import { TinyDropScanner } from './tinydrop_scanner.js';
import { Alert } from './alert.js';
export declare class ConnectPanel {
    searchButton: HTMLElement;
    searchDurationMs: number;
    deviceList: HTMLElement;
    deviceListEntryTemplate: HTMLElement;
    constructor(progressUI: ProgressUI, scanner: TinyDropScanner, alert: Alert, onConnect: CallableFunction, onDisconnect: CallableFunction);
}
