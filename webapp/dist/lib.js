"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.testFromTS = testFromTS;
function qs(query) {
    return document.querySelector(query);
}
function qsa(query) {
    return document.querySelectorAll(query);
}
function interpolate(template, params) {
    const replaceTags = new Map([
        ['&', '& amp;'], ['<', '& lt;'], ['>', '& gt;'], ['(', '% 28'], [')', '% 29']
    ]);
    const safeInnerHTML = text => text.toString()
        .replace(/[&<>()]/g, tag => replaceTags.get(tag) || tag);
    const keys = Object.keys(params);
    const keyVals = Object.values(params).map(safeInnerHTML);
    /* eslint no-new-func: "off" */
    return new Function(...keys, `return \`${template}\``)(...keyVals);
}
class ProgressRing {
    constructor(id) {
        this.progressRing = qs(id);
        this.hide();
    }
    setValue(value) {
        this.progressRing.value = value.toString();
    }
    getValue() {
        return Number(this.progressRing.value);
    }
    hide() {
        if (!this.progressRing.classList.contains("is_hidden")) {
            this.progressRing.classList.add("is_hidden");
        }
    }
    show() {
        if (this.progressRing.classList.contains("is_hidden")) {
            this.progressRing.classList.remove("is_hidden");
        }
    }
    startAutoUpdate(durationMs) {
        let searchProgress = 0;
        this.autoUpdateInterval = setInterval(() => {
            this.progressRing.value = searchProgress.toString();
            searchProgress += 100 / (durationMs / this.updateRateMs);
            if (searchProgress >= 100) {
                searchProgress = 100;
                this.stopAutoUpdate();
                this.setValue(0);
                this.hide();
            }
        }, this.updateRateMs);
        this.setValue(0);
        this.show();
    }
    stopAutoUpdate() {
        clearInterval(this.autoUpdateInterval);
        this.autoUpdateInterval = -1;
    }
    updateRateMs = 100;
    progressRing;
    autoUpdateInterval = -1;
}
class ProgressSpinner {
    constructor(id) {
        this.progressSpinner = qs(id);
        this.hide();
    }
    hide() {
        if (!this.progressSpinner.classList.contains("is_hidden")) {
            this.progressSpinner.classList.add("is_hidden");
        }
    }
    show() {
        if (this.progressSpinner.classList.contains("is_hidden")) {
            this.progressSpinner.classList.remove("is_hidden");
        }
    }
    progressSpinner;
}
function testFromTS() {
    const progressRing = new ProgressRing('#progress_ring');
    const progressSpinner = new ProgressSpinner('#progress_spinner');
    const tabs = qs('#tabs');
    const searchButton = qs('#search_button');
    const searchDurationMs = 5000;
    const deviceList = qs('#device_list');
    const deviceListEntryTemplate = qs('#device_list_entry_template');
    searchButton.addEventListener('click', () => {
        progressRing.startAutoUpdate(searchDurationMs);
        for (let index = 0; index < 3; index++) {
            const name = "Device #" + (index + 1).toString();
            const id = index;
            const data = { index, name, id };
            const entryHTML = interpolate(deviceListEntryTemplate.innerHTML.toString().trim(), data);
            deviceList.innerHTML += entryHTML;
        }
        qsa('.connect-button').forEach(btn => {
            btn.addEventListener('click', () => {
                progressRing.hide();
                progressSpinner.show();
                setTimeout(() => {
                    progressSpinner.hide();
                    tabs.show('program');
                }, 1000);
            });
        });
    });
}
