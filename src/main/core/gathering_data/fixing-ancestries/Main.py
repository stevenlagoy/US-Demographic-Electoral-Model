import os
from typing import List, Dict, Any
from bs4 import BeautifulSoup
import requests

nation_name = "United States"

state_names = [
    "Alabama",
    "Alaska",
    "Arizona",
    "Arkansas",
    "California",
    "Colorado",
    "Connecticut",
    "Delaware",
    "District of Columbia",
    "Florida",
    "Georgia",
    "Hawaii",
    "Idaho",
    "Illinois",
    "Indiana",
    "Iowa",
    "Kansas",
    "Kentucky",
    "Louisiana",
    "Maine",
    "Maryland",
    "Massachusetts",
    "Michigan",
    "Minnesota",
    "Mississippi",
    "Missouri",
    "Montana",
    "Nebraska",
    "Nevada",
    "New Hampshire",
    "New Jersey",
    "New Mexico",
    "New York",
    "North Dakota",
    "North Carolina",
    "Ohio",
    "Oklahoma",
    "Oregon",
    "Pennsylvania",
    "Rhode Island",
    "South Carolina",
    "South Dakota",
    "Tennessee",
    "Texas",
    "Utah",
    "Vermont",
    "Virginia",
    "Washington",
    "West Virginia",
    "Wisconsin",
    "Wyoming"
]

def list_files_recursive(root_dir: str, files = None) -> List[str]:
    if files is None:
        files = []
    for path in os.listdir(root_dir):
        if '.' in path: # File
            files.append(root_dir + "\\" + path)
        else:
            for file in list_files_recursive(root_dir + "\\" + path):
                files.append(file)
    return files

def get_data_files(all_files: List[str]) -> List[str]:
    data_files: List[str] = []
    for file in all_files:
        filename = file.split("\\")[-1]
        try:
            if filename == "nation.json":
                data_files.append(file)
                continue
            int(filename.split(".")[0])
            data_files.append(file)
        except ValueError:
            continue
    return data_files

def get_name(filename: str) -> str:
    with open(filename) as f:
        for line in f:
            if "\"name\"" in line:
                name = line.split(":")[-1].replace(",","").replace("\"","").strip()
                return name
        else:
            return filename.split(".")[0]

RESOURCES_DIR = "src\\main\\resources\\2020"

base_url = "https://statisticalatlas.com/"
ancestry_url = "/Ancestry"

class Nation:
    def __init__(self, filename: str):
        with open(filename) as file:
            for line in file:
                if "\"name\"" in line:
                    self.name = line.split(":")[-1].replace(",","").replace("\"","").strip()
                elif "\"population\"" in line:
                    self.population = int(line.split(":")[-1].replace(",","").replace(",","").strip())
        self.url = base_url + self.name.replace(" ","-") + ancestry_url
        self.demographics = {}

class State:
    def __init__(self, filename: str):
        with open(filename) as file:
            for line in file:
                if "\"name\"" in line:
                    self.name = line.split(":")[-1].replace(",","").replace("\"","").strip()
                elif "\"population\"" in line:
                    self.population = int(line.split(":")[-1].replace(",","").replace(",","").strip())
                elif "\"FIPS\"" in line:
                    self.FIPS = line.split(":")[-1].replace(",","").replace("\"","").strip()
        self.url = base_url + "state/" + self.name.replace(" ","-").strip() + ancestry_url
        self.demographics = {}

class County:
    def __init__(self, filename: str, states: List[State]):
        with open(filename) as file:
            for line in file:
                if "\"name\"" in line:
                    self.name = line.split(":")[-1].replace(",","").replace("\"","").strip()
                elif "\"population\"" in line:
                    self.population = int(line.split(":")[-1].replace(",","").replace(",","").strip())
                elif "\"FIPS\"" in line:
                    self.FIPS = line.split(":")[-1].replace(",","").replace("\"","").strip()
                elif "\"state\"" in line:
                    statename = line.split(":")[-1].replace(",","").replace("\"","").strip()
                    self.state = [state for state in states if state.name == statename][0]
        self.url = base_url + "county/" + self.state.name.replace(" ","-").strip() + "/" + self.name.replace(" ","-").strip() + ancestry_url
        self.demographics = {}

def create_map_entity(filename: str, states: List[State] | None = None) -> Nation | State | County:
    if states is None: states = []

    name = get_name(filename)
    if name == nation_name:
        return Nation(filename)
    if name in state_names:
        return State(filename)
    else:
        return County(filename, states)

def main() -> None:
    all_files = list_files_recursive(RESOURCES_DIR)
    data_files = get_data_files(all_files)
    
    nation: Nation | None = None
    states: List[State] = []
    counties: List[County] = []
    for file in data_files:
        map_entity = create_map_entity(file, states)
        if isinstance(map_entity, Nation):
            nation = map_entity
        elif isinstance(map_entity, State):
            states.append(map_entity)
        elif isinstance(map_entity, County):
            counties.append(map_entity)

    all_map_entities = [nation, *states, *counties]
    print(len(all_map_entities))

    for map_entity in all_map_entities:
        url = map_entity.url
        print(url)
        response = requests.get(url)
        soup = BeautifulSoup(response.text, 'html.parser')
        
        european_graphic = "european-and-african-ancestry"
        european: Dict[str, float] = get_statistics_dictionary(soup, european_graphic)
        hispanic_graphic = "hispanic-ancestry"
        hispanic: Dict[str, float] = get_statistics_dictionary(soup, hispanic_graphic)
        asian_graphic = "asian-ancestry"
        asian: Dict[str, float] = get_statistics_dictionary(soup, asian_graphic)

        print(european)
        print(hispanic)
        print(asian)
        break
    
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
 
def get_graphic(soup: BeautifulSoup, selector: str):
    ''' Get the graphic from a page using its CSS selector. '''
    try:
        graphic = soup.select(selector)[0]
        return graphic
    except IndexError as e:
        print(str(e))
        with open("logs\\log.out", 'w') as out:
            out.write(soup.prettify())
        raise e

def percent_to_float(percent: str) -> float | None:
    ''' Turn a percent (I.E. 54.32% 0.0432%) into a float. '''
    try:
        return float(percent.replace("%","")) / 100
    except ValueError as e:
        print(str(e))
        return None


if __name__ == "__main__":
    main()