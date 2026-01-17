from typing import List, Dict, Any
import os
import json

def read_json(filename: str) -> Dict[str, Any]:
    with open(filename, 'r', encoding='utf-8') as data:
        return json.load(data)
    return {}

def write_json(filename: str, data: Dict[str, Any]) -> None:
    with open(filename, 'w', encoding='utf-8') as out:
        out.write(json.dumps(data, separators=(',',' : '), indent='	'))

def list_files(directoryName: str, recurse: bool = True) -> List[str]:
    res: List[str] = []
    for elem in os.listdir(directoryName):
        path = directoryName + "\\" + elem
        if os.path.isdir(path) and recurse:
            for _ in list_files(path, recurse):
                res.append(_)
        else:
            res.append(directoryName + "\\" + elem)
    return res

def main() -> None:
    adjacencies = read_json("src\\main\\resources\\2020\\adjacencies.json")
    files = [_ for _ in list_files("src\\main\\resources\\2020") if _.find(".json") != -1]
    for file in files:
        jsonData = read_json(file)
        try:
            FIPS: str = jsonData["FIPS"]
            neighbors = adjacencies[FIPS]
            jsonData["neighbors"] = neighbors

            write_json(file, jsonData)
        except KeyError as e:
            continue

    print("Done!")

if __name__ == "__main__":
    main()