function interpolate (template, params) {
  const replaceTags = {
    '&': '& amp;', '<': '& lt;', '>': '& gt;', '(': '% 28', ')': '% 29'
  }
  const safeInnerHTML = text => text.toString()
    .replace(/[&<>()]/g, tag => replaceTags[tag] || tag)
  const keys = Object.keys(params)
  const keyVals = Object.values(params).map(safeInnerHTML)
  /* eslint no-new-func: "off" */
  return new Function(...keys, `return \`${template}\``)(...keyVals)
}

$(document).ready(function () {
  function hide (elem) {
    elem.css('display', 'none')
  };

  function show (elem) {
    elem.css('display', 'block')
  };

  $('#standard_calendar').calendar({
    firstDayOfWeek: 1
  })

  $('.ui.dropdown').dropdown()

  $('.menu .item').tab()

  const mainView = $('#main_view')
  const statusBar = $('#status_bar')
  const statusText = $('#status_text')
  const statusLoader = $('#status_loader')
  const connectBtn = $('#connect_btn')
  const testBtn = $('#test_btn')
  const stopBtn = $('#stop_btn')
  const pumpOnBtn = $('#pump_on_btn')
  const pumpOffBtn = $('#pump_off_btn')

  // main_view.css('display', 'none');
  hide(statusBar)
  hide(statusLoader)
  hide(stopBtn)
  hide(pumpOffBtn)

  let loaderTimer = null

  connectBtn.click(function () {
    // const address = $("#system_address_input").val();
    // const url = "http://" + address + "/api/v1/schedule/read";
    // $.getJSON(url, data => {
    //     console.log(data);
    // });
    statusText.html('Connexion')
    show(statusBar)
    show(statusLoader)

    loaderTimer = setInterval(function () {
      hide(statusLoader)
      show(mainView)
      statusText.html('Connecté')
      clearInterval(loaderTimer)
    }, 1000)
  })

  testBtn.click(function () {
    hide(testBtn)
    show(stopBtn)

    statusText.html('Arrosage')
    show(statusLoader)

    loaderTimer = setInterval(function () {
      show(testBtn)
      hide(stopBtn)
      hide(statusLoader)
      statusText.html('Connecté')
      clearInterval(loaderTimer)
    }, 2000)
  })

  stopBtn.click(function () {
    show(testBtn)
    hide(stopBtn)

    statusText.html('Connecté')
    hide(statusLoader)

    clearInterval(loaderTimer)
  })

  pumpOnBtn.click(function () {
    hide(pumpOnBtn)
    show(pumpOffBtn)
  })

  pumpOffBtn.click(function () {
    show(pumpOnBtn)
    hide(pumpOffBtn)
  })

  const testOutputContainer = document.getElementById('test_output_container')
  const testOutputTemplate = document.getElementById('test_output_template')
  for (let index = 1; index <= 8; index++) {
    const data = { index }
    testOutputContainer.innerHTML += interpolate(testOutputTemplate.innerHTML.toString().trim(), data)
  }
})
