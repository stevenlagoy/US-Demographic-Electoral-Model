// PROTOTYPE AND UTIL FUNCTIONS -------------------------------------------------------------------

String.prototype.toTitleCase = function() {
    return this.toLowerCase().replace(/\b\w/g, char => char.toUpperCase());
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

function jensenShannonDistance(v1, v2) {
    function kullbackLeibler(p, q) {
        let d = [];
        for (let i = 0; i < p.length; i++) {
            if (p[i] && q[i]) d[i] = p[i] * Math.log2(p[i]/q[i]);
        }
        return d.reduce((acc, cur) => cur + acc, 0.0);
    }
    function normalize(vec) {
        let newVec = [];
        const sum = vec.reduce((acc, cur) => acc + (typeof cur === 'number' ? cur : 0.0), 0.0);
        vec.forEach(val => newVec.push(val / sum));
        return newVec;
    }
    const nv1 = normalize(v1), nv2 = normalize(v2);
    let m = [];
    for (let i = 0; i < nv1.length; i++) {
        m[i] = (nv1[i] + nv2[i]) / 2;
    }
    const js = (kullbackLeibler(nv1, m) + kullbackLeibler(nv2, m)) / 2; // Get Jensen-Shannon Divergence
    const scale = 300;
    const sim = ((1 - js) - (1 - 1 / scale)) * scale; // Scale and invert
    return 0.0 > sim ? 0.0 : 1.0 < sim ? 1.0 : sim; // Clamp to [0.0, 1.0]
}

// ENUMS AND CONSTANTS ----------------------------------------------------------------------------

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
    DESCRIPTORS: "descriptors"
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

let currentPrimarySelecteds = {
    DEMOGRAPHICS: '',
    ELECTORAL: '',
    DESCRIPTORS: '',
};

const Grades = {
    primary_selected: {
        demographics: {
            raw: max => [
                {value: max * .9, color: '#520016'},
                {value: max * .8, color: '#680020'},
                {value: max * .7, color: '#800026'},
                {value: max * .6, color: '#BD0026'},
                {value: max * .5, color: '#E31A1C'},
                {value: max * .4, color: '#FC4E2A'},
                {value: max * .3, color: '#FD8D3C'},
                {value: max * .2, color: '#FEB24C'},
                {value: max * .1, color: '#FFEDA0'},
                {value: max * .0, color: '#FFFFFF'},
            ],
            relative: max => [
                {value: max *  .500, color: '#27427B'},
                {value: max *  .370, color: '#235A9E'},
                {value: max *  .250, color: '#4E8CDB'},
                {value: max *  .120, color: '#6592BE'},
                {value: max *  .001, color: '#B0F0FF'},
                {value: max * -.001, color: '#FFFFFF'},
                {value: max * -.150, color: '#F8E172'},
                {value: max * -.250, color: '#DC9633'},
                {value: max * -.360, color: '#D67E25'},
                {value: max * -.500, color: '#CC6A19'},
                {value: max * -.999, color: '#A7521F'},
            ],
            count: scale => [
                {value: scale * (100_000 / 100_000), color: '#003000'},
                {value: scale * ( 75_000 / 100_000), color: '#004000'},
                {value: scale * ( 50_000 / 100_000), color: '#005000'},
                {value: scale * ( 25_000 / 100_000), color: '#006000'},
                {value: scale * ( 12_000 / 100_000), color: '#007000'},
                {value: scale * (   9000 / 100_000), color: '#008000'},
                {value: scale * (   7500 / 100_000), color: '#009000'},
                {value: scale * (   6000 / 100_000), color: '#00A000'},
                {value: scale * (   5000 / 100_000), color: '#00B000'},
                {value: scale * (   4000 / 100_000), color: '#00C000'},
                {value: scale * (   3000 / 100_000), color: '#00D000'},
                {value: scale * (   2500 / 100_000), color: '#00E000'},
                {value: scale * (   2000 / 100_000), color: '#00F000'},
                {value: scale * (   1500 / 100_000), color: '#20FF20'},
                {value: scale * (   1250 / 100_000), color: '#40FF40'},
                {value: scale * (   1000 / 100_000), color: '#60FF60'},
                {value: scale * (    750 / 100_000), color: '#80FF80'},
                {value: scale * (    500 / 100_000), color: '#A0FFA0'},
                {value: scale * (    250 / 100_000), color: '#C0FFC0'},
                {value: scale * (      0 / 100_000), color: '#FFFFFF'},
            ],
        },
        electoral: {
            raw: [
                {value:  0.25, color: '#F00000', label: 'Strong <b>Ⓡ</b>'},
                {value:  0.12, color: '#FF3030', label: 'Safe <b>Ⓡ</b>'},
                {value:  0.07, color: '#FF6060', label: 'Competitive <b>Ⓡ</b>'},
                {value:  0.01, color: '#FFA0A0', label: 'Leans <b>Ⓡ</b>'},
                {value: -0.01, color: '#FFFFFF', label: 'Equal'},
                {value: -0.07, color: '#A0A0FF', label: 'Leans <b>Ⓓ</b>'},
                {value: -0.12, color: '#6060FF', label: 'Competitive <b>Ⓓ</b>'},
                {value: -0.25, color: '#3030FF', label: 'Safe <b>Ⓓ</b>'},
                {value: -1.00, color: '#0000FF', label: 'Strong <b>Ⓓ</b>'},
            ],
            relative: [
                {value:  0.25, color: '#F00000', label: 'Very much more <b>Ⓡ</b> than US avg'},
                {value:  0.12, color: '#FF3030', label: 'Much more <b>Ⓡ</b> than US avg'},
                {value:  0.07, color: '#FF6060', label: 'Somewhat more <b>Ⓡ</b> than US avg'},
                {value:  0.01, color: '#FFA0A0', label: 'Slightly more <b>Ⓡ</b> than US avg'},
                {value: -0.01, color: '#FFFFFF', label: 'The same as US avg'},
                {value: -0.07, color: '#A0A0FF', label: 'Slightly more <b>Ⓓ</b> than US avg'},
                {value: -0.12, color: '#6060FF', label: 'Somewhat more <b>Ⓓ</b> than US avg'},
                {value: -0.25, color: '#3030FF', label: 'Much more <b>Ⓓ</b> than US avg'},
                {value: -1.00, color: '#0000FF', label: 'Extremely more <b>Ⓓ</b> than US avg'},
            ],
            count: scale => [
                {value: scale * (100_000 / 100_000), color: '#003000'},
                {value: scale * ( 75_000 / 100_000), color: '#004000'},
                {value: scale * ( 50_000 / 100_000), color: '#005000'},
                {value: scale * ( 25_000 / 100_000), color: '#006000'},
                {value: scale * ( 12_000 / 100_000), color: '#007000'},
                {value: scale * (   9000 / 100_000), color: '#008000'},
                {value: scale * (   7500 / 100_000), color: '#009000'},
                {value: scale * (   6000 / 100_000), color: '#00A000'},
                {value: scale * (   5000 / 100_000), color: '#00B000'},
                {value: scale * (   4000 / 100_000), color: '#00C000'},
                {value: scale * (   3000 / 100_000), color: '#00D000'},
                {value: scale * (   2500 / 100_000), color: '#00E000'},
                {value: scale * (   2000 / 100_000), color: '#00F000'},
                {value: scale * (   1500 / 100_000), color: '#20FF20'},
                {value: scale * (   1250 / 100_000), color: '#40FF40'},
                {value: scale * (   1000 / 100_000), color: '#60FF60'},
                {value: scale * (    750 / 100_000), color: '#80FF80'},
                {value: scale * (    500 / 100_000), color: '#A0FFA0'},
                {value: scale * (    250 / 100_000), color: '#C0FFC0'},
                {value: scale * (      0 / 100_000), color: '#FFFFFF'},
            ],
        },
        descriptors: [
            
        ]
    },
    no_primary_selected: {
        demographics: {
            count: scale => [
                {value: scale * (100_000 / 100_000), color: '#003000'},
                {value: scale * ( 75_000 / 100_000), color: '#004000'},
                {value: scale * ( 50_000 / 100_000), color: '#005000'},
                {value: scale * ( 25_000 / 100_000), color: '#006000'},
                {value: scale * ( 12_000 / 100_000), color: '#007000'},
                {value: scale * (   9000 / 100_000), color: '#008000'},
                {value: scale * (   7500 / 100_000), color: '#009000'},
                {value: scale * (   6000 / 100_000), color: '#00A000'},
                {value: scale * (   5000 / 100_000), color: '#00B000'},
                {value: scale * (   4000 / 100_000), color: '#00C000'},
                {value: scale * (   3000 / 100_000), color: '#00D000'},
                {value: scale * (   2500 / 100_000), color: '#00E000'},
                {value: scale * (   2000 / 100_000), color: '#00F000'},
                {value: scale * (   1500 / 100_000), color: '#20FF20'},
                {value: scale * (   1250 / 100_000), color: '#40FF40'},
                {value: scale * (   1000 / 100_000), color: '#60FF60'},
                {value: scale * (    750 / 100_000), color: '#80FF80'},
                {value: scale * (    500 / 100_000), color: '#A0FFA0'},
                {value: scale * (    250 / 100_000), color: '#C0FFC0'},
                {value: scale * (      0 / 100_000), color: '#FFFFFF'},
            ],
        },
        descriptors: [

        ]
    }
}

const statesData = [
    {FIPS: '01', abbr: 'AL', name: 'Alabama',                        censusRegion: 'South',     censusDivision: 'East South Central'},
    {FIPS: '02', abbr: 'AK', name: 'Alaska',                         censusRegion: 'West',      censusDivision: 'Pacific'},
    {FIPS: '04', abbr: 'AZ', name: 'Arizona',                        censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '05', abbr: 'AR', name: 'Arkansas',                       censusRegion: 'South',     censusDivision: 'West South Central'},
    {FIPS: '06', abbr: 'CA', name: 'California',                     censusRegion: 'West',      censusDivision: 'Pacific'},
    {FIPS: '08', abbr: 'CO', name: 'Colorado',                       censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '09', abbr: 'CT', name: 'Connecticut',                    censusRegion: 'Northeast', censusDivision: 'New England'},
    {FIPS: '10', abbr: 'DE', name: 'Delaware',                       censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '11', abbr: 'DC', name: 'District of Columbia',           censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '12', abbr: 'FL', name: 'Florida',                        censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '13', abbr: 'GA', name: 'Georgia',                        censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '15', abbr: 'HI', name: 'Hawaii',                         censusRegion: 'West',      censusDivision: 'Pacific'},
    {FIPS: '16', abbr: 'ID', name: 'Idaho',                          censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '17', abbr: 'IL', name: 'Illinois',                       censusRegion: 'Midwest',   censusDivision: 'East North Central'},
    {FIPS: '18', abbr: 'IN', name: 'Indiana',                        censusRegion: 'Midwest',   censusDivision: 'East North Central'},
    {FIPS: '19', abbr: 'IA', name: 'Iowa',                           censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '20', abbr: 'KS', name: 'Kansas',                         censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '21', abbr: 'KY', name: 'Kentucky',                       censusRegion: 'South',     censusDivision: 'East South Central'},
    {FIPS: '22', abbr: 'LA', name: 'Louisiana',                      censusRegion: 'South',     censusDivision: 'West South Central'},
    {FIPS: '23', abbr: 'ME', name: 'Maine',                          censusRegion: 'Northeast', censusDivision: 'New England'},
    {FIPS: '24', abbr: 'MD', name: 'Maryland',                       censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '25', abbr: 'MA', name: 'Massachusetts',                  censusRegion: 'Northeast', censusDivision: 'New England'},
    {FIPS: '26', abbr: 'MI', name: 'Michigan',                       censusRegion: 'Midwest',   censusDivision: 'East North Central'},
    {FIPS: '27', abbr: 'MN', name: 'Minnesota',                      censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '28', abbr: 'MS', name: 'Mississippi',                    censusRegion: 'South',     censusDivision: 'East South Central'},
    {FIPS: '29', abbr: 'MO', name: 'Missouri',                       censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '30', abbr: 'MT', name: 'Montana',                        censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '31', abbr: 'NE', name: 'Nebraska',                       censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '32', abbr: 'NV', name: 'Nevada',                         censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '33', abbr: 'NH', name: 'New Hampshire',                  censusRegion: 'Northeast', censusDivision: 'New England'},
    {FIPS: '34', abbr: 'NJ', name: 'New Jersey',                     censusRegion: 'Northeast', censusDivision: 'Mid Atlantic'},
    {FIPS: '35', abbr: 'NM', name: 'New Mexico',                     censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '36', abbr: 'NY', name: 'New York',                       censusRegion: 'Northeast', censusDivision: 'Mid Atlantic'},
    {FIPS: '37', abbr: 'NC', name: 'North Carolina',                 censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '38', abbr: 'ND', name: 'North Dakota',                   censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '39', abbr: 'OH', name: 'Ohio',                           censusRegion: 'Midwest',   censusDivision: 'East North Central'},
    {FIPS: '40', abbr: 'OK', name: 'Oklahoma',                       censusRegion: 'South',     censusDivision: 'West South Central'},
    {FIPS: '41', abbr: 'OR', name: 'Oregon',                         censusRegion: 'West',      censusDivision: 'Pacific'},
    {FIPS: '42', abbr: 'PA', name: 'Pennsylvania',                   censusRegion: 'Northeast', censusDivision: 'Mid Atlantic'},
    {FIPS: '44', abbr: 'RI', name: 'Rhode Island',                   censusRegion: 'Northeast', censusDivision: 'New England'},
    {FIPS: '45', abbr: 'SC', name: 'South Carolina',                 censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '46', abbr: 'SD', name: 'South Dakota',                   censusRegion: 'Midwest',   censusDivision: 'West North Central'},
    {FIPS: '47', abbr: 'TN', name: 'Tennessee',                      censusRegion: 'South',     censusDivision: 'East South Central'},
    {FIPS: '48', abbr: 'TX', name: 'Texas',                          censusRegion: 'South',     censusDivision: 'West South Central'},
    {FIPS: '49', abbr: 'UT', name: 'Utah',                           censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '50', abbr: 'VT', name: 'Vermont',                        censusRegion: 'Northeast', censusDivision: 'New England'},
    {FIPS: '51', abbr: 'VA', name: 'Virginia',                       censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '53', abbr: 'WA', name: 'Washington',                     censusRegion: 'West',      censusDivision: 'Pacific'},
    {FIPS: '54', abbr: 'WV', name: 'West Virginia',                  censusRegion: 'South',     censusDivision: 'South Atlantic'},
    {FIPS: '55', abbr: 'WI', name: 'Wisconsin',                      censusRegion: 'Midwest',   censusDivision: 'East North Central'},
    {FIPS: '56', abbr: 'WY', name: 'Wyoming',                        censusRegion: 'West',      censusDivision: 'Mountain'},
    {FIPS: '60', abbr: 'AS', name: 'American Samoa',                 censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '64', abbr: 'FM', name: 'Federated States of Micronesia', censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '66', abbr: 'GU', name: 'Guam',                           censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '68', abbr: 'MH', name: 'Marshall Islands',               censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '69', abbr: 'MP', name: 'Northern Mariana Islands',       censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '70', abbr: 'PW', name: 'Palau',                          censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '72', abbr: 'PR', name: 'Puerto Rico',                    censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '74', abbr: 'UM', name: 'US Minor Outlying Islands',      censusRegion: 'Territory', censusDivision: 'Territory'},
    {FIPS: '78', abbr: 'VI', name: 'Virgin Islands',                 censusRegion: 'Territory', censusDivision: 'Territory'},
];

// MAP --------------------------------------------------------------------------------------------
const center = [45, -96], defaultZoom = 4;
const map = L.map('map').setView(center, defaultZoom);

function resetView() {
    // Saccade to default position
    map.setView(center, defaultZoom);
    // Select default values
    ["shape-auto", "view-demographics", "shading-raw"].forEach(d => {
        document.getElementById(d).checked = true;
        // This will also clear the primary select value
    });
    updateShapeMode(ShapeMode.AUTO);
    updateViewMode(ViewMode.DEMOGRAPHICS);
    updateShadingMode(ShadingMode.RAW);
    updateLayerVisibility();
    // Clear selected layer
    resetLayer(selectedLayer);
    selectedLayer = null;
    refreshStyles();
    // Clear the search input
    const featureSearchInput = document.getElementById("feature-search-input");
    if (featureSearchInput) {
        featureSearchInput.value = "";
    }
    // Clear the info box
    displayMapEntityInfo();
    // Clear primarySelecteds
    currentPrimarySelecteds = {
        DEMOGRAPHICS: '',
        ELECTORAL: '',
        DESCRIPTORS: '',
    };
    const primarySelect = document.getElementById('primary-select');
    primarySelect.value = '';
    primarySelect.dispatchEvent(new Event('change'));
    // Clear legend
    clearLegend();
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

// LEGEND -----------------------------------------------------------------------------------------
const legend = L.control({ position: 'bottomleft' });

legend.onAdd = function(map) {
    const div = L.DomUtil.create('div', 'info legend');
    return div;
}

function updateLegend(grades=[], text='') {
    legend._container.innerHTML = '';
    legend._container.style.display = "block";
    
    if (text) {
        legend._container.innerHTML = `<p>${text}</p>`;
        return;
    }
    
    let from, to;

    if ((!currentPrimarySelecteds[currentViewMode] && selectedLayer && currentShadingMode !== ShadingMode.COUNT) || currentViewMode === ViewMode.DESCRIPTORS) {
        for (let g of grades) {
            legend._container.innerHTML += `
                <div><i style="background:${g.color}"></i><p>
                ${g.label}
                </p></div>
            `;
        }
        return;
    }

    switch (currentShadingMode) {
        case ShadingMode.RAW :
        case ShadingMode.RELATIVE :
            for (let i = 0; i < grades.length; i++) {
                from = grades[i]?.value, to = grades[i+1]?.value;
                if (from > to) [from, to] = [to, from]; // Swap to correct order
                from += 0.001;
                legend._container.innerHTML += `
                    <div><i style="background:${grades[i]?.color}"></i><p>
                    ${to === undefined ? `<` : ``}
                    ${(from * 100).toFixed(1)}%
                    ${to !== undefined ? `&ndash; ${(to * 100).toFixed(1)}%` : ``}
                    </p></div>
                `;
            }
            break;
        case ShadingMode.COUNT :
            for (let i = 0; i < grades.length; i++) {
                from = grades[i]?.value, to = grades[i+1]?.value;
                if (from > to) [from, to] = [to, from]; // Swap to correct order
                from = from >= 10_000 ? from.toString().replace(/000000$/,'M').replace(/000$/, 'k') : from;
                to = to >= 10_000 ? to.toString().replace(/000000$/,'M').replace(/000$/, 'k') : to;
                legend._container.innerHTML += `
                    <div><i style="background:${grades[i]?.color}"></i><p>
                    ${from}
                    ${to !== undefined ? `&ndash; ${to}` : ``}
                    </p></div>
                `;
            }
            break;
    }
}

function updateLegendForCurrentContext() {
    
    const isPrimarySelected = currentPrimarySelecteds[currentViewMode] !== '';
    
    let grades = Grades[isPrimarySelected ? 'primary_selected' : 'no_primary_selected'][currentViewMode]?.[currentShadingMode];
    let scale;

    scale = getZoomLevel() === ShapeMode.NATION ? 400_000_000 :
            getZoomLevel() === ShapeMode.STATE  ? 10_000_000  :
                                                  200_000     ;

    const max = maxDemographicValues[currentPrimarySelecteds[currentViewMode]];

    const national = nation.features[0].demographics[currentPrimarySelecteds[currentViewMode]];

    if (currentViewMode === ViewMode.DESCRIPTORS) {
        if (currentPrimarySelecteds[currentViewMode]) {
            grades = [
                {color: '#6060FF', label: 'Is member'},
                {color: '#CCC', label: 'Is not member'},
            ];
            updateLegend(grades);
        }
        else if (selectedLayer) {
            grades = [
                {color: '#004000', label: 'Shares 5 descriptors'},
                {color: '#00A000', label: 'Shares 3 descriptors'},
                {color: '#00FF00', label: 'Shares 2 descriptors'},
                {color: '#A0FFA0', label: 'Shares 1 descriptors'},
                {color: '#FFFFFF', label: 'Shares 0 descriptors'},
            ];
            updateLegend(grades);
        }
    }

    if (isPrimarySelected) {
        switch (currentShadingMode) {
            case ShadingMode.RAW :
            case ShadingMode.RELATIVE :
                if (currentViewMode === ViewMode.DEMOGRAPHICS) {
                    updateLegend(grades(max));
                }
                else if (currentViewMode === ViewMode.ELECTORAL) {
                    updateLegend(grades);
                }
                break;
            case ShadingMode.COUNT :
                const scale = getZoomLevel() === ShapeMode.NATION ? 400_000_000 :
                              getZoomLevel() === ShapeMode.STATE  ? 10_000_000  :
                                                                    200_000     ;
                updateLegend(grades(scale));
                break;
        }
    }
    else if (currentViewMode === ViewMode.DEMOGRAPHICS && currentShadingMode === ShadingMode.COUNT) {
        const scale = getZoomLevel() === ShapeMode.NATION ? 400_000_000 :
                      getZoomLevel() === ShapeMode.STATE  ? 10_000_000  :
                                                            200_000     ;
        grades = Grades.no_primary_selected.demographics.count;
        updateLegend(grades(scale));
    }
    else if (selectedLayer && currentViewMode === ViewMode.DEMOGRAPHICS) {
        grades = [
            {color: '#00FF00', label: '90% &ndash; 100% Similarity'},
            {color: '#40FF40', label: '61% &ndash; 90% Similarity'},
            {color: '#80FF80', label: '31% &ndash; 60% Similarity'},
            {color: '#B0FFB0', label: '1% &ndash; 30% Similarity'},
            {color: '#FFFFFF', label: '<1% Similarity'},
        ];
        updateLegend(grades);
    }
    else {
        clearLegend();
    }
    return;
}

function clearLegend() {
    legend._container.innerHTML = '';
    legend._container.style.display = "none";
}

// SHADING ---

function getShadingColor(value, max=1.0, national=0.0, scale=100_000) {
    const isPrimarySelected = currentPrimarySelecteds[currentViewMode] !== '';
    const grades = Grades[isPrimarySelected ? 'primary_selected' : 'no_primary_selected'][currentViewMode]?.[currentShadingMode];

    switch (currentShadingMode) {
        case ShadingMode.RAW :
            for (let g of grades(max)) if (value >= g.value) return g.color;
            break;
        case ShadingMode.RELATIVE :
            const diff = value - national;
            for (let g of grades(max)) if (diff >= g.value) return g.color;
            break;
        case ShadingMode.COUNT :
            for (let g of grades(scale)) if (value >= g.value) return g.color;
            break;
    }
    return '#000';
}

function getPartyColor(democratic, republican, year, scale=100_000) {
    const total = democratic + republican;
    democratic /= total;
    republican /= total;
    const margin = republican - democratic;
    const grades = Grades.primary_selected[currentViewMode]?.[currentShadingMode];

    switch (currentShadingMode) {
        case ShadingMode.RAW :
            for (let g of grades) if (margin > g.value) return g.color;
            break;
        case ShadingMode.RELATIVE :
            const nationData = nation.features[0].electoralData[year];
            let nationDemocratic = nationData.find(r => r.party === "DEMOCRAT").votes;
            let nationRepublican = nationData.find(r => r.party === "REPUBLICAN").votes;
            const nationTotal = nationDemocratic + nationRepublican;
            nationDemocratic /= nationTotal;
            nationRepublican /= nationTotal;
            const nationMargin = nationRepublican - nationDemocratic;
            const diff = margin - nationMargin;
            for (let g of grades) if (diff > g.value) return g.color;
            break;
        case ShadingMode.COUNT :
            for (let g of grades(scale)) if (total > g.value) return g.color;
            break;
    }
    return '#000';
}

function getDemographicCloseness(demographics1, demographics2) {
    const v1 = [], v2 = [];
    for (const category in demographics1) {
        for (const demographic in demographics1[category]) {
            if (demographics1[category]?.[demographic] === undefined || demographics2[category]?.[demographic] === undefined) continue;
            v1.push(demographics1[category][demographic]);
            v2.push(demographics2[category][demographic]);
        }
    }
    return jensenShannonDistance(v1, v2);
}

function getDescriptorCloseness(descriptors1, descriptors2, bias=1) {
    // Bias gives a baseline number of descriptors known to be shared by all map entities
    if (!descriptors1 || !descriptors2) return 0.0;
    const len1 = descriptors1.length;
    const len2 = descriptors2.length;
    // If either side has no descriptors, treat similarity as zero and avoid division by zero
    if (len1 === 0 || len2 === 0) return 0.0;
    // Clamp the effective bias so that (length - effectiveBias) is never zero or negative.
    // For very short descriptor lists (e.g., length 1), this reduces the bias to 0.
    const effectiveBias1 = Math.min(bias, Math.max(0, len1 - 1));
    const effectiveBias2 = Math.min(bias, Math.max(0, len2 - 1));
    let closeness1 = 0.0, closeness2 = 0.0;
    descriptors1.forEach(d => {
        if (descriptors2.includes(d)) {
            closeness1 += (1 / len1);
        }
    });
    if (effectiveBias1 > 0) {
        closeness1 -= effectiveBias1 / len1;
        closeness1 *= 1 + effectiveBias1 / (len1 - effectiveBias1);
    }
    descriptors2.forEach(d => {
        if (descriptors1.includes(d)) {
            closeness2 += (1 / len2);
        }
    });
    if (effectiveBias2 > 0) {
        closeness2 -= effectiveBias2 / len2;
        closeness2 *= 1 + effectiveBias2 / (len2 - effectiveBias2);
    }
    const sim = (closeness1 + closeness2) / 2;
    return 0.0 > sim ? 0.0 : 1.0 < sim ? 1.0 : sim; // Clamp to [0.0, 1.0]
}

function updateShapeMode(mode) {
    const shapeModeInput = document.getElementById(`shape-${mode}`);
    shapeModeInput.checked = true;
    currentShapeMode = mode;
    updateLayerVisibility();
    updateLegendForCurrentContext();
    refreshStyles();
}

let primarySelectOptions = {};
async function updateViewMode(mode) {
    const viewModeInput = document.getElementById(`view-${mode}`);
    viewModeInput.checked = true;
    currentViewMode = mode;
    const primarySelect = document.getElementById("primary-select");
    clearLegend();
    if (primarySelectOptions[mode]) { // Lazy load the options
        primarySelect.innerHTML = primarySelectOptions[mode];
        if (mode === ViewMode.DESCRIPTORS) {
            displayDescriptorInfo(null);
            ["shading-raw", "shading-relative", "shading-count"].forEach(i => document.getElementById(i).disabled = true);
        }
        else {
            displayMapEntityInfo(null);
            ["shading-raw", "shading-relative", "shading-count"].forEach(i => document.getElementById(i).disabled = false);
        }
        if (mode === ViewMode.ELECTORAL) {
            document.getElementById("shading-count-label").innerHTML = "Count of voters";
            const elections = Object.keys(nation.features[0].electoralData);
            let mostRecent = elections[0];
            for (const election of elections) {
                if (election > mostRecent) mostRecent = election;
            }
            if (!currentPrimarySelecteds[currentViewMode]) currentPrimarySelecteds[currentViewMode] = mostRecent;
        }
        else {
            if (currentPrimarySelecteds[currentViewMode]) {
                document.getElementById("shading-count-label").innerHTML = "Count of members";
            }
            else document.getElementById("shading-count-label").innerHTML = "Population";
        }
        primarySelect.value = currentPrimarySelecteds[currentViewMode];
        primarySelect.dispatchEvent(new Event('change')); // Force update layer styles
        refreshStyles();
        return;
    }
    switch (mode) {
        case ViewMode.DEMOGRAPHICS :
            primarySelect.innerHTML = `<option value="">--Select a Demographic--</option>`;
            const demographics = nation.features[0].demographics;
            for (const category in demographics)    
                for (const demographic in demographics[category]) {
                    if (category === 'marital_status') continue;
                    primarySelect.innerHTML += `
                        <option class="${category.replace(/_/g, "-")}" value="${category}:${demographic}">${category.replace(/_/g, " ").toTitleCase()}: ${demographic}</option>
                    `;
                }
            primarySelectOptions[mode] = primarySelect.innerHTML;
            primarySelect.dispatchEvent(new Event('change')); // Force update layer styles
            ["shading-raw", "shading-relative", "shading-count"].forEach(i => document.getElementById(i).disabled = false);
            if (currentPrimarySelecteds[currentViewMode]) {
                document.getElementById("shading-count-label").innerHTML = "Count of members";
            }
            else document.getElementById("shading-count-label").innerHTML = "Population";
            displayMapEntityInfo(null);
            break;
        case ViewMode.ELECTORAL :
            await loadElectoralData();
            primarySelect.innerHTML = '';
            const elections = Object.keys(nation.features[0].electoralData);
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
            ["shading-raw", "shading-relative", "shading-count"].forEach(i => document.getElementById(i).disabled = false);
            document.getElementById("shading-count-label").innerHTML = "Count of voters";
            displayMapEntityInfo(null);
            break;
        case ViewMode.DESCRIPTORS :
            await loadDescriptorData();
            primarySelect.innerHTML = `<option value="">--Select a Descriptor--</option>`;
            for (const descriptor of descriptors) {
                primarySelect.innerHTML += `
                    <option value="${descriptor.name}">${descriptor.name}</option>
                `;
            }
            primarySelectOptions[mode] = primarySelect.innerHTML;
            ["shading-raw", "shading-relative", "shading-count"].forEach(i => document.getElementById(i).disabled = true);
            if (currentPrimarySelecteds[currentViewMode]) {
                document.getElementById("shading-count-label").innerHTML = "Count of members";
            }
            else document.getElementById("shading-count-label").innerHTML = "Population";
            displayDescriptorInfo(null);
            break;
    }
    refreshStyles();
    updateLegendForCurrentContext();
}

function updateShadingMode(mode) {
    const shadingModeInput = document.getElementById(`shading-${mode}`);
    shadingModeInput.checked = true;
    currentShadingMode = mode;
    refreshStyles();
    updateLegendForCurrentContext();
}

let selectedLayer = null;

let geoJSONNation = null;
let nation = null;
let geoJSONStates = null;
let states = null;
let geoJSONCounties = null;
let counties = null;

// LOADING DATA -----------------------------------------------------------------------------------

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
                    console.warn(`No state found with FIPS: ${f.id}. It may be a US territory.`);
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
                    console.warn(`No county found with FIPS: ${f.id}. It may be a US territory.`);
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
            if (FIPS.length === 5) { // County
                const county = counties.features.find(c => c.id === FIPS);
                if (county) {
                    county.electoralData = data;
                    // Check for realistic values
                    for (const year in county.electoralData) {
                        const data = county.electoralData[year];
                        const totalVotes = data.reduce((acc, cur) => acc + cur.votes, 0);
                        const turnout = totalVotes / county.population;
                        if (turnout < 0.1 || turnout > 1.2)
                            console.warn(`Turnout of ${turnout.toFixed(3)} in [${county.id}] ${county.name}, ${county.state} in ${year} is dubious.`);
                    }
                }
                else {
                    console.warn(`No county found with FIPS: ${FIPS}.`);
                    continue;
                }
            }
            else if (FIPS.length === 2) { // State
                const state = states.features.find(s => s.id === FIPS);
                if (state) {
                    state.electoralData = data;
                }
                else {
                    console.warn(`No county found with FIPS: ${FIPS}.`);
                    continue;
                }
            }
            else {
                console.warn(`Unclear whether FIPS code is county or state: ${FIPS}`);
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
                        else stateResults[year].push({...res}); // Clone county electoralData. Wow this bug was hard to find.
                        const nationMatch = nationResults[year].find(r => r.candidate == res.candidate);
                        if (nationMatch) nationMatch.votes += res.votes;
                        else nationResults[year].push({...res}); // Clone county electoralData. I love implicit references I love implicit references I love implicit references I love
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
        for (const c in descriptorsData.counties) {
            const countyData = descriptorsData.counties[c]
            const county = counties.features.find(f => f.id === countyData.FIPS);
            if (!county) {
                console.warn(`Descriptor data references unknown county FIPS: ${countyData.FIPS}`);
                continue;
            }
            county.descriptors = countyData.descriptors;
        }
        for (const d in descriptorsData.descriptors) {
            const descriptorData = descriptorsData.descriptors[d];
            let name = descriptorData.name.replace(/_|\$+/g,' ').trim().toTitleCase();
            descriptors.push(descriptorData);
            if (descriptorData.name.includes("$$$$")) {
                nation.features[0].descriptors = [descriptorData.name];
                for (const state of states.features) {
                    if (!state.descriptors) state.descriptors = [];
                    state.descriptors.push(descriptorData.name);
                }
            }
            else if (descriptorData.name.includes("$$$")) {
                const memberStates = statesData.filter(s => s.censusRegion === name);
                for (const memberState of memberStates) {
                    const state = states.features.find(s => s.id === memberState.FIPS);
                    if (!state.descriptors) state.descriptors = [];
                    state.descriptors.push(descriptorData.name);
                }
            }
            else if (descriptorData.name.includes("$$")) {
                const memberStates = statesData.filter(s => s.censusDivision === name);
                for (const memberState of memberStates) {
                    const state = states.features.find(s => s.id === memberState.FIPS);
                    if (!state.descriptors) state.descriptors = [];
                    state.descriptors.push(descriptorData.name);
                }
            }
            else if (descriptorData.name.includes("$")) {
                name = name.toUpperCase();
                const memberState = statesData.find(s => s.abbr === name);
                const state = states.features.find(s => s.id === memberState.FIPS);
                if (!state.descriptors) state.descriptors = [];
                state.descriptors.push(descriptorData.name);
            }
        }
    });
}

const maxDemographicValues = {}; // Cache each demographic's maximum value among counties

// STYLING ----------------------------------------------------------------------------------------

function style(feature) {
    const blankStyle = {
        fillColor: "#cccccc",
        weight: 0.75,
        opacity: 1,
        color: "#333",
        fillOpacity: 0.6,
        interactive: true
    };
    const blankStyleNoOverwrite = {
        fillColor: "#cccccc",
        fillOpacity: 0.6,
        interactive: true
    };
    if (!currentPrimarySelecteds[currentViewMode]) { // When there is no primary selected
        if (currentShadingMode === ShadingMode.COUNT) {
            if (feature === selectedLayer?.feature) { // Don't overwrite highlighting
                // This branch should never execute
                return {
                    fillColor: getShadingColor(
                        feature.population, 1.0, 0.0,
                        getZoomLevel() === ShapeMode.NATION ? 12_500_000 :
                        getZoomLevel() === ShapeMode.STATE  ? 5_000_000 :
                                                              350_000
                    ),
                    fillOpacity: 0.6,
                    interactive: true
                };
            }
            return {
                fillColor: getShadingColor(
                    feature.population, 1.0, 0.0,
                    getZoomLevel() === ShapeMode.NATION ? 12_500_000 :
                    getZoomLevel() === ShapeMode.STATE  ? 5_000_000 :
                                                          350_000
                ),
                weight: 0.75,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true
            };
        }
        else if (!selectedLayer) return blankStyle;
        else if (currentViewMode === ViewMode.DEMOGRAPHICS) {
            const similarity = getDemographicCloseness(feature.demographics, selectedLayer?.feature.demographics);
            const shade = 256 - Math.round(256 * similarity);
            if (feature === selectedLayer?.feature) { // Don't overwrite highlighting
                selectedLayer.bringToFront();
                return {
                    fillColor: `#${shade.toString(16).padStart(2, '0')}FF${shade.toString(16).padStart(2, '0')}`,
                    fillOpacity: 0.6,
                    interactive: true
                }
            }
            return {
                fillColor: `#${shade.toString(16).padStart(2, '0')}FF${shade.toString(16).padStart(2, '0')}`,
                weight: 0.75,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true
            }
        }
        else if (currentViewMode === ViewMode.DESCRIPTORS) {
            const similarity = getDescriptorCloseness(feature.descriptors, selectedLayer?.feature.descriptors);
            // For similarity between 0-128, vary the shade (light). Between 128-256, vary the green channel (dark)
            let shade = 256 - Math.round(512 * similarity);
            shade = shade > 255 ? 255 : shade < 0 ? 0 : shade;
            let greenChannel = 512 - Math.round(448 * similarity);
            greenChannel = greenChannel > 255 ? 255 : greenChannel < 0 ? 0 : greenChannel;
            if (feature === selectedLayer?.feature) { // Don't overwrite highlighting
                selectedLayer.bringToFront();
                return {
                    fillColor: `#${shade.toString(16).padStart(2, '0')}${greenChannel.toString(16).padStart(2, '0')}${shade.toString(16).padStart(2, '0')}`,
                    fillOpacity: 0.6,
                    interactive: true
                }
            }
            return {
                fillColor: `#${shade.toString(16).padStart(2, '0')}${greenChannel.toString(16).padStart(2, '0')}${shade.toString(16).padStart(2, '0')}`,
                weight: 0.75,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true
            }
        }
    }
    switch (currentViewMode) {
        case ViewMode.DEMOGRAPHICS :
            const [selectedDemoCategory, selectedDemographic] = currentPrimarySelecteds[currentViewMode]?.split(":");
            if (!feature.demographics) return blankStyle;
            let highestPercent = maxDemographicValues[currentPrimarySelecteds[currentViewMode]];
            if (!highestPercent) {
                const ranked = counties.features.sort((a, b) => (b.demographics?.[selectedDemoCategory]?.[selectedDemographic] || 0.0) - (a.demographics?.[selectedDemoCategory]?.[selectedDemographic] || 0.0));
                highestPercent = ranked[0].demographics[selectedDemoCategory][selectedDemographic];
            }
            maxDemographicValues[currentPrimarySelecteds[currentViewMode]] = highestPercent;
            const nationalPercent = nation.features[0].demographics[selectedDemoCategory][selectedDemographic];
            if (feature === selectedLayer?.feature) { // Don't overwrite highlighting
                return {
                    fillColor: getShadingColor(
                        currentShadingMode === ShadingMode.COUNT ?
                        feature.demographics[selectedDemoCategory]?.[selectedDemographic] * feature.population || 0 :
                        feature.demographics[selectedDemoCategory]?.[selectedDemographic] || 0,
                        highestPercent, nationalPercent,
                        getZoomLevel() === ShapeMode.NATION ? 400_000_000 :
                        getZoomLevel() === ShapeMode.STATE  ? 10_000_000  :
                                                              200_000
                    ),
                    fillOpacity: 0.6,
                    interactive: true
                };
            }
            return {
                fillColor: getShadingColor(
                    currentShadingMode === ShadingMode.COUNT ?
                    feature.demographics[selectedDemoCategory]?.[selectedDemographic] * feature.population || 0 :
                    feature.demographics[selectedDemoCategory]?.[selectedDemographic] || 0,
                    highestPercent, nationalPercent,
                    getZoomLevel() === ShapeMode.NATION ? 400_000_000 :
                    getZoomLevel() === ShapeMode.STATE  ? 10_000_000  :
                                                          200_000
                ),
                weight: 0.75,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.6,
                interactive: true
            };
        case ViewMode.ELECTORAL :
            if (!feature.electoralData) return blankStyle;
            const rep_votes = feature.electoralData[currentPrimarySelecteds[currentViewMode]]?.find(r => r.party == "REPUBLICAN")?.votes;
            const dem_votes = feature.electoralData[currentPrimarySelecteds[currentViewMode]]?.find(r => r.party == "DEMOCRAT")?.votes;
            if (feature === selectedLayer?.feature) { // Don't overwrite highlighting
                selectedLayer.bringToFront();
                return {
                    fillColor: getPartyColor(
                        dem_votes, rep_votes, currentPrimarySelecteds[currentViewMode],
                        getZoomLevel() === ShapeMode.NATION ? 800_000_000 :
                        getZoomLevel() === ShapeMode.STATE  ? 40_000_000 :
                                                              800_000
                    ),
                    fillOpacity: 0.7,
                    interactive: true
                }
            }
            return {
                fillColor: getPartyColor(
                    dem_votes, rep_votes, currentPrimarySelecteds[currentViewMode],
                    getZoomLevel() === ShapeMode.NATION ? 800_000_000 :
                    getZoomLevel() === ShapeMode.STATE  ? 40_000_000 :
                                                          800_000
                ),
                weight: 0.75,
                opacity: 1,
                color: "#333",
                fillOpacity: 0.7,
                interactive: true
            };
        case ViewMode.DESCRIPTORS :
            if (!feature.descriptors) return blankStyle;
            if (feature.descriptors.find(d => d == currentPrimarySelecteds[currentViewMode])) { // Is member
                if (feature === selectedLayer?.feature) { // Don't overwrite highlighting
                    return {
                        fillColor: "#6060FF",
                        fillOpacity: 0.6,
                        interactive: true
                    };
                }
                return {
                    fillColor: "#6060FF",
                    weight: 0.75,
                    opacity: 1,
                    color: "#333",
                    fillOpacity: 0.6,
                    interactive: true
                };
            }
            if (feature === selectedLayer?.feature) // Don't overwrite highlighting
                return blankStyleNoOverwrite;
            return blankStyle;
    }
}

function resetLayer(layer) {
    if (!layer) return;
    layer.setStyle({
        weight: 0.75,
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
    refreshStyles();
    if (layer.feature.state) updateShapeMode(ShapeMode.COUNTY);
}

function onEachFeature(feature, layer) {
    layer.on({
        click: () => {
            if (selectedLayer) resetLayer(selectedLayer);
            highlightLayer(layer);
            if (currentViewMode !== ViewMode.DESCRIPTORS || !currentPrimarySelecteds[currentViewMode])
                displayMapEntityInfo(feature);
            updateLegendForCurrentContext();
        },
        mouseover: () => {
            if (layer !== selectedLayer)
                layer.setStyle({ weight: 3, color: "#555" });
        },
        mouseout: () => {
            if (layer !== selectedLayer) {
                let ownerLayer;
                if (getZoomLevel() === ShapeMode.NATION || currentShapeMode === ShapeMode.NATION) {
                    ownerLayer = geoJSONNation;
                } else if (getZoomLevel() === ShapeMode.STATE || currentShapeMode === ShapeMode.STATE) {
                    ownerLayer = geoJSONStates;
                } else {
                    ownerLayer = geoJSONCounties;
                }
                if (ownerLayer) {
                    ownerLayer.resetStyle(layer);
                }
            }
        }
    });
}

function updateLayerVisibility() {
    if (getZoomLevel() === ShapeMode.NATION || currentShapeMode === ShapeMode.NATION) {
        map.addLayer(geoJSONNation);
        map.removeLayer(geoJSONStates);
        map.removeLayer(geoJSONCounties);
    }
    else if (getZoomLevel() === ShapeMode.STATE || currentShapeMode === ShapeMode.STATE) {
        map.removeLayer(geoJSONNation);
        map.addLayer(geoJSONStates);
        map.removeLayer(geoJSONCounties);
    }
    else {
        map.removeLayer(geoJSONNation);
        map.removeLayer(geoJSONStates);
        map.addLayer(geoJSONCounties);
    }
    refreshStyles();
    updateLegendForCurrentContext();
}

function getZoomLevel() {
    const zoom = map.getZoom();
    if (currentShapeMode === ShapeMode.AUTO && zoom <= 4 || currentShapeMode === ShapeMode.NATION) {
        return ShapeMode.NATION;
    }
    else if (currentShapeMode === ShapeMode.AUTO && zoom <= 6 || currentShapeMode === ShapeMode.STATE) {
        return ShapeMode.STATE;
    }
    else {
        return ShapeMode.COUNTY;
    }
}

// INFOBOX SIDEBAR --------------------------------------------------------------------------------

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
            case ViewMode.DESCRIPTORS :
                mapInfobox.innerHTML += `<h3>Descriptors:</h3> <p># Memberships: ${properties.descriptors.length}</p> ${formatDescritorMemberships(properties.descriptors)}`;
                break;
        }
    }
}

function displayDescriptorInfo(descriptor) {
    const mapInfobox = document.getElementById("map-infobox");
    if (!descriptor) {
        mapInfobox.innerHTML = `<h3>Click on a state, county, the nation, or select a descriptor to see details.</h3>`;
    }
    else {
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

// For a descriptor to format its members
function formatDescriptorMembers(members) {
    let html = `<ul>`;
    for (const member of members) {
        const county = counties.features.find(c => c.id === member);
        html += `<li>[${member}] ${county.name}, ${county.state}</li>`;
    }
    html += `</ul>`;
    return html;
}

// For a county or state to format its memberships
function formatDescritorMemberships(memberships) {
    let html = `<ul>`;
    for (const membership of memberships) {
        html += `<li>${membership}</li>`;
    }
    html += `</ul>`;
    return html;
}

function formatElectoralData(electoralData, population) {
    let html = '';
    for (const year in electoralData) {
        const data = electoralData[year];
        const totalVotes = data.reduce((acc, cur) => acc + parseInt(cur.votes, 10), 0);
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

// DOMCONTENTLOADED -------------------------------------------------------------------------------
document.addEventListener('DOMContentLoaded', async () => {

    // Attach listeners

    // Basemap
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 10
    }).addTo(map);

    window.addEventListener('resize', () => map.invalidateSize());

    legend.addTo(map);
    clearLegend();

    const shapeModeSelect = document.getElementById("shape-mode");
    shapeModeSelect.addEventListener('change', event => {
        let selectedMode = ShapeMode[event.target.value];
        if (!selectedMode) {
            console.error(`Failed to convert selected shape-mode to a known ShapeMode: "${event.target.value}"`);
            selectedMode = ShapeMode.AUTO;
        }
        updateShapeMode(selectedMode);
    });

    const viewModeSelect = document.getElementById("view-mode");
    viewModeSelect.addEventListener('change', event => {
        let selectedMode = ViewMode[event.target.value];
        if (!selectedMode) {
            console.error(`Failed to convert selected view-mode to a known ViewMode: "${event.target.value}"`);
            selectedMode = ViewMode.DEMOGRAPHICS;
        }
        updateViewMode(selectedMode);
    });

    const shadingModeSelect = document.getElementById("shading-mode");
    shadingModeSelect.addEventListener('change', event => {
        let selectedMode = ShadingMode[event.target.value];
        if (!selectedMode) {
            console.error(`Failed to convert selected shading-mode to a known ShadingMode: "${event.target.value}"`);
            selectedMode = ShadingMode.RAW;
        }
        updateShadingMode(selectedMode);
    });

    const primarySelect = document.getElementById("primary-select");
    primarySelect.addEventListener('change', event => {
        currentPrimarySelecteds[currentViewMode] = event.target.value;
        refreshStyles();
        updateLayerVisibility();
        updateLegendForCurrentContext();
        if (currentViewMode === ViewMode.DEMOGRAPHICS) {
            if (currentPrimarySelecteds[currentViewMode]) document.getElementById("shading-count-label").innerHTML = "Count of members";
            else document.getElementById("shading-count-label").innerHTML = "Population";
        }
        else if (currentPrimarySelecteds[currentViewMode] && currentViewMode === ViewMode.DESCRIPTORS) {
            const descriptor = descriptors.find(d => d.name === currentPrimarySelecteds[currentViewMode]);
            if (!descriptor.name.includes("$")) updateShapeMode(ShapeMode.COUNTY);
            else if (!descriptor.name.includes("$$$$")) updateShapeMode(ShapeMode.STATE); // && currentShapeMode !== ShapeMode.COUNTY
            else updateShapeMode(ShapeMode.AUTO);
            displayDescriptorInfo(descriptor);
        }
        if (!currentPrimarySelecteds[currentViewMode] && currentViewMode === ViewMode.DESCRIPTORS) {
            if (selectedLayer) {
                highlightLayer(selectedLayer);
                displayMapEntityInfo(selectedLayer?.feature);
            }
            else {
                displayDescriptorInfo(null);
            }
        }
    });

    const clearPrimarySelectButton = document.getElementById("clear-primary-select-button");
    clearPrimarySelectButton.addEventListener('click', event => {
        event.preventDefault();
        // Clear primarySelecteds
        currentPrimarySelecteds[currentViewMode] = '';
        primarySelect.value = '';
        primarySelect.dispatchEvent(new Event('change'));
        // Update legend
        updateLegendForCurrentContext();
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
            bestMatch = counties.features.find(c => `${c.name?.replaceAll(/[.,'-\s]/g, "")}${c.state}`.toLowerCase().includes(query));
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
            ...counties.features.filter(c => `${c.name?.replaceAll(/[.,'-\s]/g, "")}${c.state}`.toLowerCase().includes(query)),
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