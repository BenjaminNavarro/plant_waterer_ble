/* global mainPage, deviceList, refreshButton */
/* global detailPage, batteryState, batteryStateButton, disconnectButton */
/* global ble  */
/* jshint browser: true , devel: true*/
'use strict';

const qs = (query: string) => document.querySelector(query) as HTMLElement;
const qsa = (query: string) => document.querySelectorAll(query);

function interpolate(template: string, params: object) {
    const replaceTags: Map<string, string> = new Map([
        ['&', '& amp;'], ['<', '& lt;'], ['>', '& gt;'], ['(', '% 28'], [')', '% 29']])

    const safeInnerHTML = text => text.toString()
        .replace(/[&<>()]/g, tag => replaceTags.get(tag) || tag)
    const keys = Object.keys(params)
    const keyVals = Object.values(params).map(safeInnerHTML)
    /* eslint no-new-func: "off" */
    return new Function(...keys, `return \`${template}\``)(...keyVals)
}

class TestConfig {
    outputState: boolean[];
    flowSpeed: number;

    constructor(size: number) {
        this.outputState.fill(false, 0, size)
        this.flowSpeed = 0
    }
}

class Schedule {
    enabled: boolean;
    flow_speed: number;
    start_time: number;
    watering_duration: number;
    watering_period: number;
}

class Program {
    schedule: Schedule[];

    constructor(size: number) {
        this.schedule.fill(new Schedule(), 0, size)
    }
}

const outputs_count = 8
const address = qs('#system_address_input').innerHTML
const apiBaseUrl = 'http://' + address + '/api/v1/'

async function readApi(endpoint: string, onSuccess: CallableFunction = null, onError: CallableFunction = null) {
    try {
        const response = await fetch(apiBaseUrl + endpoint, { method: 'GET', signal: AbortSignal.timeout(5000) })
        if (!response.ok) {
            onError()
        }

        const result = await response.json()
        onSuccess(result)
    } catch (error) {
        onError()
    }
}

async function writeApi(endpoint: string, data: object, onComplete: CallableFunction = null) {
    const response = await fetch(apiBaseUrl + endpoint, {
        method: 'POST',
        headers: {
            "Content-Type": "application/json",
        },
        body: JSON.stringify(data),
        signal: AbortSignal.timeout(5000)
    })
    onComplete(response)
}

function init() {
    function hide(elem: HTMLElement) {
        elem.style.display = 'none'
    }

    function show(elem: HTMLElement) {
        elem.style.display = 'block'
    }

    const deviceListContainer = qs('#device_list')
    const deviceListEntryTemplate = qs('#device_list_entry_template')

    const testOutputContainer = qs('#test_output_container')
    const testOutputTemplate = qs('#test_output_template')
    for (let index = 1; index <= 8; index++) {
        const data = { index }
        testOutputContainer.innerHTML += interpolate(testOutputTemplate.innerHTML.toString().trim(), data)
    }

    let mmm: FomanticUI.Calendar;

    const mainView = qs('#main_view')
    const statusBar = qs('#status_bar')
    const statusText = qs('#status_text')
    const statusLoader = qs('#status_loader')
    const searchBtn = qs('#search_btn') as HTMLButtonElement
    const testBtn = qs('#test_btn') as HTMLButtonElement
    const stopBtn = qs('#stop_btn') as HTMLButtonElement
    const applyBtn = qs('#apply_btn') as HTMLButtonElement
    const testFlowSpeed = qs('#test_flow_speed') as HTMLInputElement
    const pumpOnBtn = qs('#pump_on_btn') as HTMLButtonElement
    const pumpOffBtn = qs('#pump_off_btn') as HTMLButtonElement
    const calendar = qs('#standard_calendar')
    const programSelection = qs('#program_selection') as HTMLSelectElement
    const programEnabled = qs('#program_enabled')
    const programFlowSpeed = qs('#program_flow_speed') as HTMLInputElement
    const programEnabledInput = qs('#program_enabled_input') as HTMLInputElement
    const programDuration = qs('#program_duration') as HTMLInputElement
    const programPeriod = qs('#program_period') as HTMLInputElement
    const programDurationTimeSelection = qs('#program_duration_time_type') as HTMLSelectElement
    const programPeriodTimeSelection = qs('#program_period_time_type') as HTMLSelectElement

    let program = new Program(outputs_count)
    let programDurationTimeType = 's'
    let programPeriodTimeType = 'd'
    let testConfig = new TestConfig(8)

    function sendTestConfig(flowSpeed: number) {
        type requestType = {
            flow_speed: number,
            output_state: number[]
        }
        let request: requestType;
        request.flow_speed = flowSpeed / 100
        for (let index = 0; index < 8; index++) {
            request.output_state.push(testConfig.outputState[index] ? 1 : 0)
        }
        statusText.innerHTML = 'Envoi'
        writeApi(
            'manual/write',
            request,
            function (resp: Response) {
                if (resp.statusText === 'ok') {
                    statusText.innerHTML = 'Ok'
                } else {
                    statusText.innerHTML = 'Erreur'
                }
                hide(statusLoader)
            })
        show(statusLoader)
    }

    function sendProgram() {
        statusText.innerHTML = 'Envoi'
        writeApi(
            'schedule/write',
            program,
            function (resp: Response) {
                if (resp.statusText === 'ok') {
                    statusText.innerHTML = 'Ok'
                } else {
                    statusText.innerHTML = 'Erreur'
                }
                hide(statusLoader)
            })
        show(statusLoader)
    }

    function sendProgramTest(config: TestConfig) {
        statusText.innerHTML = 'Envoi'
        writeApi(
            'program_test/write',
            config,
            function (resp: Response) {
                if (resp.statusText === 'ok') {
                    statusText.innerHTML = 'Ok'
                } else {
                    statusText.innerHTML = 'Erreur'
                }
                hide(statusLoader)
            })
        show(statusLoader)
    }

    function displayedSchedule() {
        const index = parseInt(programSelection.options[programSelection.selectedIndex].value)
        return program.schedule[index]
    }

    function toSeconds(value: number, unit: string) {
        if (unit === 'm') {
            return value * 60
        } else if (unit === 'h') {
            return value * 3660
        } else if (unit === 'd') {
            return value * 86400
        } else {
            return value
        }
    }

    function updateProgram() {
        const schedule = displayedSchedule()
        programEnabledInput.checked = schedule.enabled
        programFlowSpeed.value = (schedule.flow_speed * 100).toString()
        calendar.calendar('set date', new Date(schedule.start_time * 1000), true, false)

        if (schedule.watering_duration > 3600) {
            programDurationTimeType = 'h'
            programDuration.val(schedule.watering_duration / 3600)
        } else if (schedule.watering_duration > 60) {
            programDurationTimeType = 'm'
            programDuration.val(schedule.watering_duration / 60)
        } else {
            programDurationTimeType = 's'
            programDuration.val(schedule.watering_duration)
        }
        programDurationTimeSelection.dropdown(
            'set selected', programDurationTimeType, true
        )

        if (schedule.watering_period > 86400) {
            programPeriodTimeType = 'd'
            programPeriod.val(schedule.watering_period / 86400)
        } else if (schedule.watering_period > 3600) {
            programPeriodTimeType = 'h'
            programPeriod.val(schedule.watering_period / 3600)
        } else if (schedule.watering_period > 60) {
            programPeriodTimeType = 'm'
            programPeriod.val(schedule.watering_period / 60)
        } else {
            programPeriodTimeType = 's'
            programPeriod.val(schedule.watering_period)
        }
        programPeriodTimeSelection.dropdown(
            'set selected', programPeriodTimeType, true
        )
    }

    programSelection.dropdown({
        onChange: updateProgram
    })

    programEnabled.checkbox({
        onChecked: function () {
            const schedule = displayedSchedule()
            schedule.enabled = true
        },
        onUnchecked: function () {
            const schedule = displayedSchedule()
            schedule.enabled = false
        }
    })

    programFlowSpeed.on('change', function (value) {
        const schedule = displayedSchedule()
        schedule.flow_speed = parseFloat(value.currentTarget.value) / 100
    })

    calendar.calendar({
        firstDayOfWeek: 1,
        today: true,
        formatter: {
            time: 'H:mm',
            cellTime: 'H:mm',
            date: 'ddd D MMMM Y',
            datetime: 'dddd D MMMM, H:mm'
        },
        text: {
            days: ['D', 'L', 'M', 'M', 'J', 'V', 'S'],
            dayNamesShort: ['Dim', 'Lun', 'Mar', 'Mer', 'Jeu', 'Ven', 'Sam'],
            dayNames: ['Dimanche', 'Lundi', 'Mardi', 'Mercredi', 'Jeudi', 'Vendredi', 'Samedi'],
            months: ['Janvier', 'Février', 'Mars', 'Avril', 'Mai', 'Juin', 'Juillet', 'Août', 'Septembre', 'Octobre', 'Novembre', 'Decembre'],
            monthsShort: ['Jan', 'Fev', 'Mar', 'Avr', 'Mai', 'Juin', 'Juil', 'Aou', 'Sep', 'Oct', 'Nov', 'Dec'],
            today: "Aujourd'hui",
            now: 'Maintenant',
            am: 'AM',
            pm: 'PM',
            weekNo: 'Semaine'
        },
        onChange: function (date, text, mode) {
            const schedule = displayedSchedule()
            const unixTime = Math.floor(date.getTime() / 1000)
            schedule.start_time = unixTime
        }
    })

    programDuration.on('change', function (value) {
        const schedule = displayedSchedule()
        schedule.watering_duration = toSeconds(
            parseInt(value.currentTarget.value),
            programDurationTimeType)
    })

    programDurationTimeSelection.dropdown({
        onChange: function (value) {
            programDurationTimeType = value
            const schedule = displayedSchedule()
            schedule.watering_duration = toSeconds(
                programDuration.val(),
                programDurationTimeType)
        }
    })

    programPeriod.on('change', function (value) {
        const schedule = displayedSchedule()
        schedule.watering_period = toSeconds(
            parseInt(value.currentTarget.value),
            programPeriodTimeType)
    })

    programPeriodTimeSelection.dropdown({
        onChange: function (value) {
            programPeriodTimeType = value
            const schedule = displayedSchedule()
            schedule.watering_period = toSeconds(
                programPeriod.val(),
                programPeriodTimeType)
        }
    })

    qs('.menu .item').tab({
        onLoad: function () {
            const request = {}
            request.mode = $(this).data('tab')
            writeApi('mode/write', request)
        }
    })

    hide(mainView)
    hide(statusBar)
    hide(statusLoader)
    hide(stopBtn)
    hide(pumpOffBtn)

    searchBtn.addEventListener('click', function () {
        statusText.innerHTML = 'Recherche'
        show(statusBar)
        show(statusLoader)

        const debug = document.getElementById('debug')
        debug.innerHTML = ''
        deviceListContainer.innerHTML = ''
        var index = 0
        ble.startScanWithOptions(
            [],
            { scanMode: 'lowLatency' },
            function (device: { name: string; id: any; }) {
                console.log(device)
                if (device.name && device.name.startsWith('NimBLE_GATT')) {
                    const name = device.name
                    const id = device.id
                    const data = { index, name, id }
                    const entryHTML = interpolate(deviceListEntryTemplate.innerHTML.toString().trim(), data)
                    console.log(entryHTML)
                    deviceListContainer.innerHTML += entryHTML
                    document.getElementById('device_list_name_' + index).onclick = function () {
                        ble.connect(device.id, function (services) {
                            // connected
                            console.log("connected");
                            console.log(services);
                            debug.innerHTML += JSON.stringify(services) + '<br/>'
                        }, function () {
                            console.log("disconnected");
                            // disconnected
                        });

                    };
                    // document.getElementById('device_list_id_' + index).innerHTML = device.id;

                    ++index

                }
            },
            function () {
                hide(statusLoader)
                statusText.innerHTML = 'Echec'
            }
        );

        setTimeout(
            ble.stopScan,
            5000,
            function () {
                hide(statusLoader)
                // show(mainView)
                statusText.innerHTML = ''
            },
            function () {
                hide(statusLoader)
                statusText.innerHTML = 'Echec'
            }
        );
        // ble.scan(
        //   [],
        //   5,
        //   function (device) {
        //     debug.innerHTML += JSON.stringify(device) + '<br/>';
        //   },
        //   function () {
        //     hide(statusLoader)
        //     statusText.innerHTML = 'Echec'
        //   }
        // )

        // readApi(
        //   'schedule/read',
        //   function (data) {
        //     hide(statusLoader)
        //     show(mainView)
        //     statusText.innerHTML = 'Connecté'

        //     program = data
        //     updateProgram()

        //     const connectionCheckTimer = setInterval(function () {
        //       readApi(
        //         'system/info',
        //         function () {
        //           console.log('Still connected')
        //         },
        //         function () {
        //           hide(mainView)
        //           statusText.innerHTML = 'Déconnecté'
        //           clearInterval(connectionCheckTimer)
        //         })
        //     }, 10000)
        //   },
        //   function () {
        //     hide(statusLoader)
        //     statusText.innerHTML = 'Echec'
        //   }
        // )
    })

    testBtn.click(function () {
        const index = parseInt(programSelection.val())
        const schedule = program.schedule[index]

        const config = {}
        config.output = index
        config.duration = schedule.watering_duration
        config.flow_speed = schedule.flow_speed
        console.log(config)
        sendProgramTest(config)
    })

    stopBtn.click(function () {
        show(testBtn)
        hide(stopBtn)

        statusText.innerHTML = 'Connecté'
        hide(statusLoader)
    })

    applyBtn.click(function () {
        sendProgram()
    })

    pumpOnBtn.click(function () {
        hide(pumpOnBtn)
        show(pumpOffBtn)
        sendTestConfig(testConfig.flowSpeed)
    })

    pumpOffBtn.click(function () {
        show(pumpOnBtn)
        hide(pumpOffBtn)
        sendTestConfig(0)
    })

    testFlowSpeed.on('change', function (value) {
        testConfig.flowSpeed = value.currentTarget.value
    })

    $('.ui.test.toggle.checkbox').checkbox({
        onChecked: function () {
            const index = parseInt($(this).data('value'))
            testConfig.outputState[index - 1] = true
            sendTestConfig()
        },
        onUnchecked: function () {
            const index = parseInt($(this).data('value'))
            testConfig.outputState[index - 1] = false
            sendTestConfig()
        }
    })
}

function ble_scan() {
    document.addEventListener('deviceready', function () {
        const debug = document.getElementById('debug')
        ble.scan(
            [],
            5,
            function (device) {
                debug.innerHTML += JSON.stringify(device) + '<br/>';
            },
        )
    }, false);
}


(function () {
    init();
})();