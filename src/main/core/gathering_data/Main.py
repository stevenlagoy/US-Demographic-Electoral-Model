from typing import List, Dict
import os
import json
import time

from Paths import *
from page_reader import get_soup, identify_states, identify_counties
from MapEntityFactory import create_state_from_files, create_county_from_files, create_nation_from_files

def webpages_to_files():
    ''' Read state data into files in resources\\data for quicker access later. '''

    pages = ["Race-and-Ethnicity", "Age-and-Sex", "Household-Types", "Marital-Status", "Ancestry", "Household-Income", "Employment-Status", "Industries", "Educational-Attainment"]

    # Read and write nation data
    for page in pages:
        soup = get_soup("https://statisticalatlas.com/United-States/Overview" + page)
        with open(DATA_DIR + "\\" + page + ".html", 'w', encoding='utf-8') as out:
            out.write(soup.prettify())

    # Identify States and write files
    states_links = identify_states("https://statisticalatlas.com/United-States/Overview")
    for link in states_links:
        state_page_name = link.split("/")[-2]
        state_file_name = state_page_name.replace("-","_").lower()
        print(state_file_name + " -----------------------------------------")

        state_folder = DATA_DIR + "\\" + state_file_name
        try:
            os.mkdir(state_folder)
        except FileExistsError:
            pass
        except OSError as e:
            print(str(e))
        try:
            os.mkdir(state_folder + "\\counties\\")
        except FileExistsError:
            pass
        except OSError as e:
            print(str(e))

        for page in pages:
            if not os.path.exists(state_folder + "\\" + page + ".html"):
                soup = get_soup(link.replace("Overview", page))
                with open(state_folder + "\\" + page + ".html", 'w', encoding='utf-8') as out:
                    out.write(soup.prettify())

        # state_tasks = []
        # with ThreadPoolExecutor(max_workers=1) as executor:
        #     for page in pages:
        #         filepath = state_folder + "\\" + page + ".html"
        #         if not os.path.exists(filepath):
        #             url = link.replace("Overview", page)
        #             state_tasks.append(executor.submit(fetch_and_save, url, filepath))
        #     for future in as_completed(state_tasks):
        #         future.result()

        counties_links = identify_counties(link)
        for c_link in counties_links:
            county_page_name = c_link.split("/")[-2]
            county_file_name = county_page_name.replace("-","_").lower()
            print(county_file_name)

            county_folder = state_folder + "\\counties\\" + county_file_name
            try:
                os.mkdir(county_folder)
            except FileExistsError:
                pass
            except OSError as e:
                print(str(e))

            # county_tasks = []
            # for page in pages:
            #     filepath = county_folder + "\\" + page + ".html"
            #     if not os.path.exists(filepath):
            #         url = c_link.replace("Overview", page)
            #         county_tasks.append(executor.submit(fetch_and_save, url, filepath))
            # for future in as_completed(county_tasks):
            #     future.result()

            for page in pages:
                if not os.path.exists(county_folder + "\\" + page + ".html"):
                    c_soup = get_soup(c_link.replace("Overview", page))
                    with open(county_folder + "\\" + page + ".html", 'w', encoding='utf-8') as out:
                        out.write(c_soup.prettify())

def combine_jsons():

    output: List[str] = []
    states: List[str] = []

    with open(DATA_DIR + "\\data.json") as nation_file:
        for line in nation_file.readlines():
            output.append(line)
    for state_folder in os.listdir(DATA_DIR):
        if "." in state_folder:
            continue # only read folders
        with open(DATA_DIR + state_folder + "\\data.json") as state_file:
            for line in state_file.readlines():
                states.append(line)
        states[-1] += ",\n"
    output[-2] = output[-2].replace("\n","") + ",\n"
    output.insert(-1, "\t\"states\" : [\n")
    for line in states:
        output.insert(-1, "\t\t" + line)
    output.insert(-1, "\t]\n")
    output[-3] = output[-3].replace(",","") # remove trailing comma from states list
    with open(DATA_DIR + "combined_data.json", 'w') as file:
        for line in output:
            file.write(line)

def list_states() -> List[str]:
    return [state for state in os.listdir(DATA_DIR) if "." not in state]

def convert_html_json_files():
    if not os.path.exists(f"{RESOURCES_DIR}\\nation.json"):
        print("NATION ===========================")
        nation = create_nation_from_files()
        with open(f"{RESOURCES_DIR}\\nation.json",'w',encoding='utf-8') as out:
            nation_json = nation.to_json()
            out.write(json.dumps(nation_json, indent=4, separators=(", "," : ")))

    for state_name in os.listdir(f"{DATA_DIR}"):
        if '.' not in state_name:
            print(state_name + " ------------------------------")
            state = create_state_from_files(state_name)
            if not os.path.exists(f"{RESOURCES_DIR}\\{state_name}\\{state_name}.json"):
                try:
                    os.mkdir(f"{RESOURCES_DIR}\\{state_name}")
                except OSError:
                    pass
                with open(f"{RESOURCES_DIR}\\{state_name}\\{state_name}.json",'w',encoding='utf-8') as out:
                    state_json = state.to_json()
                    out.write(json.dumps(state_json, indent=4, separators=(", "," : ")))
            for county_name in os.listdir(f"{DATA_DIR}\\{state_name}\\counties"):
                if not "." in county_name and not os.path.exists(f"{RESOURCES_DIR}\\{state_name}\\counties\\{county_name}.json"):
                    print(county_name)
                    county = create_county_from_files(state_name, county_name, state)
                    try:
                        os.mkdir(f"{RESOURCES_DIR}\\{state_name}\\counties")
                    except OSError:
                        pass
                    with open(f"{RESOURCES_DIR}\\{state_name}\\counties\\{county_name}.json",'w',encoding='utf-8') as out:
                        county_json = county.to_json()
                        out.write(json.dumps(county_json, indent=4, separators=(", "," : ")))

def count_counties():
    for state_name in [_ for _ in os.listdir(f"{RESOURCES_DIR}") if '.' not in _]:
        counties_count = len([_ for _ in os.listdir(f"{RESOURCES_DIR}\\{state_name}\\counties")])
        print(f"{state_name} has {counties_count} counties")

unremovable_files: List[str] = []
def verify_data():
    global unremovable_files
    ''' Deletes all data files which contain a rate limit message. '''
    for file in list_files_recursive(DATA_DIR):
        print("Verifying: " + file)
        remove_file = False
        with open(file) as data:
            if "Too many requests" in data.readline():
                print("BAD FILE: " + file)
                remove_file = True
        if remove_file:
            for _ in range(10):
                try:
                    os.chmod(file, 0o777)
                    os.remove(file)
                    print("REMOVED: " + file)
                    break
                except PermissionError:
                    time.sleep(0.1)
            else:
                print("UNABLE TO REMOVE: " + file)
                unremovable_files.append(file)
    print("Files which could not be removed:")
    for file in unremovable_files:
        print(file)

def validate_json():
    ''' Check that all json data files contian the necessary objects. '''
    bad_files: List[str] = []
    required_lines = ["name", "FIPS", "population", "demographics", "race_and_ethnicity", "age_and_sex", "household_types", "marital_status", "employment_status", "industries", "educational_attainment"]
    for file in list_files_recursive(RESOURCES_DIR):
        has_required: List[bool] = [False for _ in required_lines]
        content: List[str] = []
        with open(file, 'r', encoding='utf-8') as data:
            content = data.readlines()
        for i, required_line in enumerate(required_lines):
            for line in content[i:]:
                if required_line in line:
                    has_required[i] = True
                    break
        if False in has_required:
            bad_files.append(file)
            print(f"{file} is BAD ------------------------------")
        else:
            print(f"{file} is good")
    print("Bad files:")
    for file in bad_files:
        print(file)

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

def add_states_to_jsons():
    for state_dir in os.listdir(RESOURCES_DIR):
        if '.' in state_dir:
            continue
        state_name = state_dir.replace("_"," ").title()
        for county_file in os.listdir(f"{RESOURCES_DIR}\\{state_dir}\\counties"):
            content: List[str] = []
            with open(f"{RESOURCES_DIR}\\{state_dir}\\counties\\{county_file}",'r') as data:
                content = data.readlines()
            if "\"state\"" in "\n".join(content):
                continue
            for i, line in enumerate(content):
                if "\"name\"" in line:
                    content.insert(i+1, f"\t\"state\" : \"{state_name}\", \n")
                    break
            with open(f"{RESOURCES_DIR}\\{state_dir}\\counties\\{county_file}","w") as data:
                for line in content:
                    data.write(line)

def add_fips_to_jsons():

    fips: Dict[str, str] = {}
    with open("src/main/core/gathering_data/counties.json",'r',encoding='utf-8') as c:
        counties_lines = c.readlines()
    for i, line in enumerate(counties_lines[1:]):
        if "{" in line:
            name = line.split(":")[0].replace("\"",'').replace(".",'').strip().title().replace("ö","o")
            code = counties_lines[1:][i+2].split(":")[1].replace("\"",'').replace(",",'').strip()
            fips[name] = code
    with open("src/main/core/gathering_data/state_fips.txt",'r',encoding='utf-8') as s:
        states_lines = s.readlines()
    for line in states_lines:
        name = line.split(" ")[0].title().replace("_"," ").strip()
        code = line.split(" ")[1].strip()
        fips[name] = code

    special = {
        "Anchorage Municipality" : "Municipality Of Anchorage",
        "Hoonah Angoon Census Area" : "Hoonah-Angoon Census Area",
        "Juneau City And Borough" : "City And Borough Of Juneau",
        "Matanuska Susitna Borough" : "Matanuska-Susitna Borough",
        "Prince Of Wales Hyder Census Area" : "Prince Of Wales-Hyder Census Area",
        "Sitka City And Borough" : "City And Borough Of Sitka",
        "Skagway Municipality" : "Municipality And Borough Of Skagway",
        "Valdez Cordova Census Area" : "Chugach Census Area",
        "Wrangell City And Borough" : "City And Borough Of Wrangell",
        "Yakutat City And Borough" : "City And Borough Of Yakutat",
        "Yukon Koyukuk Census Area" : "Yukon-Koyukuk Census Area",
        "Miami Dade County" : "Miami-Dade County",
        "De Witt County" : "Dewitt County",
        "Obrien County" : "O'Brien County",
        "De Soto Parish" : "Desoto Parish",
        "Baltimore City" : "Baltimore Independent City",
        "Prince Georges County" : "Prince George'S County",
        "Queen Annes County" : "Queen Anne'S County",
        "St Marys County" : "St Mary'S County",
        "St Louis City" : "St Louis Independent City",
        "Carson City" : "Carson Independent City",
        "Doã±A Ana County" : "Doña Ana County",
        "Le Flore County" : "Leflore County",
        "Alexandria City" : "Alexandria Independent City",
        "Bristol City" : "Bristol Independent City",
        "Buena Vista City" : "Buena Vista Independent City",
        "Charlottesville City" : "Charlottesville Independent City",
        "Chesapeake City" : "Chesapeake Independent City",
        "Colonial Heights City" : "Colonial Heights Independent City",
        "Covington City" : "Covington Independent City",
        "Colonial Heights City" : "Colonial Heights Independent City",
        "Danville City" : "Danville Independent City",
        "Emporia City" : "Emporia Independent City",
        "Fairfax City" : "Fairfax Independent City",
        "Falls Church City" : "Falls Church Independent City",
        "Franklin City" : "Franklin Independent City",
        "Fredericksburg City" : "Fredericksburg Independent City",
        "Galax City" : "Galax Independent City",
        "Hampton City" : "Hampton Independent City",
        "Hampton City" : "Hampton Independent City",
        "Harrisonburg City" : "Harrisonburg Independent City",
        "Hopewell City" : "Hopewell Independent City",
        "Lexington City" : "Lexington Independent City",
        "Lynchburg City" : "Lynchburg Independent City",
        "Manassas City" : "Manassas Independent City",
        "Manassas Park City" : "Manassas Park Independent City",
        "Martinsville City" : "Martinsville Independent City",
        "Newport News City" : "Newport News Independent City",
        "Norfolk City" : "Norfolk Independent City",
        "Norton City" : "Norton Independent City",
        "Petersburg City" : "Petersburg Independent City",
        "Poquoson City" : "Poquoson Independent City",
        "Portsmouth City" : "Portsmouth Independent City",
        "Radford City" : "Radford Independent City",
        "Richmond City" : "Richmond Independent City",
        "Roanoke City" : "Roanoke Independent City",
        "Salem City" : "Salem Independent City",
        "Staunton City" : "Staunton Independent City",
        "Suffolk City" : "Suffolk Independent City",
        "Virginia Beach City" : "Virginia Beach Independent City",
        "Waynesboro City" : "Waynesboro Independent City",
        "Williamsburg City" : "Williamsburg Independent City",
        "Winchester City" : "Winchester Independent City",
    }

    data_files = list_files_recursive(RESOURCES_DIR);
    for data_file in data_files:
        state_name = data_file.split("\\")[-3].replace("_"," ").title()
        if state_name == "Main":
            pass
        elif state_name == "Resources":
            state_name = data_file.split("\\")[-1].replace(".json",'').replace("_"," ").title()
            code = fips[state_name]
            with open(data_file,'r',encoding='utf-8') as data:
                content = data.readlines()
            content.insert(2, "\t\"FIPS\" : \"" + code + "\",\n")
            with open(data_file,'w',encoding="utf-8") as out:
                for line in content:
                    out.write(line)
        else:
            county_name = data_file.split("\\")[-1].replace(".json",'').replace("_"," ").title()
            if county_name in special:
                county_name = special[county_name]
            fullname = county_name + ", " + state_name
            code = fips[fullname]
            with open(data_file,'r',encoding='utf-8') as data:
                content = data.readlines()
            content.insert(3, "\t\"FIPS\" : \"" + code + "\",\n")
            with open(data_file,'w',encoding="utf-8") as out:
                for line in content:
                    out.write(line)

def rename_jsons_to_fips():
    data_files = list_files_recursive(RESOURCES_DIR)
    for data_file in data_files:
        # Get FIPS code from file
        code: str = ""
        content: List[str] = []
        with open(data_file, 'r', encoding='utf-8') as data:
            content = data.readlines()
        for line in content:
            if 'FIPS' in line:
                code = line.split(":")[-1].replace(",","").replace("\"","").strip()
                break
        if code:
            path = "\\".join(data_file.split("\\")[:-1]) + "\\"
            with open(path + code + ".json",'w',encoding='utf-8') as out:
                for line in content:
                    out.write(line)

def main() -> None:

    # Gather all webpages
    webpages_to_files()

    # Verify downloaded webpage files, and delete any invalid
    verify_data()

    # Convert downloaded pages into json files
    convert_html_json_files()

    # Validate the json files
    validate_json()

    # Count the counties in each state
    count_counties()

    # Add state names to the json files
    add_states_to_jsons()

    # Add FIPS codes to the json files
    add_fips_to_jsons()

    # Generate files with FIPS codes as names
    rename_jsons_to_fips()

if __name__ == "__main__":
    main()