const stateLookup = {
    "01": "alabama",
    "02": "alaska",
    "04": "arizona",
    "05": "arkansas",
    "06": "california",
    "08": "colorado",
    "09": "connecticut",
    "10": "delaware",
    "11": "district_of_columbia",
    "12": "florida",
    "13": "georgia",
    "15": "hawaii",
    "16": "idaho",
    "17": "illinois",
    "18": "indiana",
    "19": "iowa",
    "20": "kansas",
    "21": "kentucky",
    "22": "louisiana",
    "23": "maine",
    "24": "maryland",
    "25": "massachusetts",
    "26": "michigan",
    "27": "minnesota",
    "28": "mississippi",
    "29": "missouri",
    "30": "montana",
    "31": "nebraska",
    "32": "nevada",
    "33": "new_hampshire",
    "34": "new_jersey",
    "35": "new_mexico",
    "36": "new_york",
    "37": "north_carolina",
    "38": "north_dakota",
    "39": "ohio",
    "40": "oklahoma",
    "41": "oregon",
    "42": "pennsylvania",
    "44": "rhode_island",
    "45": "south_carolina",
    "46": "south_dakota",
    "47": "tennessee",
    "48": "texas",
    "49": "utah",
    "50": "vermont",
    "51": "virginia",
    "53": "washington",
    "54": "west_virginia",
    "55": "wisconsin",
    "56": "wyoming"
};

function toTitleCase(str) {
  if (!str) {
    return "";
  }
  return str.toLowerCase().replace(/\b\w/g, function(char) {
    return char.toUpperCase();
  });
}

document.addEventListener('DOMContentLoaded', () => {

    const center = [45, -96];
    const map = L.map('map').setView(center, 4);

    let descriptorsObject = null;

    // Basemap
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 10
    }).addTo(map);

    let selectedLayer = null; // Track highlighted area

    let geojsonCounties = null;
    let geojsonNation = null;
    let geojsonStates = null;

    let shadingMode = "raw";

    let nationalAverages = {};

    let selectedDemographic = null;
    let selectedDescriptor = null;

    map.on("zoomend", () => {
        updateLayerVisibility();

        if (geojsonCounties) geojsonCounties.resetStyle();
        if (geojsonStates) geojsonStates.resetStyle();
        if (geojsonNation) geojsonNation.resetStyle();

        if (selectedLayer) {
            selectedLayer.setStyle({
                weight: 3,
                color: "#ff7800",
                fillOpacity: 0.5
            });
            selectedLayer.bringToFront();
        }
    
    });

    function updateLayerVisibility() {
        const zoom = map.getZoom();
        console.log(zoom);

        if (descriptorsObject) { // Make all counties visible when descriptors loaded
            if (geojsonNation && map.hasLayer(geojsonNation)) map.removeLayer(geojsonNation);
            if (geojsonStates && map.hasLayer(geojsonStates)) map.removeLayer(geojsonStates);
            if (geojsonCounties && !map.hasLayer(geojsonCounties)) map.addLayer(geojsonCounties);
        }
        else if (zoom <= 4) {
            if (geojsonNation && !map.hasLayer(geojsonNation)) map.addLayer(geojsonNation);
            if (geojsonStates && map.hasLayer(geojsonStates)) map.removeLayer(geojsonStates);
            if (geojsonCounties && map.hasLayer(geojsonCounties)) map.removeLayer(geojsonCounties);
        }
        else if (zoom <= 5) {
            if (geojsonNation && map.hasLayer(geojsonNation)) map.removeLayer(geojsonNation);
            if (geojsonStates && !map.hasLayer(geojsonStates)) map.addLayer(geojsonStates);
            if (geojsonCounties && map.hasLayer(geojsonCounties)) map.removeLayer(geojsonCounties);
        }
        else {
            if (geojsonNation && map.hasLayer(geojsonNation)) map.removeLayer(geojsonNation);
            if (geojsonStates && map.hasLayer(geojsonStates)) map.removeLayer(geojsonStates);
            if (geojsonCounties && !map.hasLayer(geojsonCounties)) map.addLayer(geojsonCounties);
        }
    }

    document.getElementById("scale-select").addEventListener("change", e => {
        shadingMode = e.target.value;
        if (geojsonCounties) geojsonCounties.setStyle(style);

        const legendDiv = document.querySelector('.legend');
        if (legendDiv) updateLegend(legendDiv);
    });

    let stateIndex;
    let countyIndex;

    window.addEventListener("resize", () => map.invalidateSize());

    fetch("counties.json")
        .then(res => res.json())
        .then(countyData => {

            // Compute national averages
            fetch("nation.json")
                .then(res => res.json())
                .then(data => {
                    nationalAverages = data.demographics;
                    // Load geometries
                    fetch("us-states.json")
                        .then(res => res.json())
                        .then(topoData => {
                            const counties = topojson.feature(topoData, topoData.objects.counties);
                            const states = topojson.feature(topoData, topoData.objects.states);
                            const nation = topojson.feature(topoData, topoData.objects.nation);

                            // Attach demographics to nation
                            nation.features.forEach(f => {
                                f.properties.demographics = nationalAverages;
                                f.properties.population = Object.values(countyData)
                                    .reduce((sum, c) => sum + (c.population || 0), 0);
                                f.properties.name = "United States";
                            });

                            // Attach demographics to states
                            states.features.forEach(f => {
                                try {
                                    const sid = f.id;
                                    const stateName = stateLookup[sid];
                                    const stateData = countyData[sid]; // if you store state-level data in counties.json, otherwise fetch separately
                                    if (stateData) {
                                        f.properties.demographics = stateData.demographics;
                                        f.properties.population = stateData.population;
                                        f.properties.name = toTitleCase(stateName.replace(/_/g, " "));
                                    } else {
                                        f.properties.demographics = nationalAverages; // fallback
                                        f.properties.population = 0;
                                        f.properties.name = toTitleCase(stateName.replace(/_/g, " "));
                                    }
                                }
                                catch (err) {
                                    console.error(`Could not read data for state with ID = ${f.id} (is it a territory?):`, err);
                                }
                            });

                            // Attach demographics to counties
                            counties.features.forEach(f => {
                                f.properties.demographics = countyData[f.id]?.demographics || null;
                                f.properties.name = countyData[f.id]?.name || "";
                                f.properties.population = countyData[f.id]?.population || 0;
                                const fp = f.id.substring(0, 2);
                                f.properties.state = stateLookup[fp];
                            });

                            // Add GeoJSON layers
                            geojsonNation = L.geoJSON(nation, { style, onEachFeature }).addTo(map);
                            geojsonStates = L.geoJSON(states, { style, onEachFeature });
                            geojsonCounties = L.geoJSON(counties, { style, onEachFeature });

                            // Build lookup list
                            countyIndex = counties.features.map(f => {
                                const stateName = f.properties.state
                                    ? toTitleCase(f.properties.state.replace(/_/g, " "))
                                    : "Unknown";
                                return {
                                    name: f.properties.name || "Unknown",
                                    state: stateName,
                                    fips: f.id
                                };
                            });
                            stateIndex = states.features.map(f => ({
                                name: f.properties.name,
                                fips: f.id
                            }));

                            updateLayerVisibility();
                        });
                });
        });

    function getColor(value, mode = "relative") {
        if (mode === "relative") {
            return value > 2.0 ? "#260080" :
                   value > 1.5 ? "#2600BD" :
                   value > 1.2 ? "#1C1AE3" :
                   value > 1.0 ? "#2A4EFC" :
                   value > 0.8 ? "#3C8DFD" :
                   value > 0.5 ? "#4CB2FE" :
                                 "#A0EDFF" ; 
        }
        else if (mode === "raw") {
            return value > 0.95 ? "#520016" :
                   value > 0.85 ? "#680020" :
                   value > 0.75 ? "#800026" :
                   value > 0.50 ? "#BD0026" : 
                   value > 0.25 ? "#E31A1C" : 
                   value > 0.10 ? "#FC4E2A" : 
                   value > 0.05 ? "#FD8D3C" : 
                   value > 0.01 ? "#FEB24C" : 
                                  "#FFEDA0" ;
        }
        else if (mode === "count") {
            return value > 800000 ? "#001900" :
                   value > 400000 ? "#002600" :
                   value > 100000 ? "#003300" :
                   value > 50000  ? "#0D4D0D" :
                   value > 25000  ? "#1A661A" :
                   value > 10000  ? "#268026" :
                   value > 5000   ? "#339933" :
                   value > 2500   ? "#4DB34d" :
                   value > 1000   ? "#80D580" :
                   value > 500    ? "#99DD99" :
                   value > 250    ? "#B2E5B2" :
                   value > 100    ? "#CCEECC" :
                   value > 0      ? "#E6F7E6" :
                                    "#FFFFFF" ;
        }
    }

    function getDemographicCategory(demo) {
        switch (demo) {
            case "White" : 
            case "Black" : 
            case "Hispanic" : 
            case "Asian" :
            case "Mixed" :
            case "Other" :
                return "race_and_ethnicity";
            case "Married" :
            case "Single Female" :
            case "Single Male" :
            case "One-Person" : 
            case "Other Non-Family" :
                return "household_types";
            case "Healthcare" :
            case "Retail" :
            case "Manufacturing" :
            case "Education" :
            case "Hospitality" :
            case "Professional" :
            case "Construction" :
            case "Other Services" :
            case "Government" :
            case "Finance & Insurance" :
            case "Administrative" :
            case "Transportation" :
            case "Wholesalers" :
            case "Entertainment" :
            case "Information" :
            case "Real estate" :
            case "Agriculture" :
            case "Utilities" :
            case "Oil & Gas, and Mining" :
            case "Management" :
                return "industries";
            case "Doctorate" :
            case "Professional" :
            case "Master's" :
            case "Bachelor's" :
            case "Associate's" :
            case "Some College" :
            case "High School" :
            case "Some H.S." :
            case "Less than H.S." :
            case "None" :
                return "educational_attainment";
        }
    }

    function descriptorCloseness(descriptors1, descriptors2) {
        // console.log(descriptors1, descriptors2);
        if (!descriptors1 || !descriptors2) return 0.0;
        let closeness1 = 0.0, closeness2 = 0.0;
        // Get closeness of 1 to 2
        descriptors1.forEach(d => {
            if (descriptors2.includes(d)) {
                closeness1 += (1 / descriptors1.length);
            }
        });
        // Get closeness of 2 to 1
        descriptors2.forEach(d => {
            if (descriptors1.includes(d)) {
                closeness2 += (1 / descriptors2.length);
            }
        });
        // console.log(closeness1, closeness2);
        // Average closeness
        let closeness = (closeness1 + closeness2) / 2;
        return closeness;
    }

    function style(feature) {
        if (!feature.properties.demographics) {
            return {
                fillColor: "#cccccc",
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true // <-- make sure polygons can be clicked
            };
        }

        if (descriptorsObject && selectedLayer && !selectedDescriptor) {
            let closeness = descriptorCloseness(feature.properties.descriptors, selectedLayer.feature.properties.descriptors);
            // console.log(closeness);
            const lightness = (1-closeness) * 256;
            return {
                fillColor: `rgb(${lightness}, 256, ${lightness})`,
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.7
            };
        }

        if (selectedDemographic) {
            const countyPercent = feature.properties.demographics[getDemographicCategory(selectedDemographic)][selectedDemographic] || 0;
            
            let colorValue;
            if (shadingMode === "raw") {
                colorValue = countyPercent;
            }
            else if (shadingMode === "relative") {
                const nationalPercent = nationalAverages[getDemographicCategory(selectedDemographic)][selectedDemographic] || 0.0001;
                colorValue = countyPercent / nationalPercent;
            }
            else if (shadingMode === "count") {
                colorValue = countyPercent * feature.properties.population;
            }
            return {
                fillColor: getColor(colorValue, shadingMode),
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.7
            };
        }
        else if (selectedDescriptor && feature.properties.descriptors) {
            try {
                const isMember = feature.properties.descriptors.includes(selectedDescriptor);

                return {
                    fillColor: isMember ? "#6060ff" : "#cccccc",
                    weight: 1,
                    opacity: 1,
                    color: "#333",
                    fillOpacity: 0.7
                }
            }
            catch (e) {
                console.error(e, feature.properties.descriptors);
                return {
                    fillColor: "#cccccc",
                    weight: 1,
                    opacity: 1,
                    color: "#333",
                    fillOpacity: 0.6,
                    interactive: true // <-- make sure polygons can be clicked
                };
            }
        }
        else {
            return {
                fillColor: "#cccccc",
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true // <-- make sure polygons can be clicked
            };
        }

    }

    document.getElementById("demographic-select").addEventListener("change", (e) => {
        if (descriptorsObject) {
            selectedDescriptor = e.target.value;
            if (geojsonNation) geojsonNation.setStyle(style);
            if (geojsonStates) geojsonStates.setStyle(style);
            if (geojsonCounties) geojsonCounties.setStyle(style);
        }
        else {
            selectedDemographic = e.target.value;
            if (geojsonNation) geojsonNation.setStyle(style);
            if (geojsonStates) geojsonStates.setStyle(style);
            if (geojsonCounties) geojsonCounties.setStyle(style);
            if (selectedDemographic) {
                if (!legendAdded) {
                    legend.addTo(map);
                    legendAdded = true;
                }
                else {
                    const legendDiv = document.querySelector('.legend');
                    if (legendDiv) updateLegend(legendDiv);
                }
            }
            else if (legendAdded) {
                // Remove legend if nothing selected
                legend.remove();
                legendAdded = false;
            }
        }
        
        
    });

    document.getElementById("search-form").addEventListener("submit", e => {
        e.preventDefault(); // stop page reload
        const query = searchInput.value.trim().replaceAll(/[.,'-\s]/g,"").toLowerCase();
        if (!query) return;

        // Try county match

        // Find the county feature by name
        const countyMatch = countyIndex.find(c => 
            (c.name + " " + c.state).toLowerCase() === query
        );
        if (countyMatch) {
            zoomToCounty(countyMatch.fips);
            return;
        }

        // Try state match
        
        const stateMatch = stateIndex.find(s => 
            s.name.replaceAll(/[.,'-\s]/g, '').toLowerCase() === query
        );
        if (stateMatch) {
            zoomToState(stateMatch.fips);
            return;
        }
        
        alert("No county found matching: " + query);
    });

    function zoomToCounty(fips) {
        const match = geojsonCounties.getLayers().find(layer => layer.feature.id === fips);
        if (match) {
            map.fitBounds(match.getBounds());
            highlightFeature({ target: match });
        }
    }
    function zoomToState(fips) {
        const match = geojsonStates.getLayers().find(layer => layer.feature.id === fips);
        if (match) {
            map.fitBounds(match.getBounds());
            highlightFeature({ target: match });
        }
    }

    function formatDemographics(demo) {
        let html = '';
        for (const category in demo) {
            html += `<b>${toTitleCase(category).replace(/_/g, " ")}</b><ul>`;
            const values = demo[category];
            for (const key in values) {
                if (typeof values[key] === 'object') {
                    html += `<li>${key}:<ul>`;
                    for (const subkey in values[key]) {
                        const val = values[key][subkey];
                        html += `<li>${subkey}: ${typeof val === 'number' ? val.toFixed(5) : val}</li>`;
                    }
                    html += `</ul></li>`;
                }
                else {
                    const val = values[key];
                    html += `<li>${key}: ${typeof val === 'number' ? val.toFixed(5) : val}</li>`;
                }
            }
            html += '</ul>';
        }
        return html;
    }

    function highlightFeature(e) {
        const props = e.target.feature.properties;
        console.log(props);

        const fips = e.target.feature.id;
        const state = props.state;
        const name = props.name || "United States";
        let datapath;

        if (!fips && name === "United States") { // The selected feature is the nation
            datapath = `/src/main/resources/nation.json`;
        }
        else if (fips && !state) { // The selected feature is a state
            datapath = `/src/main/resources/${name.replaceAll(" ", "_").toLowerCase()}/${fips}.json`;
        }
        else if (fips) { // The selected feature is a county
            datapath = `/src/main/resources/${state}/counties/${fips}.json`;
        }
        else {
            console.error("Missing FIPS:", props);
            return;
        }

        fetch(datapath)
            .then(res => res.json())
            .then(data => {
                const infobox = document.getElementById("infobox");
                infobox.innerHTML = `
                    <b>${data.name}</b><br>
                    Population: ${data.population}<br>
                    Demographics: <br>
                    ${formatDemographics(data.demographics)}
                `;
                infobox.scrollTop = 0;
            })
            .catch(err => console.error("Failed to load feature JSON", err));

        if (selectedLayer) {
            geojsonCounties.resetStyle(selectedLayer);
        }
        selectedLayer = e.target;
        selectedLayer.setStyle({
            weight: 3,
            color: "#ff7800",
            fillOpacity: 0.5
        });
        if (descriptorsObject) geojsonCounties.setStyle(style);
        selectedLayer.bringToFront();
    }

    function onEachFeature(feature, layer) {
        layer.on({
            click: highlightFeature,
            mouseover: () => {
                // Skip hover styling if selected
                if (layer === selectedLayer) return;
                layer.setStyle({ weight: 4, color: "#555" })
            },
            mouseout: () => {
                // Skip styling reset if selected
                if (layer === selectedLayer) return;
                geojsonCounties.resetStyle(layer)
            }
        });
    }

    document.getElementById("reset-button").addEventListener("click", () => {
        resetView();
    });

    function resetView() {
        // Reset camera
        map.setView(center, 4); // reset to default center & zoom
        // Reset demographic select
        const select = document.getElementById("demographic-select");
        if (select) {
            if (!descriptorsObject) {
                select.innerHTML = `
                    <option value="">-- Select demographic --</option>
                    <option class="race-ethnicity" value="White">Race/Ethnicity: White</option>
                    <option class="race-ethnicity" value="Hispanic">Race/Ethnicity: Hispanic</option>
                    <option class="race-ethnicity" value="Black">Race/Ethnicity: Black</option>
                    <option class="race-ethnicity" value="Asian">Race/Ethnicity: Asian</option>
                    <option class="race-ethnicity" value="Mixed">Race/Ethnicity: Mixed</option>
                    <option class="race-ethnicity" value="Other">Race/Ethnicity: Other</option>
                    <option class="household-type" value="Married">Household Type: Married</option>
                    <option class="household-type"value="Single Female">Household Type: Single Female</option>
                    <option class="household-type"value="Single Male">Household Type: Single Male</option>
                    <option class="household-type"value="One-Person">Household Type: One-Person</option>
                    <option class="household-type"value="Other Non-Family">Household Type: Other Non-Family</option>
                    <option class="industry" value="Healthcare">Industry: Healthcare</option>
                    <option class="industry" value="Retail">Industry: Retail</option>
                    <option class="industry" value="Manufacturing">Industry: Manufacturing</option>
                    <option class="industry" value="Education">Industry: Education</option>
                    <option class="industry" value="Hospitality">Industry: Hospitality</option>
                    <option class="industry" value="Professional">Industry: Professional</option>
                    <option class="industry" value="Construction">Industry: Construction</option>
                    <option class="industry" value="Other Services">Industry: Other Services</option>
                    <option class="industry" value="Government">Industry: Government</option>
                    <option class="industry" value="Finance & Insurance">Industry: Finance & Insurance</option>
                    <option class="industry" value="Administrative">Industry: Administrative</option>
                    <option class="industry" value="Transportation">Industry: Transportation</option>
                    <option class="industry" value="Wholesalers">Industry: Wholesalers</option>
                    <option class="industry" value="Entertainment">Industry: Entertainment</option>
                    <option class="industry" value="Information">Industry: Information</option>
                    <option class="industry" value="Real estate">Industry: Real estate</option>
                    <option class="industry" value="Agriculture">Industry: Agriculture</option>
                    <option class="industry" value="Utilities">Industry: Utilities</option>
                    <option class="industry" value="Oil & Gas, and Mining">Industry: Oil & Gas, and Mining</option>
                    <option class="industry" value="Management">Industry: Management</option>
                    <option class="educational-attainment" value="Doctorate">Education: Doctorate</option>
                    <option class="educational-attainment" value="Professional">Education: Professional</option>
                    <option class="educational-attainment" value="Master's">Education: Master's Degree</option>
                    <option class="educational-attainment" value="Bachelor's">Education: Bachelor's Degree</option>
                    <option class="educational-attainment" value="Associate's">Education: Associate's Degree</option>
                    <option class="educational-attainment" value="Some College">Education: Some College</option>
                    <option class="educational-attainment" value="High School">Education: High School Diploma</option>
                    <option class="educational-attainment" value="Some H.S.">Education: Some High School</option>
                    <option class="educational-attainment" value="Less than H.S.">Education: Less than High School</option>
                    <option class="educational-attainment" value="None">Education: No Formal Education</option>
                `;
            }
            select.value = "";
            select.dispatchEvent(new Event('change'));
        }
        selectedDemographic = "";
        // Remove fill colors
        if (geojsonCounties) {
            geojsonCounties.setStyle(() => ({
                fillColor: "#cccccc",
                weight: 1,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6
            }));
            if(selectedLayer) geojsonCounties.resetStyle(selectedLayer);
            selectedLayer = null;
        }
        // Reset infobox text
        document.getElementById("infobox").innerHTML = "Click on a county or county-equivalent to see demographic details.";
    }

    let legend = L.control({ position: 'bottomleft' });

    legend.onAdd = function(map) {
        const div = L.DomUtil.create('div', 'info legend');
        updateLegend(div); // initial
        return div;
    };

    let legendAdded = false;

    function updateLegend(div) {
        let grades = [];
        if (shadingMode === "relative") 
            grades = [0.50, 0.80, 1.0, 1.2, 1.5, 2]
        else if (shadingMode === "raw")
            grades = [0.0, 0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 0.85, 0.95];
        else if (shadingMode === "count")
            grades = [0, 100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000, 250000, 500000, 1000000, 4000000, 800000];

        div.innerHTML = '';
        for (let i = 0; i < grades.length; i++) {
            const from = grades[i];
            const to = grades[i + 1];
            if (shadingMode === "relative") {
                div.innerHTML +=
                `<i style="background:${getColor(from + 0.001, shadingMode)}"></i> ` +
                from.toFixed(2) * 100 + `%` + (to ? ` &ndash; ${to.toFixed(2) * 100}% of US Average<br>` :
                    `+ of US Average`);
            }
            else if (shadingMode === "raw") {
                div.innerHTML +=
                `<i style="background:${getColor(from + 0.001, shadingMode)}"></i> ` +
                from.toFixed(2) * 100 + `%` + (to ? ` &ndash; ${to.toFixed(2) * 100}%<br>` :
                    `+`);
            }
            else if (shadingMode === "count") {
                div.innerHTML +=
                `<i style="background:${getColor(from + 0.001, shadingMode)}"></i> ` +
                from + (to ? ` &ndash; ${to} people<br>` : `+ people`);
            }
        }
    }    

    const searchInput = document.getElementById("search-input");
    const suggestionsBox = document.getElementById("suggestions");

    searchInput.addEventListener("input", () => {
        const query = searchInput.value.trim().replaceAll(/[.,'-\s]/g,"").toLowerCase();
        suggestionsBox.innerHTML = "";
        if (!query) {
            suggestionsBox.style.display = "none";
            return;
        }

        const matches = [
            ...stateIndex.filter(s => 
                s.name.replaceAll(/[.,'-\s]/g,"").toLowerCase().includes(query)
            ),
            ...countyIndex.filter(c => 
                (c.name.replaceAll(/[.,'-\s]/g,"") + " " + c.state).toLowerCase().includes(query)
            ),
        ].slice(0, 10); // limit to 10 results

        if (matches.length === 0) {
            suggestionsBox.style.display = "none";
            return;
        }

        matches.forEach(match => {
            const div = document.createElement("div");
            div.textContent = match.state ?
                `${match.name}, ${match.state}` : // county
                `${match.name}`; // state
            div.addEventListener("click", () => {
                searchInput.value = match.state ?
                    `${match.name}, ${match.state}` : // county
                    `${match.name}`; // state
                suggestionsBox.style.display = "none";
                if (match.state) zoomToCounty(match.fips);
                else zoomToState(match.fips);
            });
            suggestionsBox.appendChild(div);
        });
        suggestionsBox.style.display = "block";
    });

    document.getElementById('load-descriptors-input').addEventListener('change', handleLoadDescriptorsInput, false);

    function handleLoadDescriptorsInput(event) {
        const file = event.target.files[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = (e) => {
            const fileContent = e.target.result;
            try {
                const jsonObject = JSON.parse(fileContent);
                console.log(jsonObject);
                // Successfully parsed
                descriptorsObject = jsonObject;

                // Deselect demographics
                // and remove filters from all counties
                resetView();
                const select = document.getElementById('demographic-select');
                select.innerHTML = ``;
                
                // Allow selection of descriptors
                options = [];
                if (select) {
                    Object.keys(jsonObject.descriptors).forEach(d => {
                        options.push(`<option class="descriptor" value="${d}">${d.includes("$") ? d : "Descriptor " + d}</option>`);
                    });
                }
                options.sort();
                options.unshift(`<option value="">-- Select Descriptor --</option>`);
                options.forEach(o => select.innerHTML += o); 

                // Attach descriptors to counties
                if (geojsonCounties) {
                    geojsonCounties.eachLayer(layer => {
                        layer.feature.properties.descriptors = [];
                        const fips = layer.feature.id;
                        // console.log(fips);
                        try {
                            for (let d of jsonObject.counties[fips].descriptors) {
                                layer.feature.properties.descriptors.push(d);
                            }
                        }
                        catch (e) {}
                    });

                    // Reapply style if a descriptor is selected
                    if (selectedDescriptor) geojsonCounties.setStyle(style);
                }

            } catch (error) {
                console.error('Error parsing JSON:', error);
                alert('Invalid JSON file.');
            }
        };

        reader.onerror = (e) => {
            console.error('Error reading file:', e);
            alert('Error reading the file.');
        };

        reader.readAsText(file);
    }

    resetView();

});