String.prototype.toTitleCase = function() {
    return this.toLowerCase().replace(/\b\w/g, function(char) {
        return char.toUpperCase();
    });
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
    DESCRIPTORS_18: "descriptors18",
    DESCRIPTORS_11: "descriptors11"
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

function getShadingColor(value, max=1.0, mode=ShadingMode.RELATIVE) {
    switch (mode) {
        case ShadingMode.RELATIVE :
            return value >= max * .90 ? "#260080" :
                   value >= max * .75 ? "#2600BD" :
                   value >= max * .60 ? "#1C1AE3" :
                   value >= max * .45 ? "#2A4EFC" :
                   value >= max * .30 ? "#3C8DFD" :
                   value >= max * .15 ? "#4CB2FE" :
                   value >= max * .00 ? "#A0EDFF" :
                                        "#FFFFFF" ;
        case ShadingMode.RAW :
            return value >= 0.90 ? "#520016" :
                   value >= 0.80 ? "#680020" :
                   value >= 0.70 ? "#800026" :
                   value >= 0.60 ? "#BD0026" :
                   value >= 0.50 ? "#E31A1C" :
                   value >= 0.40 ? "#FC4E2A" :
                   value >= 0.30 ? "#FD8D3C" :
                   value >= 0.20 ? "#FEB24C" :
                   value >= 0.10 ? "#FFEDA0" :
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
    currentShapeMode = mode;
    updateLayerVisibility();
}

function updateViewMode(mode) {
    currentViewMode = mode;
}

function updateShadingMode(mode) {
    currentShadingMode = mode;
}

let selectedLayer = null;

let geoJSONNation = null;
let geoJSONStates = null;
let geoJSONCounties = null;

function loadMapData(map) {
    let nation, states, counties;
    fetch("us-states.json").then(res => res.json()).then(topoData => {
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
            });

            // Create and add layers
            geoJSONNation = L.geoJSON(nation, {style, onEachFeature}).addTo(map);
            geoJSONStates = L.geoJSON(states, {style, onEachFeature});
            geoJSONCounties = L.geoJSON(counties, {style, onEachFeature});
        });
    });

}

function style(feature) {
    if (!currentPrimarySelected) {
        return {
            fillColor: "#cccccc",
            weight: 1,
            opacity: 1,
            color: "#333",
            fillOpacity: 0.6,
            interactive: true
        };
    }
    switch (currentViewMode) {
        case ViewMode.DEMOGRAPHICS :
            break;
        case ViewMode.ELECTORAL :
            break;
        case ViewMode.DESCRIPTORS_18 :
            break;
        case ViewMode.DESCRIPTORS_11 :
            break;
    }
}

function resetLayer(layer) {
    layer.setStyle({
        weight: 1,
        color: "#333",
        fillOpacity: 0.6
    });
}

function highlightLayer(layer) {
    selectedLayer = layer;
    layer.setStyle({
        weight: 3,
        color: "#ff7800",
        fillOpacity: 0.1
    });
    selectedLayer.bringToFront();
}

function onEachFeature(feature, layer) {
    layer.on({
        click: () => {
            if (selectedLayer) resetLayer(selectedLayer);
            highlightLayer(layer);
            console.log(feature);
            displayMapEntityInfo(feature);
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
    console.log(properties);
    const mapInfobox = document.getElementById("map-infobox");
    if (!properties) {
        mapInfobox.innerHTML = `<h3>Click on a state, county, or the nation to see demographic details.</h3>`;
    }
    else {
        mapInfobox.innerHTML = `
            <h2>${properties.name}</h2>
            <h3>Population:</h3> ${properties.population}
            <h3>Demographics:</h3> ${formatDemographics(properties.demographics)}
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

document.addEventListener('DOMContentLoaded', () => {

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
    });

    const resetViewButton = document.getElementById("reset-view");
    resetViewButton.addEventListener('click', resetView);

    map.on('zoomend', () => {
        updateLayerVisibility();
    });

    loadMapData(map);

    displayMapEntityInfo();
});