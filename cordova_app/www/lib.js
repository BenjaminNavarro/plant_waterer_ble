/* global mainPage, deviceList, refreshButton */ /* global detailPage, batteryState, batteryStateButton, disconnectButton */ /* global ble  */ /* jshint browser: true , devel: true*/ 'use strict';
const $cfc67dcce26d371a$var$qs = (query)=>document.querySelectorAll(query);
function $cfc67dcce26d371a$var$interpolate(template, params) {
    const replaceTags = {
        '&': '& amp;',
        '<': '& lt;',
        '>': '& gt;',
        '(': '% 28',
        ')': '% 29'
    };
    const safeInnerHTML = (text)=>text.toString().replace(/[&<>()]/g, (tag)=>replaceTags[tag] || tag);
    const keys = Object.keys(params);
    const keyVals = Object.values(params).map(safeInnerHTML);
    /* eslint no-new-func: "off" */ return new Function(...keys, `return \`${template}\``)(...keyVals);
}
function $cfc67dcce26d371a$var$init() {
    function hide(elem) {
        elem.css('display', 'none');
    }
    function show(elem) {
        elem.css('display', 'block');
    }
    const deviceListContainer = document.getElementById('device_list');
    const deviceListEntryTemplate = document.getElementById('device_list_entry_template');
    const testOutputContainer = document.getElementById('test_output_container');
    const testOutputTemplate = document.getElementById('test_output_template');
    for(let index = 1; index <= 8; index++){
        const data = {
            index: index
        };
        testOutputContainer.innerHTML += $cfc67dcce26d371a$var$interpolate(testOutputTemplate.innerHTML.toString().trim(), data);
    }
    const mainView = $('#main_view');
    const statusBar = $('#status_bar');
    const statusText = $('#status_text');
    const statusLoader = $('#status_loader');
    const searchBtn = $('#search_btn');
    const testBtn = $('#test_btn');
    const stopBtn = $('#stop_btn');
    const applyBtn = $('#apply_btn');
    const testFlowSpeed = $('#test_flow_speed');
    const pumpOnBtn = $('#pump_on_btn');
    const pumpOffBtn = $('#pump_off_btn');
    const calendar = $('#standard_calendar');
    const programSelection = $('#program_selection');
    const programEnabled = $('#program_enabled');
    const programFlowSpeed = $('#program_flow_speed');
    const programEnabledInput = $('#program_enabled_input');
    const programDuration = $('#program_duration');
    const programPeriod = $('#program_period');
    const programDurationTimeSelection = $('#program_duration_time_type');
    const programPeriodTimeSelection = $('#program_period_time_type');
    let program = {};
    let programDurationTimeType = 's';
    let programPeriodTimeType = 'd';
    let testConfig = {};
    testConfig.outputState = [
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false
    ];
    testConfig.flowSpeed = 0;
    function sendTestConfig(flowSpeed) {
        const request = {};
        request.flow_speed = flowSpeed / 100;
        request.output_state = [];
        for(let index = 0; index < 8; index++)request.output_state.push(testConfig.outputState[index] ? 1 : 0);
        statusText.html('Envoi');
        writeApi('manual/write', request, function(resp) {
            if (resp.responseText === 'ok') statusText.html('Ok');
            else statusText.html('Erreur');
            hide(statusLoader);
        });
        show(statusLoader);
    }
    function sendProgram() {
        statusText.html('Envoi');
        writeApi('schedule/write', program, function(resp) {
            if (resp.responseText === 'ok') statusText.html('Ok');
            else statusText.html('Erreur');
            hide(statusLoader);
        });
        show(statusLoader);
    }
    function sendProgramTest(config) {
        statusText.html('Envoi');
        writeApi('program_test/write', config, function(resp) {
            if (resp.responseText === 'ok') statusText.html('Ok');
            else statusText.html('Erreur');
            hide(statusLoader);
        });
        show(statusLoader);
    }
    function displayedSchedule() {
        const index = parseInt(programSelection.val());
        return program.schedule[index];
    }
    function toSeconds(value, unit) {
        if (unit === 'm') return value * 60;
        else if (unit === 'h') return value * 3660;
        else if (unit === 'd') return value * 86400;
        else return value;
    }
    function updateProgram() {
        const schedule = displayedSchedule();
        programEnabledInput.prop('checked', schedule.enabled === 1);
        programFlowSpeed.val(schedule.flow_speed * 100);
        calendar.calendar('set date', new Date(schedule.start_time * 1000), true, false);
        if (schedule.watering_duration > 3600) {
            programDurationTimeType = 'h';
            programDuration.val(schedule.watering_duration / 3600);
        } else if (schedule.watering_duration > 60) {
            programDurationTimeType = 'm';
            programDuration.val(schedule.watering_duration / 60);
        } else {
            programDurationTimeType = 's';
            programDuration.val(schedule.watering_duration);
        }
        programDurationTimeSelection.dropdown('set selected', programDurationTimeType, true);
        if (schedule.watering_period > 86400) {
            programPeriodTimeType = 'd';
            programPeriod.val(schedule.watering_period / 86400);
        } else if (schedule.watering_period > 3600) {
            programPeriodTimeType = 'h';
            programPeriod.val(schedule.watering_period / 3600);
        } else if (schedule.watering_period > 60) {
            programPeriodTimeType = 'm';
            programPeriod.val(schedule.watering_period / 60);
        } else {
            programPeriodTimeType = 's';
            programPeriod.val(schedule.watering_period);
        }
        programPeriodTimeSelection.dropdown('set selected', programPeriodTimeType, true);
    }
    programSelection.dropdown({
        onChange: updateProgram
    });
    programEnabled.checkbox({
        onChecked: function() {
            const schedule = displayedSchedule();
            schedule.enabled = true;
        },
        onUnchecked: function() {
            const schedule = displayedSchedule();
            schedule.enabled = false;
        }
    });
    programFlowSpeed.on('change', function(value) {
        const schedule = displayedSchedule();
        schedule.flow_speed = parseFloat(value.currentTarget.value) / 100;
    });
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
            days: [
                'D',
                'L',
                'M',
                'M',
                'J',
                'V',
                'S'
            ],
            dayNamesShort: [
                'Dim',
                'Lun',
                'Mar',
                'Mer',
                'Jeu',
                'Ven',
                'Sam'
            ],
            dayNames: [
                'Dimanche',
                'Lundi',
                'Mardi',
                'Mercredi',
                'Jeudi',
                'Vendredi',
                'Samedi'
            ],
            months: [
                'Janvier',
                "F\xe9vrier",
                'Mars',
                'Avril',
                'Mai',
                'Juin',
                'Juillet',
                "Ao\xfbt",
                'Septembre',
                'Octobre',
                'Novembre',
                'Decembre'
            ],
            monthsShort: [
                'Jan',
                'Fev',
                'Mar',
                'Avr',
                'Mai',
                'Juin',
                'Juil',
                'Aou',
                'Sep',
                'Oct',
                'Nov',
                'Dec'
            ],
            today: "Aujourd'hui",
            now: 'Maintenant',
            am: 'AM',
            pm: 'PM',
            weekNo: 'Semaine'
        },
        onChange: function(date, text, mode) {
            const schedule = displayedSchedule();
            const unixTime = Math.floor(date.getTime() / 1000);
            schedule.start_time = unixTime;
        }
    });
    programDuration.on('change', function(value) {
        const schedule = displayedSchedule();
        schedule.watering_duration = toSeconds(parseInt(value.currentTarget.value), programDurationTimeType);
    });
    programDurationTimeSelection.dropdown({
        onChange: function(value) {
            programDurationTimeType = value;
            const schedule = displayedSchedule();
            schedule.watering_duration = toSeconds(programDuration.val(), programDurationTimeType);
        }
    });
    programPeriod.on('change', function(value) {
        const schedule = displayedSchedule();
        schedule.watering_period = toSeconds(parseInt(value.currentTarget.value), programPeriodTimeType);
    });
    programPeriodTimeSelection.dropdown({
        onChange: function(value) {
            programPeriodTimeType = value;
            const schedule = displayedSchedule();
            schedule.watering_period = toSeconds(programPeriod.val(), programPeriodTimeType);
        }
    });
    $('.menu .item').tab({
        onLoad: function() {
            const request = {};
            request.mode = $(this).data('tab');
            writeApi('mode/write', request);
        }
    });
    hide(mainView);
    hide(statusBar);
    hide(statusLoader);
    hide(stopBtn);
    hide(pumpOffBtn);
    searchBtn.click(function() {
        statusText.html('Recherche');
        show(statusBar);
        show(statusLoader);
        const debug = document.getElementById('debug');
        debug.innerHTML = '';
        deviceListContainer.innerHTML = '';
        var index = 0;
        ble.startScanWithOptions([], {
            scanMode: 'lowLatency'
        }, function(device) {
            console.log(device);
            if (device.name && device.name.startsWith('NimBLE_GATT')) {
                const name = device.name;
                const id = device.id;
                const data = {
                    index: index,
                    name: name,
                    id: id
                };
                const entryHTML = $cfc67dcce26d371a$var$interpolate(deviceListEntryTemplate.innerHTML.toString().trim(), data);
                console.log(entryHTML);
                deviceListContainer.innerHTML += entryHTML;
                document.getElementById('device_list_name_' + index).onclick = function() {
                    ble.connect(device.id, function(services) {
                        // connected
                        console.log("connected");
                        console.log(services);
                        debug.innerHTML += JSON.stringify(services) + '<br/>';
                    }, function() {
                        console.log("disconnected");
                    // disconnected
                    });
                };
                // document.getElementById('device_list_id_' + index).innerHTML = device.id;
                ++index;
            }
        }, function() {
            hide(statusLoader);
            statusText.html('Echec');
        });
        setTimeout(ble.stopScan, 5000, function() {
            hide(statusLoader);
            // show(mainView)
            statusText.html('');
        }, function() {
            hide(statusLoader);
            statusText.html('Echec');
        });
    // ble.scan(
    //   [],
    //   5,
    //   function (device) {
    //     debug.innerHTML += JSON.stringify(device) + '<br/>';
    //   },
    //   function () {
    //     hide(statusLoader)
    //     statusText.html('Echec')
    //   }
    // )
    // readApi(
    //   'schedule/read',
    //   function (data) {
    //     hide(statusLoader)
    //     show(mainView)
    //     statusText.html('Connecté')
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
    //           statusText.html('Déconnecté')
    //           clearInterval(connectionCheckTimer)
    //         })
    //     }, 10000)
    //   },
    //   function () {
    //     hide(statusLoader)
    //     statusText.html('Echec')
    //   }
    // )
    });
    testBtn.click(function() {
        const index = parseInt(programSelection.val());
        const schedule = program.schedule[index];
        const config = {};
        config.output = index;
        config.duration = schedule.watering_duration;
        config.flow_speed = schedule.flow_speed;
        console.log(config);
        sendProgramTest(config);
    });
    stopBtn.click(function() {
        show(testBtn);
        hide(stopBtn);
        statusText.html("Connect\xe9");
        hide(statusLoader);
    });
    applyBtn.click(function() {
        sendProgram();
    });
    pumpOnBtn.click(function() {
        hide(pumpOnBtn);
        show(pumpOffBtn);
        sendTestConfig(testConfig.flowSpeed);
    });
    pumpOffBtn.click(function() {
        show(pumpOnBtn);
        hide(pumpOffBtn);
        sendTestConfig(0);
    });
    testFlowSpeed.on('change', function(value) {
        testConfig.flowSpeed = value.currentTarget.value;
    });
    $('.ui.test.toggle.checkbox').checkbox({
        onChecked: function() {
            const index = parseInt($(this).data('value'));
            testConfig.outputState[index - 1] = true;
            sendTestConfig();
        },
        onUnchecked: function() {
            const index = parseInt($(this).data('value'));
            testConfig.outputState[index - 1] = false;
            sendTestConfig();
        }
    });
}
function $cfc67dcce26d371a$var$ble_scan() {
    document.addEventListener('deviceready', function() {
        const debug = document.getElementById('debug');
        ble.scan([], 5, function(device) {
            debug.innerHTML += JSON.stringify(device) + '<br/>';
        });
    }, false);
}
(function() {
    $cfc67dcce26d371a$var$init();
})();


//# sourceMappingURL=lib.js.map
