from typing import List, Dict, Set, Tuple, Any
from bs4 import BeautifulSoup, Tag
import os

from Paths import DATA_DIR
from NumberOperations import formatted_number_to_int, percent_to_float
from MapEntity import MapEntity
from State import State
from County import County
from page_reader import get_soup, identify_states, filter_links, get_links

def nation_html_file(data_file: str) -> str:
    ''' Get the path to an HTML file with the passed name for the nation. '''
    return DATA_DIR + "\\" + data_file + ".html"

def state_html_file(state_name: str, data_file: str) -> str:
    ''' Get the path to an HTML file with the passed name for the passed state. '''
    return DATA_DIR + "\\" + state_name + "\\" + data_file + ".html"

def county_html_file(state_name: str, county_name: str, data_file: str) -> str:
    ''' Get the path to an HTML file with the passed name for the passed county in the passed state. '''
    return DATA_DIR + "\\" + state_name + "\\counties\\" + county_name + "\\" + data_file + ".html"

def create_all_states_from_files() -> List[State]:
    ''' Create States from files in DATA_DIR. '''
    states: List[State] = []
    for state_name in os.listdir(DATA_DIR):
        if "." in state_name:
            continue # Parse directories only
        states.append(create_state_from_files(state_name))
    return states

def create_states(start_url: str, limit: int = -1) -> List[State]:
    ''' Identify and create all States (links having "state/") linked to by the start_url page.
        Only creates limit number of states (-1 to identify all). '''
    states: List[State] = []
    links = identify_states(start_url)
    for link in links:
        if limit >= 0 and len(states)+1 > limit:
            break
        state_name = link.split("/")[-2].replace("-"," ")
        soup = get_soup(link.replace("Overview","Race-and-Ethnicity"))
        states.append(create_state(soup, state_name))
    states.sort(key=lambda item: item.name)
    return states

def create_state(soup: BeautifulSoup, state_name: str) -> State:
    population = get_population(soup)
    demographics = get_race_and_ethnicity(soup)
    return State(state_name, population, {})

def create_counties(state_url: str, state: State, limit: int = -1) -> List[County]:
    ''' Identify and create all Counties (links having "county/") linked to by the start_url page.
        Only identifies limit number of counties (-1 to identify all). '''
    counties: List[County] = []
    links = filter_links(get_links(get_soup(state_url)), ["county/"])
    for link in links:
        if limit >= 0 and len(counties)+1 > limit:
            break
        county_name = link.split("/")[-2].replace("-"," ")
        soup = get_soup(link.replace("Overview","Race-and-Ethnicity"))
        print(county_name + ", " + state.name)
        counties.append(create_county(soup, county_name, state))
    counties.sort(key=lambda item: item.name)
    return counties

def create_county(soup: BeautifulSoup, county_name: str, state: State) -> County:
    population = get_population(soup)
    demographics = get_race_and_ethnicity(soup)
    return County(county_name, population, {}, state)

def create_nation_from_files() -> MapEntity:
    ''' Create the nation from its statistics files in DATA_DIR. '''
    name = "United States"
    population = get_population(BeautifulSoup(open(nation_html_file("Age-and-Sex")).read(), features="html.parser"))
    demographics = create_national_demographics_from_files()
    nation = MapEntity(name, population, demographics)
    return nation

def create_state_from_files(state_name: str) -> State:
    ''' Create one state from its statistics files in DATA_DIR. '''
    name = state_name.replace("_"," ").title()
    population = get_population(BeautifulSoup(open(state_html_file(state_name, "Age-and-Sex")).read(), features="html.parser"))
    demographics = create_demographics_from_files(state_name)
    state = State(name, population, demographics)
    return state

def create_all_counties_from_files() -> Dict[State, List[County]]:
    ''' Creates all counties (and states) from DATA_DIR files. '''
    counties: Dict[State, List[County]] = {}
    for state_name in os.listdir(DATA_DIR):
        if "." in state_name:
            continue
        state = create_state_from_files(state_name)
        counties[state] = create_all_counties_in_state_from_files(state_name, state)
    return counties

def create_all_counties_in_state_from_files(state_name, state: State | None = None) -> List[County]:
    counties: List[County] = []
    if state is None:
        print("creating " + state_name + "...")
        state = create_state_from_files(state_name)
        print(state_name + " created")
    for county_name in os.listdir(DATA_DIR + "\\" + state_name + "\\counties"):
        if "." in county_name:
            continue
        print("creating " + county_name + "...")
        counties.append(create_county_from_files(state_name, county_name, state))
        print(county_name + " created")
    return counties

def create_county_from_files(state_name: str, county_name: str, state: State) -> County:
    ''' Create one county from its statistics files in DATA_DIR. '''
    name = county_name.replace("_"," ").title()
    population = get_population(BeautifulSoup(open(county_html_file(state_name, county_name, "Age-and-Sex")).read(), features="html.parser"))
    demographics = create_demographics_from_files(state_name, county_name)
    county = County(name, population, demographics, state)
    return county

def get_population(soup: BeautifulSoup) -> int:
    ''' Get the population from the sidebar on the page. '''
    selector = "#contents-nav > div > table > tbody > tr:nth-child(1) > td"
    element = soup.select(selector)[0]
    try:
        population = int(element.get_text().replace(",",""))
        return population
    except ValueError as e:
        print(str(e))
        return 0

def create_national_demographics_from_files() -> Dict[str, Dict[str, Any]]:
    
    demographics: Dict[str, Dict[str, Any]] = {}

    soup = BeautifulSoup(open(nation_html_file("Race-and-Ethnicity")).read(), features="html.parser")
    race_and_ethnicity = get_race_and_ethnicity(soup)
    demographics["race_and_ethnicity"] = race_and_ethnicity

    soup = BeautifulSoup(open(nation_html_file("Age-and-Sex")).read(), features="html.parser")
    age_and_sex = get_age_and_sex(soup)
    demographics["age_and_sex"] = age_and_sex

    soup = BeautifulSoup(open(nation_html_file("Household-Types")).read(), features="html.parser")
    household_types = get_household_types(soup)
    demographics["household_types"] = household_types

    soup = BeautifulSoup(open(nation_html_file("Marital-Status")).read(), features="html.parser")
    marital_status = get_marital_status(soup)
    demographics["marital_status"] = marital_status

    soup = BeautifulSoup(open(nation_html_file("Ancestry")).read(), features="html.parser")
    ancestry = get_ancestry(soup)
    demographics["ancestry"] = ancestry

    soup = BeautifulSoup(open(nation_html_file("Employment-Status")).read(), features="html.parser")
    employment_status = get_employment_status(soup)
    demographics["employment_status"] = employment_status

    soup = BeautifulSoup(open(nation_html_file("Industries")).read(), features="html.parser")
    industries = get_industries(soup)
    demographics["industries"] = industries

    soup = BeautifulSoup(open(nation_html_file("Educational-Attainment")).read(), features="html.parser")
    educational_attainment = get_educational_attainment(soup)
    demographics["educational_attainment"] = educational_attainment

    return demographics

def create_demographics_from_files(state_name: str, county_name: str = "") -> Dict[str, Dict[str, Any]]:
    ''' Get all demographic details from files on a state/county. '''

    demographics: Dict[str, Dict[str, Any]] = {}

    def file_function(state_name: str, county_name: str, file_name: str) -> str:
        if county_name:
            return county_html_file(state_name, county_name, file_name)
        else:
            return state_html_file(state_name, file_name)

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Race-and-Ethnicity")).read(), features="html.parser")
        race_and_ethnicity = get_race_and_ethnicity(soup)
        demographics["race_and_ethnicity"] = race_and_ethnicity
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Age-and-Sex")).read(), features="html.parser")
        age_and_sex = get_age_and_sex(soup)
        demographics["age_and_sex"] = age_and_sex
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Household-Types")).read(), features="html.parser")
        household_types = get_household_types(soup)
        demographics["household_types"] = household_types
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Marital-Status")).read(), features="html.parser")
        marital_status = get_marital_status(soup)
        demographics["marital_status"] = marital_status
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Ancestry")).read(), features="html.parser")
        ancestry = get_ancestry(soup)
        demographics["ancestry"] = ancestry
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Employment-Status")).read(), features="html.parser")
        employment_status = get_employment_status(soup)
        demographics["employment_status"] = employment_status
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Industries")).read(), features="html.parser")
        industries = get_industries(soup)
        demographics["industries"] = industries
    except IndexError:
        pass

    try:
        soup = BeautifulSoup(open(file_function(state_name, county_name, "Educational-Attainment")).read(), features="html.parser")
        educational_attainment = get_educational_attainment(soup)
        demographics["educational_attainment"] = educational_attainment
    except IndexError:
        pass

    return demographics

def get_graphic(soup: BeautifulSoup, selector: str) -> Tag :
    ''' Get the graphic from a page using its CSS selector. '''
    try:
        graphic = soup.select(selector)[0]
        return graphic
    except IndexError as e:
        print(str(e))
        with open("logs\\log.out", 'w') as out:
            out.write(soup.prettify())
        raise e

def get_statistics_dictionary(soup: BeautifulSoup, graphic_name: str, inner_categories: List[str] | None = None) -> Dict[str, Any]:
    ''' Get a dictionary of statistics from a page using its graphic name. Inner categories will intelligently subdivide parsed information. '''

    selector = f"#figure\\/{graphic_name} > div.figure-contents > svg > g"    
    graphics = get_graphic(soup, selector).select("g")

    first: int = 0 # The first usable graphic

    for index, graphic in enumerate(graphics):
        if "font-style=\"normal\"" in graphic.prettify():
            first = index
            break

    # Get label strings from graphics
    labels: List[str] = []
    for graphic in graphics[first:]:
        if not "font-style=\"normal\"" in graphic.prettify():
            break
        label = graphic.get_text().strip()
        if len(label.split("\n")) > 1:
            label = label.split("\n")[0].strip()
        if label:
            labels.append(label)


    # Get percent values (as floats) from graphics
    values: List[float] = []
    values_per_label = len(inner_categories if inner_categories else [1])
    start_values = first+len(labels)
    end_values = first+len(labels)+(len(labels)*2*values_per_label)
    for i, graphic in enumerate(graphics[start_values:end_values]):
        if len(values) >= len(labels) * values_per_label:
            break
        try:
            value = graphic.find_all("title")[0].get_text().strip()
            if '%' in value:
                value = percent_to_float(value)
                values.append(value if value else 0.0)
        except IndexError as e:
            with open("logs\\log.out",'w') as out:
                for g in graphics:
                    out.write(g.prettify() + "\n")
            print("Graphic which broke:\n" + graphic.prettify())
            print("Which is graphic #" + str(i))
            print(str(e))
            break

    # Zip the result into dictionary
    result = {}
    if inner_categories:
        for i, label in enumerate(labels):
            result[label] = {}
            # Each label gets a dict with 'female' and 'male' values
            v = []
            for j, inner in enumerate(inner_categories):
                v.append(values[values_per_label*i+j])
                result[label][inner] = abs(values[values_per_label*i+j])
    else:
        result = dict(zip(labels, values, strict=True))

    return result

def get_race_and_ethnicity(soup: BeautifulSoup) -> Dict[str, float]:
    graphic_name = "race-and-ethnicity"

    result: Dict[str, float] = get_statistics_dictionary(soup, graphic_name)

    return result

def get_age_and_sex(soup: BeautifulSoup) -> Dict[str, float]:
    graphic_name = "age-structure"

    result: Dict[str, float] = get_statistics_dictionary(soup, graphic_name)

    return result

def get_household_types(soup: BeautifulSoup) -> Dict[str, float]:
    graphic_name = "household-types"
    
    result: Dict[str, float] = get_statistics_dictionary(soup, graphic_name)

    # consider getting more information too
    return result

def get_marital_status(soup: BeautifulSoup) -> Dict[str, Dict[str, float]]:
    graphic_name = "detailed-marital-status"
    inner_categories = ['female', 'male']

    result: Dict[str, Dict[str, float]] = get_statistics_dictionary(soup, graphic_name, inner_categories)

    return result

def get_ancestry(soup: BeautifulSoup) -> Dict[str, float]:

    # European and African Ancestry, Hispanic Ancestry, and Asian Ancestry are listed separately.
    european_graphic = "european-and-african-ancestry"
    european: Dict[str, float] = get_statistics_dictionary(soup, european_graphic)
    hispanic_graphic = "hispanic-ancestry"
    hispanic: Dict[str, float] = get_statistics_dictionary(soup, hispanic_graphic)
    asian_graphic = "asian-ancestry"
    asian: Dict[str, float] = get_statistics_dictionary(soup, asian_graphic)

    result: Dict[str, float] = european | hispanic | asian

    return result


def get_employment_status(soup: BeautifulSoup) -> Dict[str, float]:
    graphic_name = "employment-status"

    result: Dict[str, float] = get_statistics_dictionary(soup, graphic_name)

    return result

def get_industries(soup: BeautifulSoup) -> Dict[str, float]:
    graphic_name = "industry"

    result: Dict[str, float] = get_statistics_dictionary(soup, graphic_name)

    return result

def get_educational_attainment(soup: BeautifulSoup) -> Dict[str, float]:
    graphic_name = "detailed-educational-attainment"
    
    result: Dict[str, float] = get_statistics_dictionary(soup, graphic_name)

    return result
