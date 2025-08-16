import { ProgressUI } from './progress_ui.js';
import { TinyDropScanner } from './tinydrop_scanner.js';
export declare class ConnectPanel {
    searchButton: HTMLElement;
    searchDurationMs: number;
    deviceList: HTMLElement;
    deviceListEntryTemplate: HTMLElement;
    constructor(progressUI: ProgressUI, scanner: TinyDropScanner, onConnect: CallableFunction, onDisconnect: CallableFunction);
}
