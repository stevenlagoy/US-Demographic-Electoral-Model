String.prototype.toTitleCase = function() {
    return this.toLowerCase().replace(/\b\w/g, function(char) {
        return char.toUpperCase();
    });
}

String.prototype.addCommas = function() {
    let res = "";
    let i = 0;
    for (const char of this.split('').reverse().join('')) {
        res += char;
        i++;
        if (i % 3 == 0) res += ",";
    }
    res = res.split('').reverse().join('').replace(/^,/, "");
    return res;
}

const ShapeMode = {
    AUTO: "auto",
    NATION: "nation",
    STATE: "state",
    COUNTY: "county"
};
Object.freeze(ShapeMode);
let currentShapeMode = ShapeMode.AUTO;

const ViewMode = {
    DEMOGRAPHICS: "demographics",
    ELECTORAL: "electoral",
    DESCRIPTORS18: "descriptors18",
    DESCRIPTORS11: "descriptors11"
};
Object.freeze(ViewMode);
let currentViewMode = ViewMode.DEMOGRAPHICS;

const ShadingMode = {
    RAW: "raw",
    RELATIVE: "relative",
    COUNT: "count"
};
Object.freeze(ShadingMode);
let currentShadingMode = ShadingMode.RAW;

let currentPrimarySelected = '';

const center = [45, -96];
const defaultZoom = 4;
const map = L.map('map').setView(center, defaultZoom);

function resetView() {
    // Saccade to default position
    map.setView(center, defaultZoom);
    // Select default values
    ["shape-auto", "view-demographics", "shading-raw"].forEach(d => {
        document.getElementById(d).checked = true;
        // This will also clear the primary select value
    });
    currentShapeMode = ShapeMode.AUTO;
    currentViewMode = ViewMode.DEMOGRAPHICS;
    currentShadingMode = ShadingMode.RAW;
    updateLayerVisibility();
    // Clear selected layer
    resetLayer(selectedLayer);
    // Clear the search input
    document.getElementById("feature-search").value = "";
    // Clear the info box(es)
    displayMapEntityInfo();
}

function saccadeTo(FIPS) {
    let match = geoJSONCounties.getLayers().find(layer => layer.feature.id === FIPS);
    if (!match) match = geoJSONStates.getLayers().find(layer => layer.feature.id === FIPS);
    if (match) {
        map.fitBounds(match.getBounds());
        resetLayer(selectedLayer);
        highlightLayer(match);
    } 
}

function getShadingColor(value, max=1.0) {
    switch (currentShadingMode) {
        case ShadingMode.RAW :
            return value >= max * .90 ? "#520016" :
                   value >= max * .80 ? "#680020" :
                   value >= max * .70 ? "#800026" :
                   value >= max * .60 ? "#BD0026" :
                   value >= max * .50 ? "#E31A1C" :
                   value >= max * .40 ? "#FC4E2A" :
                   value >= max * .30 ? "#FD8D3C" :
                   value >= max * .20 ? "#FEB24C" :
                   value >= max * .10 ? "#FFEDA0" :
                                   "#FFFFFF" ;
        case ShadingMode.RELATIVE :
            return value >= max * .90 ? "#260080" :
                   value >= max * .75 ? "#2600BD" :
                   value >= max * .60 ? "#1C1AE3" :
                   value >= max * .45 ? "#2A4EFC" :
                   value >= max * .30 ? "#3C8DFD" :
                   value >= max * .15 ? "#4CB2FE" :
                   value >= max * .00 ? "#A0EDFF" :
                                        "#FFFFFF" ;
        case ShadingMode.COUNT : 
            return value > max * 0.95 ? "#003000" :
                   value > max * 0.90 ? "#004000" :
                   value > max * 0.85 ? "#005000" :
                   value > max * 0.80 ? "#006000" :
                   value > max * 0.75 ? "#007000" :
                   value > max * 0.70 ? "#008000" :
                   value > max * 0.65 ? "#009000" :
                   value > max * 0.60 ? "#00A000" :
                   value > max * 0.55 ? "#00B000" :
                   value > max * 0.50 ? "#00C000" :
                   value > max * 0.45 ? "#00D000" :
                   value > max * 0.40 ? "#00E000" :
                   value > max * 0.35 ? "#00F000" :
                   value > max * 0.30 ? "#20FF20" :
                   value > max * 0.25 ? "#40FF40" :
                   value > max * 0.20 ? "#60FF60" :
                   value > max * 0.15 ? "#80FF80" :
                   value > max * 0.10 ? "#A0FFA0" :
                   value > max * 0.05 ? "#C0FFC0" :
                                        "#FFFFFF" ;
    }
}

function getPartyColor(democratic, republican, year) {
    // Democratic is % or # votes for democratic candidate, Repubican is % or # votes for republican candidate
    
    let total, margin;

    switch (currentShadingMode) {
        case ShadingMode.RAW :
            total = democratic + republican;
            democratic /= total;
            republican /= total;
            margin = republican - democratic;
            return margin >  0.75 ? "#F00000" :
                   margin >  0.50 ? "#FF4040" :
                   margin >  0.25 ? "#FF8080" :
                   margin >  0.00 ? "#FFC0C0" :
                   margin === 0.0 ? "#FFFFFF" :
                   margin > -0.25 ? "#C0C0FF" :
                   margin > -0.50 ? "#8080FF" :
                   margin > -0.75 ? "#4040FF" :
                                    "#0000FF" ;
        case ShadingMode.RELATIVE :
            total = democratic + republican;
            democratic /= total;
            republican /= total;
            margin = republican - democratic;
            let nationDemocratic = nation.features[0].electoralData[year].find(r => r.party === "DEMOCRAT").votes;
            let nationRepublican = nation.features[0].electoralData[year].find(r => r.party === "REPUBLICAN").votes;
            const nationTotal = nationDemocratic + nationRepublican;
            nationDemocratic /= nationTotal;
            nationRepublican /= nationTotal;
            const nationMargin = nationRepublican - nationDemocratic;

            const diff = margin - nationMargin;
            return diff >  0.75 ? "#F00000" :
                   diff >  0.50 ? "#FF4040" :
                   diff >  0.25 ? "#FF8080" :
                   diff >  0.00 ? "#FFC0C0" :
                   diff === 0.0 ? "#FFFFFF" :
                   diff > -0.25 ? "#C0C0FF" :
                   diff > -0.50 ? "#8080FF" :
                   diff > -0.75 ? "#4040FF" :
                                  "#0000FF" ;
        case ShadingMode.COUNT :
            break;
    }
}

function getDescriptorCloseness(descriptors1, descriptors2) {
    if (!descriptors1 || !descriptors2) return 0.0;
    let closeness1 = 0.0, closeness2 = 0.0;
    descriptors1.forEach(d => {
        if (descriptors2.includes(d)) {
            closeness1 += (1/ descriptors1.length);
        }
    });
    descriptors2.forEach(d => {
        if (descriptors1.includes(d)) {
            closeness2 += (1 / descriptors2.length);
        }
    });
    return (closeness1 + closeness2) / 2;
}

function updateShapeMode(mode) {
    const shapeModeInput = document.getElementById(`shape-${mode}`);
    shapeModeInput.checked = true;
    currentShapeMode = mode;
    updateLayerVisibility();
    refreshStyles();
}

let primarySelectOptions = {};
async function updateViewMode(mode) {
    const viewModeInput = document.getElementById(`view-${mode}`);
    viewModeInput.checked = true;
    currentViewMode = mode;
    const primarySelect = document.getElementById("primary-select");
    if (primarySelectOptions[mode]) { // Lazy load the options
        primarySelect.innerHTML = primarySelectOptions[mode];
        refreshStyles();
        if (mode === ViewMode.DESCRIPTORS18) displayDescriptorInfo(null);
        else displayMapEntityInfo(null);
        return;
    }
    switch (mode) {
        case ViewMode.DEMOGRAPHICS :
            primarySelect.innerHTML = `<option value="">--Select a Demographic--</option>`;
            const demographics = nation.features[0].demographics;
            for (const category in demographics)    
                for (const demographic in demographics[category]) {
                    primarySelect.innerHTML += `
                        <option class="${category.replace(/_/g, "-")}" value="${category}:${demographic}">${category.replace(/_/g, " ").toTitleCase()}: ${demographic}</option>
                    `;
                }
            primarySelectOptions[mode] = primarySelect.innerHTML;
            displayMapEntityInfo(null);
            break;
        case ViewMode.ELECTORAL :
            await loadElectoralData();
            primarySelect.innerHTML = '';
            const elections = Object.keys(counties.features[0].electoralData);
            let mostRecent = elections[0];
            for (const election of elections) {
                primarySelect.innerHTML += `
                    <option value="${election}">${election} Election</option>
                `;
                if (election > mostRecent) mostRecent = election;
            }
            primarySelect.value = mostRecent;
            primarySelect.dispatchEvent(new Event('change')); // Force update layer styles
            primarySelectOptions[mode] = primarySelect.innerHTML;
            displayMapEntityInfo(null);
            break;
        case ViewMode.DESCRIPTORS18 :
            await loadDescriptorData();
            primarySelect.innerHTML = `<option value="">--Select a Descriptor--</option>`;
            for (const descriptor of descriptors) {
                primarySelect.innerHTML += `
                    <option value="${descriptor.name}">${descriptor.name}</option>
                `;
            }
            primarySelectOptions[mode] = primarySelect.innerHTML;
            displayDescriptorInfo(null);
            break;
        case ViewMode.DESCRIPTORS11 :
            break;
    }
    refreshStyles();
}

function updateShadingMode(mode) {
    const shadingModeInput = document.getElementById(`shading-${mode}`);
    shadingModeInput.checked = true;
    currentShadingMode = mode;
    refreshStyles();
}

let selectedLayer = null;

let geoJSONNation = null;
let nation = null;
let geoJSONStates = null;
let states = null;
let geoJSONCounties = null;
let counties = null;

async function loadMapData(map) {
    await fetch("us-states.json").then(res => res.json()).then(topoData => {
        nation = topojson.feature(topoData, topoData.objects.nation);
        states = topojson.feature(topoData, topoData.objects.states);
        counties = topojson.feature(topoData, topoData.objects.counties);
        
        return fetch("mapdata.json").then(res => res.json()).then(mapData => {
            // Attach peroperties to Nation
            nation.features.forEach(f => {
                const nationData = mapData['nation'];
                f.name = nationData.name;
                f.population = nationData.population;
                f.demographics = nationData.demographics;
            });
    
            // Attach properties to States
            states.features.forEach(f => {
                const stateData = mapData[f.id];
                if (!stateData) {
                    console.log(`No state found with FIPS: ${f.id}. It may be a US territory.`);
                    return;
                }
                f.name = stateData.name;
                f.population = stateData.population;
                f.demographics = stateData.demographics;
            });
    
            // Attach properties to Counties
            counties.features.forEach(f => {
                const countyData = mapData[f.id];
                if (!countyData) {
                    console.log(`No county found with FIPS: ${f.id}. It may be a US territory.`);
                    return;
                }
                f.name = countyData.name;
                f.population = countyData.population;
                f.demographics = countyData.demographics;
                f.state = countyData.state;
            });

            // Create and add layers
            geoJSONNation = L.geoJSON(nation, {style, onEachFeature}).addTo(map);
            geoJSONStates = L.geoJSON(states, {style, onEachFeature});
            geoJSONCounties = L.geoJSON(counties, {style, onEachFeature});
        });
    });
}

async function loadElectoralData(map) {
    await fetch("elections.json").then(res => res.json()).then(electoralData => {
        for (const FIPS in electoralData) {
            const data = electoralData[FIPS];
            const county = counties.features.find(c => c.id === FIPS);
            const state = states.features.find(s => s.id === FIPS);
            if (county) {
                county.electoralData = data;
            }
            else if (state) {
                state.electoralData = data;
            }
            else {
                console.log(`No county or state found with FIPS: ${FIPS}.`);
                continue;
            }
        }
    });
    let nationResults = {};
    for (const state of states.features) {
        let stateResults = {};
        if (!state.name) continue;
        for (const county of counties.features) {
            if (state.name && county.state && state.name === county.state) {
                for (const year in county.electoralData) {
                    if (!stateResults[year]) stateResults[year] = Array();
                    if (!nationResults[year]) nationResults[year] = Array();
                    for (const res of county.electoralData[year]) {
                        const stateMatch = stateResults[year].find(r => r.candidate == res.candidate);
                        if (stateMatch) stateMatch.votes += res.votes;
                        else stateResults[year].push(res);
                        const nationMatch = nationResults[year].find(r => r.candidate == res.candidate);
                        if (nationMatch) nationMatch.votes += res.votes;
                        else nationResults[year].push(res);
                    }
                }
            }
        }
        state.electoralData = stateResults;
    }
    nation.features[0].electoralData = nationResults;
}

const descriptors = [];

async function loadDescriptorData(map) {
    await fetch("descriptors_18.json").then(res => res.json()).then(descriptorsData => {
        console.log(descriptorsData);
        for (const c in descriptorsData.counties) {
            const countyData = descriptorsData.counties[c]
            const county = counties.features.find(f => f.id === countyData.FIPS);
            county.descriptors = countyData.descriptors;
        }
        for (const d in descriptorsData.descriptors) {
            const descriptorData = descriptorsData.descriptors[d];
            descriptors.push(descriptorData);
            const newEnglandStates = ["Maine", "New Hampshire", "Vermont", "Massachusetts", "Rhode Island", "Connecticut"];
            const middleAtlanticStates = ["New York", "Pennsylvania", "New Jersey"];
            const southAtlanticStates = ["Delaware", "Maryland", "District of Columbia", "West Virginia", "Virginia", "North Carolina", "South Carolina", "Georgia", "Florida"];
            const eastSouthCentralStates = ["Kentucky", "Tennessee", "Alabama", "Mississippi"];
            const westSouthCentralStates = ["Arkansas", "Louisiana", "Oklahoma", "Texas"];
            const eastNorthCentralStates = ["Ohio", "Michigan", "Indiana", "Illinois", "Wisconsin"];
            const westNorthCentralStates = ["Minnesota", "Iowa", "Missouri", "North Dakota", "South Dakota", "Nebraska", "Kansas"];
            const mountainStates = ["Montana", "Idaho", "Wyoming", "Nevada", "Utah", "Colorado", "Arizona", "New Mexico"];
            const pacificStates = ["Washington", "Oregon", "California", "Alaska", "Hawaii"];
            if (descriptorData.name.includes("$$$$")) {
                nation.features[0].descriptors = [descriptorData.name];
                for (const state of states.features) {
                    if (!state.descriptors) state.descriptors = [];
                    state.descriptors.push(descriptorData.name);
                }
            }
            else if (descriptorData.name.includes("$$$")) {
                let statesNames = [];
                switch (descriptorData.name) {
                    case "$$$MIDWEST" :
                        statesNames = [...eastNorthCentralStates, ...westNorthCentralStates];
                        break;
                    case "$$$NORTHEAST" :
                        statesNames = [...newEnglandStates, ...middleAtlanticStates];
                        break;
                    case "$$$SOUTH" :
                        statesNames = [...southAtlanticStates, ...eastSouthCentralStates, ...westSouthCentralStates];
                        break;
                    case "$$$WEST" :
                        statesNames = [...mountainStates, ...pacificStates];
                        break;
                    default :
                        console.log(`Unrecognized census-region-level descriptor name: ${descriptorData.name}.`);
                }
                for (const stateName of statesNames) {
                    const state = states.features.find(s => s.name === stateName);
                    if (!state.descriptors) state.descriptors = [];
                    state.descriptors.push(descriptorData.name);
                }
            }
            else if (descriptorData.name.includes("$$")) {
                let statesNames = [];
                switch (descriptorData.name) {
                    case "$$EAST_NORTH_CENTRAL" :
                        statesNames = eastNorthCentralStates;
                        break;
                    case "$$EAST_SOUTH_CENTRAL" :
                        statesNames = eastSouthCentralStates;
                        break;
                    case "$$MID_ATLANTIC" :
                        statesNames = middleAtlanticStates;
                        break;
                    case "$$MOUNTAIN" :
                        statesNames = mountainStates;
                        break;
                    case "$$NEW_ENGLAND" :
                        statesNames = newEnglandStates;
                        break;
                    case "$$PACIFIC" :
                        statesNames = pacificStates;
                        break;
                    case "$$SOUTH_ATLANTIC" :
                        statesNames = southAtlanticStates;
                        break;
                    case "$$WEST_NORTH_CENTRAL" :
                        statesNames = westNorthCentralStates;
                        break;
                    case "$$WEST_SOUTH_CENTRAL" :
                        statesNames = westSouthCentralStates;
                        break;
                    default :
                        console.log(`Unrecognized census-division-level descriptor name: ${descriptorData.name}.`);
                }
                for (const stateName of statesNames) {
                    const state = states.features.find(s => s.name === stateName);
                    if (!state.descriptors) state.descriptors = [];
                    state.descriptors.push(descriptorData.name);
                }
            }
            else if (descriptorData.name.includes("$")) {
                const state = states.features.find(s => `$${stateAbbrs[s.name]}` == descriptorData.name);
                if (!state.descriptors) state.descriptors = [];
                state.descriptors.push(descriptorData.name);
            }
        }
    });
}

const maxDemographicValues = {}; // Cache each demographic's maximum value among counties

function style(feature) {
    const blankStyle = {
        fillColor: "#cccccc",
        weight: 1,
        opacity: 1,
        color: "#333",
        fillOpacity: 0.6,
        interactive: true
    };
    if (!currentPrimarySelected) return blankStyle; 
    switch (currentViewMode) {
        case ViewMode.DEMOGRAPHICS :
            const [selectedDemoCategory, selectedDemographic] = currentPrimarySelected?.split(":");
            if (!feature.demographics) return blankStyle;
            let highestPercent = maxDemographicValues[currentPrimarySelected];
            if (!highestPercent) {
                const ranked = counties.features.sort((a, b) => {
                    return (b.demographics?.[selectedDemoCategory]?.[selectedDemographic] || 0.0) - (a.demographics?.[selectedDemoCategory]?.[selectedDemographic] || 0.0)
                });
                highestPercent = ranked[0].demographics[selectedDemoCategory][selectedDemographic];
            }
            maxDemographicValues[currentPrimarySelected] = highestPercent;
            return {
                fillColor: getShadingColor(feature.demographics[selectedDemoCategory]?.[selectedDemographic] || 0, highestPercent),
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true
            };
        case ViewMode.ELECTORAL :
            if (!feature.electoralData) return blankStyle;
            rep_votes = feature.electoralData[currentPrimarySelected]?.find(r => r.party == "REPUBLICAN")?.votes;
            dem_votes = feature.electoralData[currentPrimarySelected]?.find(r => r.party == "DEMOCRAT")?.votes;
            return {
                fillColor: getPartyColor(dem_votes, rep_votes, currentPrimarySelected),
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true
            };
        case ViewMode.DESCRIPTORS18 :
            if (!feature.descriptors) return blankStyle;
            if (feature.descriptors.find(d => d == currentPrimarySelected)) { // Is member
                return {
                    fillColor: "#6060ff",
                    weight: 1,
                    opacity: 1,
                    color: "#333",
                    fillOpacity: 0.6,
                    interactive: true
                };
            }
            return blankStyle;
        case ViewMode.DESCRIPTORS11 :
            break;
    }
}

function resetLayer(layer) {
    if (!layer) return;
    layer.setStyle({
        weight: 1,
        color: "#333",
        fillOpacity: 0.6
    });
}

function refreshStyles() {
    const refreshAll = true;
    if (map.hasLayer(geoJSONNation) || refreshAll) geoJSONNation.setStyle(style);
    if (map.hasLayer(geoJSONStates) || refreshAll) geoJSONStates.setStyle(style);
    if (map.hasLayer(geoJSONCounties) || refreshAll) geoJSONCounties.setStyle(style);
}

function highlightLayer(layer) {
    selectedLayer = layer;
    layer.setStyle({
        weight: 3,
        color: "#ff7800",
    });
    selectedLayer.bringToFront();
}

function onEachFeature(feature, layer) {
    layer.on({
        click: () => {
            if (selectedLayer) resetLayer(selectedLayer);
            highlightLayer(layer);
            if (currentViewMode !== ViewMode.DESCRIPTORS18) displayMapEntityInfo(feature);
        },
        mouseover: () => {
            if (layer !== selectedLayer)
                layer.setStyle({ weight: 4, color: "#555" });
        },
        mouseout: () => {
            if (layer !== selectedLayer)
                geoJSONCounties.resetStyle(layer);
        }
    });
}

function updateLayerVisibility() {
    const zoom = map.getZoom();
    if ((currentShapeMode === ShapeMode.AUTO && zoom <= 4) || currentShapeMode === ShapeMode.NATION) {
        map.addLayer(geoJSONNation);
        map.removeLayer(geoJSONStates);
        map.removeLayer(geoJSONCounties);
    }
    else if ((currentShapeMode === ShapeMode.AUTO && zoom <= 6) || currentShapeMode === ShapeMode.STATE) {
        map.removeLayer(geoJSONNation);
        map.addLayer(geoJSONStates);
        map.removeLayer(geoJSONCounties);
    }
    else {
        map.removeLayer(geoJSONNation);
        map.removeLayer(geoJSONStates);
        map.addLayer(geoJSONCounties);
    }
}

function displayMapEntityInfo(properties) {
    const mapInfobox = document.getElementById("map-infobox");
    if (!properties) {
        mapInfobox.innerHTML = `<h3>Click on a state, county, or the nation to see details.</h3>`;
    }
    else {
        mapInfobox.innerHTML = `
            <h2>${properties.name}</h2>
            ${properties.state ? `<h3>${properties.state}</h3>` : ""}
            ${properties.id ? `<h4>FIPS: ${properties.id}</h4>` : ""}
            <h3>Population:</h3>${properties.population.toString().addCommas()} (2020 census)
        `;
        switch(currentViewMode) {
            case ViewMode.DEMOGRAPHICS :
                mapInfobox.innerHTML += `<h3>Demographics:</h3> ${formatDemographics(properties.demographics)}`;
                break;
            case ViewMode.ELECTORAL :
                mapInfobox.innerHTML += `<h3>Electoral History:</h3> ${formatElectoralData(properties.electoralData, properties.population)}`;
                break;
        }
    }
}

function displayDescriptorInfo(descriptor) {
    const mapInfobox = document.getElementById("map-infobox");
    if (!descriptor) {
        mapInfobox.innerHTML = `<h3>Select a descriptor to see details.</h3>`;
    }
    else {
        console.log(descriptor);
        mapInfobox.innerHTML = `
            <h2>${descriptor.name}</h2>
            <h3>Demographics:</h3> ${formatDescriptorDemographics(descriptor.demographics)}
            <h3>Members:</h3>
            <p># members: ${descriptor.number_members}</p>
            ${formatDescriptorMembers(descriptor.members)}
        `;
    }
}

function formatDemographics(demographics) {
    let html = '';
    for (const category in demographics) {
        const values = demographics[category]
        html += `<h4>${category.toTitleCase().replace(/_/g, " ")}</h4><ul>`;
        for (const key in values) {
            if (typeof values[key] === 'object') {
                html += `<li>${key}:<ul>`;
                for (const subkey in values[key]) {
                    const val = values[key][subkey]
                    if (val !== 0.0)
                        html += `<li>${subkey}: ${typeof val === 'number' ? val.toFixed(5) : val}</li>`;
                }
                html += `</ul></li>`
            }
            else {
                const val = values[key]
                if (val !== 0.0)
                    html += `<li>${key}: ${typeof val === 'number' ? val.toFixed(5) : val}</li>`;
            }
        }
        html += `</ul>`;
    }
    return html;
}

function formatDescriptorDemographics(descriptorDemographics) {
    let html = `<ul>`;
    for (const demographic of descriptorDemographics) {
        html += `<li><b>${demographic.name}</b>: ${demographic.value.toFixed(5)}</li>`;
    }
    html += `</ul>`;
    return html;
}

function formatDescriptorMembers(members) {
    let html = `<ul>`;
    for (const member of members) {
        const county = counties.features.find(c => c.id === member);
        html += `<li>[${member}] ${county.name}, ${county.state}</li>`;
    }
    html += `</ul>`;
    return html;
}

function formatElectoralData(electoralData, population) {
    let html = '';
    for (const year in electoralData) {
        data = electoralData[year];
        totalVotes = data.reduce((acc, cur) => acc + parseInt(cur.votes, 10), 0);
        html += `
            <h4>${year} Election</h4>
            <p>Total Votes: ${totalVotes.toString().addCommas()}</p>
            <p>Turnout (based on 2020 population): ${(totalVotes / population * 100).toFixed(1)}%</p>
            <table><thead><tr>
                <th>Candidate</th>
                <th>Party</th>
                <th>Votes</th>
                <th>%</th>
            </tr></thead><tbody>
        `;
        for (const result in data) {
            if (data[result].votes === 0) continue;
            html += `<tr class=${data[result].party.toLowerCase()}>
                <td>${data[result].candidate.toTitleCase()}</td>
                <td>${data[result].party.toTitleCase()}</td>
                <td>${data[result].votes.toString().addCommas()}</td>
                <td>${(parseInt(data[result].votes, 10) / totalVotes * 100).toFixed(1)}%</td>
            </tr>`
        }
        html += `</tbody></table>`;
    }
    return html;
}

document.addEventListener('DOMContentLoaded', async () => {

    // Basemap
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 10
    }).addTo(map);

    window.addEventListener('resize', () => map.invalidateSize());

    const shapeModeSelect = document.getElementById("shape-mode");
    shapeModeSelect.addEventListener('change', event => {
        const selectedMode = ShapeMode[event.target.value];
        if (!selectedMode) {
            console.error(`Failed to convert selected shape-mode to a known ShapeMode: "${event.target.value}"`);
            selectedMode = ShapeMode.AUTO;
        }
        updateShapeMode(selectedMode);
    });

    const viewModeSelect = document.getElementById("view-mode");
    viewModeSelect.addEventListener('change', event => {
        const selectedMode = ViewMode[event.target.value];
        if (!selectedMode) {
            console.error(`Failed to convert selected view-mode to a known ViewMode: "${event.target.value}"`);
            selectedMode = ViewMode.DEMOGRAPHICS;
        }
        updateViewMode(selectedMode);
    });

    const shadingModeSelect = document.getElementById("shading-mode");
    shadingModeSelect.addEventListener('change', event => {
        const selectedMode = ShadingMode[event.target.value];
        if (!selectedMode) {
            console.error(`Failed to convert selected shading-mode to a known ShadingMode: "${event.target.value}"`);
            selectedMode = ShadingMode.RAW;
        }
        updateShadingMode(selectedMode);
    });

    const primarySelect = document.getElementById("primary-select");
    primarySelect.addEventListener('change', event => {
        currentPrimarySelected = event.target.value;
        refreshStyles();
        updateLayerVisibility();
        if (currentViewMode === ViewMode.DESCRIPTORS18) {
            const descriptor = descriptors.find(d => d.name === currentPrimarySelected);
            displayDescriptorInfo(descriptor);
        }
    });

    const resetViewButton = document.getElementById("reset-view");
    resetViewButton.addEventListener('click', resetView);

    map.on('zoomend', () => {
        updateLayerVisibility();
    });

    await loadMapData(map);
    
    displayMapEntityInfo();

    updateViewMode(currentViewMode);

    const featureSearchInput = document.getElementById("feature-search-input");
    document.getElementById("feature-search").addEventListener('submit', e => {
        e.preventDefault(); // Prevent page reloading when user presses `enter`
        const query = featureSearchInput.value.trim().replaceAll(/[.,'-\s]/g,"").toLowerCase();
        let bestMatch = states.features.find(s => s.name?.replaceAll(/[.,'-\s]/g, "").toLowerCase().includes(query));
        if (bestMatch) {
            featureSearchInput.value = `${bestMatch.name}`;
            updateShapeMode(ShapeMode.STATE);
        }
        else {
            bestMatch = counties.features.find(c => `${c.name?.replaceAll(/[.,'-\s]/g, "")}, ${c.state}`.toLowerCase().includes(query));
            if (!bestMatch) return;
            featureSearchInput.value = `${bestMatch.name}, ${bestMatch.state}`;
            updateShapeMode(ShapeMode.COUNTY);
        }
        document.getElementById("search-suggestions").innerHTML = '';
        saccadeTo(bestMatch.id);
    });
    const searchSuggestionsBox = document.getElementById("search-suggestions");
    featureSearchInput.addEventListener("input", e => {
        const query = featureSearchInput.value.trim().replaceAll(/[.,'-\s]/g,"").toLowerCase();
        searchSuggestionsBox.innerHTML = "";
        if (!query) {
            searchSuggestionsBox.style.display = "none";
            return;
        }

        const matches = [
            ...states.features.filter(s => s.name?.replaceAll(/[.,'-\s]/g, "").toLowerCase().includes(query)),
            ...counties.features.filter(c => `${c.name?.replaceAll(/[.,'-\s]/g, "")}, ${c.state}`.toLowerCase().includes(query)),
        ].slice(0, 10); // Limit to 10 results

        if (matches.length === 0) {
            searchSuggestionsBox.style.display = "none";
            return;
        }

        matches.forEach(match => {
            const div = document.createElement("div");
            div.textContent = match.state ?
                `${match.name}, ${match.state}` : // county
                `${match.name}`; // state
            div.addEventListener("click", () => {
                if (match.state) { // Match is a county
                    featureSearchInput.value = `${match.name}, ${match.state}`;
                    updateShapeMode(ShapeMode.COUNTY);
                }
                else { // Match is a state
                    featureSearchInput.value = `${match.name}`;
                    updateShapeMode(ShapeMode.STATE);
                }
                searchSuggestionsBox.style.display = "none";
                saccadeTo(match.id);
            });
            searchSuggestionsBox.appendChild(div);
        });
        searchSuggestionsBox.style.display = "block";
    });
});

const stateAbbrs = {
    'Alabama': 'AL',
    'Alaska': 'AK',
    'American Samoa': 'AS',
    'Arizona': 'AZ',
    'Arkansas': 'AR',
    'California': 'CA',
    'Colorado': 'CO',
    'Connecticut': 'CT',
    'Delaware': 'DE',
    'District of Columbia': 'DC',
    'States of Micronesia': 'FM',
    'Florida': 'FL',
    'Georgia': 'GA',
    'Guam': 'GU',
    'Hawaii': 'HI',
    'Idaho': 'ID',
    'Illinois': 'IL',
    'Indiana': 'IN',
    'Iowa': 'IA',
    'Kansas': 'KS',
    'Kentucky': 'KY',
    'Louisiana': 'LA',
    'Maine': 'ME',
    'Marshall Islands': 'MH',
    'Maryland': 'MD',
    'Massachusetts': 'MA',
    'Michigan': 'MI',
    'Minnesota': 'MN',
    'Mississippi': 'MS',
    'Missouri': 'MO',
    'Montana': 'MT',
    'Nebraska': 'NE',
    'Nevada': 'NV',
    'New Hampshire': 'NH',
    'New Jersey': 'NJ',
    'New Mexico': 'NM',
    'New York': 'NY',
    'North Carolina': 'NC',
    'North Dakota': 'ND',
    'Northern Mariana Islands': 'MP',
    'Ohio': 'OH',
    'Oklahoma': 'OK',
    'Oregon': 'OR',
    'Palau': 'PW',
    'Pennsylvania': 'PA',
    'Puerto Rico': 'PR',
    'Rhode Island': 'RI',
    'South Carolina': 'SC',
    'South Dakota': 'SD',
    'Tennessee': 'TN',
    'Texas': 'TX',
    'Utah': 'UT',
    'Vermont': 'VT',
    'Virgin Islands': 'VI',
    'Virginia': 'VA',
    'Washington': 'WA',
    'West Virginia': 'WV',
    'Wisconsin': 'WI',
    'Wyoming': 'WY',
}