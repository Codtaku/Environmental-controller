let lastMode = -1;
let lastData = {}; // Stores the last full dataset from /status
let isInitialPageLoad = true; // Flag for initial data load

// --- Tab Handling ---
function openTab(evt, tabName) {
    let i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(tabName).style.display = "block";
    if (evt && evt.currentTarget) {
        evt.currentTarget.className += " active";
    } else {
        // Fallback for programmatic tab opening
        const defaultTabButton = Array.from(tablinks).find(btn => btn.textContent.trim().includes(tabName));
        if (defaultTabButton) defaultTabButton.className += " active";
    }
}

// --- Gauge Initialization and Update ---
let gaugeTemp, gaugeHum, gaugeGas, gaugeSoil;
function initializeGauges() {
    // Common options for all gauges
    const baseGaugeOptions = {
        angle: 0.0, // Starting angle
        lineWidth: 0.3, // Relative to gauge radius
        radiusScale: 0.9, // Scale of the gauge radius
        pointer: { length: 0.5, strokeWidth: 0.035, color: '#333' },
        limitMax: false, limitMin: false,
        strokeColor: '#E0E0E0', // Background track color
        generateGradient: true, highDpiSupport: true,
        renderTicks: { // Options for rendering ticks
            divisions: 5, divWidth: 1.1, divLength: 0.5, divColor: '#666',
            subDivisions: 3, subLength: 0.3, subWidth: 0.6, subColor: '#aaa'
        },
        staticLabels: { font: "11px sans-serif", labels: [], color: "#000000", fractionDigits: 0 }
    };

    // Temperature Gauge
    gaugeTemp = new Gauge(document.getElementById('gauge-temp')).setOptions({
        ...baseGaugeOptions,
        staticLabels: {font: "11px sans-serif", labels: [0,10,20,30,40,50], color: "#000"},
        staticZones: [
           {strokeStyle: "#87CEFA", min: 0, max: 15},  // Light Blue for Cold
           {strokeStyle: "#90EE90", min: 15, max: 25}, // Light Green for Cool
           {strokeStyle: "#FFD700", min: 25, max: 35}, // Gold for Optimal
           {strokeStyle: "#FFA500", min: 35, max: 45}, // Orange for Warm
           {strokeStyle: "#FF0000", min: 45, max: 50}  // Red for Hot
        ]
    });
    gaugeTemp.minValue = 0; gaugeTemp.maxValue = 50; gaugeTemp.set(0);
    gaugeTemp.setTextField(document.getElementById('gauge-temp-val'));

    // Humidity Gauge
    gaugeHum = new Gauge(document.getElementById('gauge-hum')).setOptions({
        ...baseGaugeOptions,
        staticLabels: {font: "11px sans-serif", labels: [0,25,50,75,100], color: "#000"},
        staticZones: [
           {strokeStyle: "#CD853F", min: 0, max: 30},  // Peru (Dry)
           {strokeStyle: "#9ACD32", min: 30, max: 60}, // YellowGreen (Optimal)
           {strokeStyle: "#87CEEB", min: 60, max: 85}, // SkyBlue (Humid)
           {strokeStyle: "#0000CD", min: 85, max: 100} // MediumBlue (Very Humid)
        ]
    });
    gaugeHum.minValue = 0; gaugeHum.maxValue = 100; gaugeHum.set(0);
    gaugeHum.setTextField(document.getElementById('gauge-hum-val'));

    // Gas Gauge
    gaugeGas = new Gauge(document.getElementById('gauge-gas')).setOptions({
        ...baseGaugeOptions,
        staticLabels: {font: "11px sans-serif", labels: [0,1000,2000,3000,4095], color: "#000"},
        staticZones: [ // Example zones, adjust based on sensor specifics
           {strokeStyle: "#90EE90", min: 0, max: 1000},    // Good
           {strokeStyle: "#FFD700", min: 1000, max: 2500}, // Moderate
           {strokeStyle: "#FF0000", min: 2500, max: 4095}  // High
        ]
    });
    gaugeGas.minValue = 0; gaugeGas.maxValue = 4095; gaugeGas.set(0);
    gaugeGas.setTextField(document.getElementById('gauge-gas-val'));

    // Soil Moisture Gauge
    gaugeSoil = new Gauge(document.getElementById('gauge-soil')).setOptions({
        ...baseGaugeOptions,
        staticLabels: {font: "11px sans-serif", labels: [0,25,50,75,100], color: "#000"},
        staticZones: [
           {strokeStyle: "#D2B48C", min: 0, max: 30},  // Tan (Dry)
           {strokeStyle: "#228B22", min: 30, max: 70}, // ForestGreen (Optimal)
           {strokeStyle: "#1E90FF", min: 70, max: 100} // DodgerBlue (Wet)
        ]
    });
    gaugeSoil.minValue = 0; gaugeSoil.maxValue = 100; gaugeSoil.set(0);
    gaugeSoil.setTextField(document.getElementById('gauge-soil-val'));
}

function updateGaugeValue(gauge, value) {
    if (gauge && value !== undefined && value !== null && !isNaN(value)) {
        gauge.set(parseFloat(value));
    } else if (gauge) {
        // Set to a neutral/default value if data is invalid
        gauge.set(gauge.minValue); // Or some other indicator like 0
    }
}

// --- UI Update Functions ---
function updateUI(data, actionType = 'periodic') { // actionType: 'initial', 'periodic', 'add', 'remove'
    // Update gauges
    updateGaugeValue(gaugeTemp, data.temp);
    updateGaugeValue(gaugeHum, data.humidity);
    updateGaugeValue(gaugeGas, data.gas);
    updateGaugeValue(gaugeSoil, data.soil);

    // Update mode display and Timer tab visibility
    let modeText = '--';
    const timerTabButton = document.getElementById('timer-tab-button');
    if (data.mode !== undefined) {
        const modes = ['AUTO', 'OFF', 'ON', 'HALF', 'TIMER'];
        modeText = modes[data.mode % 5] || 'Unknown';
        if (timerTabButton) {
            if (data.mode === 4) { // Timer mode
                timerTabButton.classList.remove('hidden');
            } else {
                timerTabButton.classList.add('hidden');
                // If Timer tab is active and mode changes, switch to Dashboard
                if (document.getElementById('Timer').style.display === 'block') {
                    openTab(null, 'Dashboard'); // Programmatically open Dashboard
                }
            }
        }
    } else {
        if (timerTabButton) timerTabButton.classList.add('hidden');
    }
    setTextContent('mode', modeText);

    // Update relay statuses
    updateRelayStatus('r1', data.relay1);
    updateRelayStatus('r2', data.relay2);
    updateRelayStatus('r3', data.relay3);
    updateRelayStatus('r4', data.relay4);

    // Update general settings display and input fields (if not focused)
    updateSettingDisplay('min-temp-disp', data.min_temp, 'min-temp-input');
    updateSettingDisplay('max-temp-disp', data.max_temp, 'max-temp-input');
    updateSettingDisplay('delay-disp', data.delay, 'delay-input');
    updateSettingDisplay('hum-limit-disp', data.hum_limit, 'hum-limit-input');
    updateSettingDisplay('gas-limit-disp', data.gas_limit, 'gas-limit-input');

    const modeSelect = document.getElementById('mode-select');
    if (modeSelect && document.activeElement !== modeSelect && data.mode !== undefined) {
        modeSelect.value = data.mode;
    }

    // Update Timer tab specific info
    const forceTimerInputsPopulation = (actionType === 'initial' || actionType === 'add' || actionType === 'remove');
    if (data.mode === 4 || document.getElementById('Timer').style.display === 'block') { // Update if timer mode OR timer tab is visible
        setTextContent('timer-current-time', data.currentTimeStr || '--:--:--');
        setTextContent('timer-start-date', data.startDateStr || '--/--/--');
        setTextContent('timer-days-passed', data.daysPassed !== undefined ? data.daysPassed : '--');
        updateTimerTable(data.timerEntries || [], forceTimerInputsPopulation);
    }

    // Update IP address
    setTextContent('nav-ip', data.ip_address ? `IP: ${data.ip_address}` : 'IP: N/A');

    lastData = data; // Store the latest full data
    if (actionType === 'initial') isInitialPageLoad = false;
}

function updateTimerTable(timerEntries, forceInputsPopulation) {
    const tableBody = document.getElementById("timer-list-table").querySelector('tbody');
    if (!tableBody) return;

    let focusedInputId = null;
    let focusedInputValue = null;
    if (document.activeElement && document.activeElement.classList.contains('timer-input')) {
        focusedInputId = document.activeElement.id;
        focusedInputValue = document.activeElement.value;
    }

    const existingNonFocusedInputValues = {};
    if (!forceInputsPopulation) {
        tableBody.querySelectorAll('.timer-input').forEach(input => {
            if (input.id !== focusedInputId) {
                existingNonFocusedInputValues[input.id] = input.value;
            }
        });
    }

    tableBody.innerHTML = ""; // Clear existing rows

    if (!timerEntries || timerEntries.length === 0) {
        tableBody.innerHTML = '<tr><td colspan="4" style="text-align:center;">No timer schedule entries found.</td></tr>';
        return;
    }

    timerEntries.forEach((entry, index) => {
        const row = tableBody.insertRow();
        row.insertCell().textContent = entry.offset !== undefined ? entry.offset : '?';

        const minInputId = `timer-min-input-${index}`;
        const maxInputId = `timer-max-input-${index}`;

        // Determine the value for the input field
        let minInputValueForField = (entry.min !== undefined ? entry.min : '');
        let maxInputValueForField = (entry.max !== undefined ? entry.max : '');

        if (!forceInputsPopulation) { // Periodic update, try to preserve
            if (minInputId === focusedInputId) {
                minInputValueForField = focusedInputValue;
            } else if (existingNonFocusedInputValues.hasOwnProperty(minInputId)) {
                minInputValueForField = existingNonFocusedInputValues[minInputId];
            }

            if (maxInputId === focusedInputId) {
                maxInputValueForField = focusedInputValue;
            } else if (existingNonFocusedInputValues.hasOwnProperty(maxInputId)) {
                maxInputValueForField = existingNonFocusedInputValues[maxInputId];
            }
        }

        // System value display (span)
        const minSystemValue = entry.min !== undefined ? entry.min : '--';
        const maxSystemValue = entry.max !== undefined ? entry.max : '--';

        const minTempCell = row.insertCell();
        minTempCell.className = 'timer-input-cell';
        minTempCell.innerHTML = `<div>
                                    <span class="timer-value-display" id="timer-min-disp-${index}">${minSystemValue}</span>
                                    <input type="number" class="timer-input" id="${minInputId}" value="${minInputValueForField}" step="1">
                                </div>`;

        const maxTempCell = row.insertCell();
        maxTempCell.className = 'timer-input-cell';
        maxTempCell.innerHTML = `<div>
                                    <span class="timer-value-display" id="timer-max-disp-${index}">${maxSystemValue}</span>
                                    <input type="number" class="timer-input" id="${maxInputId}" value="${maxInputValueForField}" step="1">
                                </div>`;

        const actionCell = row.insertCell();
        actionCell.innerHTML = `<button onclick="saveTimerEntry(${index})" title="Save Entry ${index + 1}"><svg width="1em" height="1em" fill="currentColor"><use xlink:href="#icon-save"></use></svg></button>
                               <button onclick="resetTimerEntry(${index})" title="Reset Entry ${index + 1} to Defaults" style="background-color: #ffc107;"><svg width="1em" height="1em" fill="currentColor"><use xlink:href="#icon-reset"></use></svg></button>`;
    });

    if (focusedInputId) {
        const focusedElement = document.getElementById(focusedInputId);
        if (focusedElement) {
            focusedElement.focus();
            // Restore the exact value being typed, especially important if it was partially typed
            focusedElement.value = focusedInputValue;
            try {
                // Attempt to restore cursor position
                focusedElement.setSelectionRange(focusedInputValue.length, focusedInputValue.length);
            } catch (e) { /* Ignore if not possible (e.g., element not visible yet) */ }
        }
    }
}


function setTextContent(id, text) {
    const el = document.getElementById(id);
    if (el) el.textContent = text !== null && text !== undefined ? text : '--';
}

function updateSettingDisplay(dispId, value, inputId) {
    const dispElement = document.getElementById(dispId);
    if (dispElement) dispElement.textContent = (value !== undefined && value !== null) ? value : '--';

    const inputElement = document.getElementById(inputId);
    // Only update input if it's not the currently focused element
    if (inputElement && document.activeElement !== inputElement) {
        inputElement.value = (value !== undefined && value !== null) ? value : '';
    }
}

function updateRelayStatus(elementId, status) {
    const el = document.getElementById(elementId);
    if (el) {
        if (status !== undefined && status !== null) {
            el.textContent = status ? 'ON' : 'OFF';
            el.className = status ? 'value' : 'value off'; // Assumes 'value' is green and 'value off' is red
        } else {
            el.textContent = '--';
            el.className = 'value warn'; // A class for uncertain/loading state
        }
    }
}

function displayError(message) {
    const errEl = document.getElementById('error-msg');
    if (errEl) {
        errEl.textContent = message;
        errEl.style.display = message ? 'block' : 'none';
        if (message) {
            // Clear error after some time
            setTimeout(() => { displayError(''); }, 7000); // Increased timeout
        }
    }
}

// --- Data Fetching and Command Sending ---
const UPDATE_INTERVAL = 2500; // milliseconds
async function fetchStatus(actionType = 'periodic') {
    try {
        const response = await fetch('/status');
        if (!response.ok) {
            let errorText = `HTTP error! Status: ${response.status}`;
            try {
                // Try to get more specific error text from response body
                errorText = await response.text() || errorText;
            } catch (e) { /* ignore if can't read body */ }
            throw new Error(errorText);
        }
        const data = await response.json();
        updateUI(data, actionType);
        if (actionType !== 'initial') displayError(''); // Clear error on successful subsequent fetches
    } catch (error) {
        console.error('Error fetching status:', error);
        displayError(`Connection Error: ${error.message}. Retrying...`);
        // Optionally, update UI to show a disconnected state more explicitly
        // For example, set all gauge values to min or show 'N/A'
        const offlineData = { temp: null, humidity: null, gas: null, soil: null, mode: undefined, relay1: null, relay2: null, relay3: null, relay4: null, min_temp: null, max_temp: null, delay: null, hum_limit: null, gas_limit: null, ip_address: 'Error', timerEntries: lastData.timerEntries || [], currentTimeStr: '--:--:--', startDateStr: '--/--/--', daysPassed: '--' };
        updateUI(offlineData, actionType); // Update UI with placeholder/error data
    }
}

async function sendCommand(url, isPost = false, params = {}) {
    try {
        displayError(''); // Clear previous errors
        let fullUrl = url;
        const options = {};

        if (isPost) { // Though current ESP32 code uses GET for commands
            options.method = 'POST';
            options.headers = {'Content-Type': 'application/x-www-form-urlencoded'};
            options.body = new URLSearchParams(params).toString();
        } else { // Using GET
            if (Object.keys(params).length > 0) {
                fullUrl += '?' + new URLSearchParams(params).toString();
            }
            options.method = 'GET';
        }

        const response = await fetch(fullUrl, options);
        if (!response.ok) {
            const errorText = await response.text();
            throw new Error(`Command failed: ${response.status} ${errorText || 'No error details'}`);
        }
        const result = await response.text();
        console.log('Command result for', url, ':', result);

        // Determine actionType for fetchStatus based on URL or a new parameter if needed
        let actionForFetch = 'periodic';
        if (url.includes('addtimer') || url.includes('removelasttimer')) {
            actionForFetch = url.includes('addtimer') ? 'add' : 'remove';
        }
        await fetchStatus(actionForFetch); // Fetch status immediately to reflect changes
    } catch (error) {
        console.error('Error sending command:', error);
        displayError(`Command Error: ${error.message}`);
    }
}

// --- Event Handlers / Public Functions ---
window.setMode = function() {
    const selectedMode = document.getElementById('mode-select').value;
    sendCommand(`/setmode`, false, { mode: selectedMode });
}

window.saveAllSettings = function() {
    const minTemp = document.getElementById('min-temp-input').value;
    const maxTemp = document.getElementById('max-temp-input').value;
    const delay = document.getElementById('delay-input').value;
    const humLimit = document.getElementById('hum-limit-input').value;
    const gasLimit = document.getElementById('gas-limit-input').value;

    if (minTemp === '' || maxTemp === '' || delay === '' || humLimit === '' || gasLimit === '') {
        displayError("All setting fields must have values."); return;
    }
    const minVal = parseInt(minTemp); const maxVal = parseInt(maxTemp);
    const delayVal = parseInt(delay); const humVal = parseInt(humLimit); const gasVal = parseInt(gasLimit);

    if (isNaN(minVal) || isNaN(maxVal) || isNaN(delayVal) || isNaN(humVal) || isNaN(gasVal)) {
        displayError("All settings must be valid numbers."); return;
    }
    if (minVal > maxVal) { displayError("Min Temp cannot be greater than Max Temp."); return; }
    if (delayVal < 1 || delayVal > 300) { displayError("Switch Delay must be between 1 and 300."); return; }
    if (humVal < 0 || humVal > 100) { displayError("Humidity Limit must be between 0 and 100."); return; }
    if (gasVal < 0 || gasVal > 4095) { displayError("Gas Limit must be between 0 and 4095."); return; }

    sendCommand(`/setsettings`, false, {min: minVal, max: maxVal, delay: delayVal, hum: humVal, gas: gasVal});
}

window.syncTimeNow = function() { sendCommand('/synctime'); }

window.saveTimerEntry = function(index) {
    const minInput = document.getElementById(`timer-min-input-${index}`);
    const maxInput = document.getElementById(`timer-max-input-${index}`);
    let minValStr = minInput.value.trim();
    let maxValStr = maxInput.value.trim();

    if (minValStr === "") { displayError(`Entry ${index + 1}: Min Temp cannot be empty.`); return; }
    if (maxValStr === "") { displayError(`Entry ${index + 1}: Max Temp cannot be empty.`); return; }
    const minTemp = parseInt(minValStr); const maxTemp = parseInt(maxValStr);
    if (isNaN(minTemp)) { displayError(`Entry ${index + 1}: Min Temp must be a valid number.`); return; }
    if (isNaN(maxTemp)) { displayError(`Entry ${index + 1}: Max Temp must be a valid number.`); return; }
    // Add reasonable temperature range validation if desired, e.g., 0-50
    if (minTemp < 0 || minTemp > 50) { displayError(`Entry ${index + 1}: Min Temp must be between 0 and 50.`); return;}
    if (maxTemp < 0 || maxTemp > 50) { displayError(`Entry ${index + 1}: Max Temp must be between 0 and 50.`); return;}
    if (minTemp > maxTemp) { displayError(`Entry ${index + 1}: Min Temp (${minTemp}) cannot be greater than Max Temp (${maxTemp}).`); return; }

    sendCommand(`/settimerentry`, false, {index: index, min: minTemp, max: maxTemp});
}

window.resetTimerEntry = function(index) {
    if (!confirm(`Are you sure you want to reset Timer Entry ${index + 1} to defaults (28/32)? This will be saved immediately.`)) { return; }
    sendCommand(`/resettimerentry`, false, {index: index});
}

window.addTimerEntry = function() {
    if (!confirm("Are you sure you want to add a new timer entry with default values (Day offset: next available, Temp: 28/32)? This will be saved immediately.")) return;
    sendCommand('/addtimer');
}

window.removeLastTimerEntry = function() {
    if (!confirm("Are you sure you want to remove the last timer entry? This action cannot be undone from the Web UI and will be saved immediately.")) return;
    sendCommand('/removelasttimer');
}

// --- Initialization ---
window.onload = () => {
    // Set default tab
    openTab({currentTarget: document.querySelector('.tablinks.active')}, 'Dashboard'); // Ensure Dashboard is active
    initializeGauges();
    fetchStatus('initial'); // Initial data fetch
    setInterval(() => fetchStatus('periodic'), UPDATE_INTERVAL); // Periodic updates
};
