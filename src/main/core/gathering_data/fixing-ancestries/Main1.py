from typing import List
import os

european_mena_african_ancestries = [
    "Afghan",
	"Albanian",
	"Alsatian",
	"American",
	"Arab",
	"Egyptian",
	"Iraqi",
	"Jordanian",
	"Lebanese",
	"Moroccan",
	"Palestinian",
	"Syrian",
	"Arab",
	"Other Arab",
	"Armenian",
	"Assyrian/Chaldean/Syriac",
	"Australian",
	"Austrian",
	"Basque",
	"Belgian",
	"Brazilian",
	"British",
	"Bulgarian",
	"Cajun",
	"Canadian",
	"Carpatho Rusyn",
	"Celtic",
	"Croatian",
	"Cypriot",
	"Czech",
	"Czechoslovakian",
	"Danish",
	"Dutch",
	"Eastern European",
	"English",
	"Estonian",
	"European",
	"Finnish",
	"French",
	"French Canadian",
	"German",
	"German Russian",
	"Greek",
	"Guyanese",
	"Hungarian",
	"Icelander",
	"Iranian",
	"Irish",
	"Israeli",
	"Italian",
	"Latvian",
	"Lithuanian",
	"Luxemburger",
	"Macedonian",
	"Maltese",
	"New Zealander",
	"Northern European",
	"Norwegian",
	"Pennsylvania German",
	"Polish",
	"Portuguese",
	"Romanian",
	"Russian",
	"Scandinavian",
	"Scotch-Irish",
	"Scottish",
	"Serbian",
	"Slavic",
	"Slovak",
	"Slovene",
	"Soviet Union",
	"Subsaharan African",
	"Cape Verdean",
	"Ethiopian",
	"Ghanaian",
	"Kenyan",
	"Liberian",
	"Nigerian",
	"Senegalese",
	"Sierra Leonean",
	"Somali",
	"South African",
	"Sudanese",
	"Ugandan",
	"Zimbabwean",
	"African",
	"Other",
	"Swedish",
	"Swiss",
	"Turkish",
	"Ukrainian",
	"Welsh",
	"West Indian",
	"Bahamian",
	"Barbadian",
	"Belizean",
	"Bermudan",
	"British West Indian",
	"Dutch West Indian",
	"Haitian",
	"Jamaican",
	"Trinidadian and Tobagonian",
	"U.S. Virgin Islander",
	"West Indian",
	"Other West Indian",
	"Yugoslavian",
	"Other",
	"Unclassified"
]
hispanic_ancestries = [
    "Mexican",
	"Puerto Rican",
	"Cuban",
	"Dominican",
	"Central American",
	"Costa Rican",
	"Guatemalan",
	"Honduran",
	"Nicaraguan",
	"Panamanian",
	"Salvadoran",
	"Other Central American",
	"South American",
	"Argentinean",
	"Bolivian",
	"Chilean",
	"Colombian",
	"Ecuadorian",
	"Paraguayan",
	"Peruvian",
	"Uruguayan",
	"Venezuelan",
	"Other South American",
	"Other Hispanic",
	"Spaniard",
	"Spanish",
	"Spanish American"
]
asian_ancestries = [
    "Indian",
	"Bangladeshi",
	"Bhutanese",
	"Burmese",
	"Cambodian",
	"Chinese",
	"Filipino",
	"Hmong",
	"Indonesian",
	"Japanese",
	"Korean",
	"Laotian",
	"Malaysian",
	"Mongolian",
	"Nepalese",
	"Okinawan",
	"Pakistani",
	"Sri Lankan",
	"Taiwanese",
	"Thai",
	"Vietnamese",
	"Other",
	"Not Specified",
	"Mixed"
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

RESOURCES_DIR = "src\\main\\resources\\2020"

def write_to_one_file(data_files: List[str]):
    with open("src\\main\\core\\visualization\\counties.json", 'w', encoding='utf-8') as out:
        out.write("{\n")
        for data_file in data_files:
            filename = data_file.split("\\")[-1].split(".")[0].strip()
            out.write("\t\"" + filename + "\" : ")
            with open(data_file, 'r', encoding='utf-8') as file:
                for line in file:
                    out.write("\t" + line)
            out.write(",\n")
        out.write("}")

def main() -> None:
    all_files = list_files_recursive(RESOURCES_DIR)
    data_files = get_data_files(all_files)

    write_to_one_file(data_files)
    return

    for file in data_files:
        white_pct = -1.0
        hispanic_pct = -1.0
        black_pct = -1.0
        asian_pct = -1.0
        mixed_pct = -1.0
        other_pct = -1.0
        lines: List[str] = []
        with open(file, 'r', encoding='utf-8') as f:
            for line in f:
                lines.append(line)
                if "\"White\"" in line:
                    white_pct = float(line.split(":")[-1].replace(",","").strip())
                if "\"Hispanic\"" in line:
                    hispanic_pct = float(line.split(":")[-1].replace(",","").strip())
                if "\"Black\"" in line:
                    black_pct = float(line.split(":")[-1].replace(",","").strip())
                if "\"Asian\"" in line:
                    asian_pct = float(line.split(":")[-1].replace(",","").strip())
                if "\"Mixed\"" in line:
                    mixed_pct = float(line.split(":")[-1].replace(",","").strip())
                if "\"Other\"" in line:
                    other_pct = float(line.split(":")[-1].replace(",","").strip())
            if len([_ for _ in [white_pct, hispanic_pct, black_pct, asian_pct, mixed_pct, other_pct] if _ < 0.0]) > 0:
                print("Could not find a race/ethnicity percentage in " + file)
        for i, line in enumerate(lines):
            key = line.split(":")[0].replace("\"","").strip()
            try:
                value = float(line.split(":")[-1].replace(",","").strip())
            except ValueError:
                continue
            if key in european_mena_african_ancestries:
                lines[i] = line.split(":")[0] + ": " + str(value * (white_pct + black_pct + other_pct)) + ("," if "," in line else "") + "\n"
            elif key in hispanic_ancestries:
                lines[i] = line.split(":")[0] + ": " + str(value * (hispanic_pct)) + ("," if "," in line else "") +"\n"
            elif key in asian_ancestries:
                lines[i] = line.split(":")[0] + ": " + str(value * (asian_pct + mixed_pct)) + ("," if "," in line else "") +"\n"
        
        with open(file, 'w', encoding='utf-8') as f:
            for line in lines:
                f.write(line)        

if __name__ == "__main__":
    main()