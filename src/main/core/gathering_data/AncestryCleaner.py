from typing import Any, List, Set
import json
import os

def read_json(file_name: str) -> Any:
    with open(file_name, 'r', encoding='utf-8') as data:
        return json.load(data)

def each_file(directory: str, recurse = False):
    items = os.listdir(directory)
    for item in items:
        full_path = os.path.join(directory, item)
        if os.path.isfile(full_path):
            yield full_path
        elif os.path.isdir(full_path) and recurse:
            for f in each_file(full_path, recurse):
                yield f

def list_ancestries(json) -> List[str] | None:
    try:
        ancestries = [k for k in json["demographics"]["ancestry"]]
        return ancestries
    except KeyError as e:
        return None

def add_ancestries(json, to_add: Set[str], value = 0.0):
    try:
        existing = [k for k in json["demographics"]["ancestry"]]
        for a in to_add:
            if a not in existing:
                json["demographics"]["ancestry"][a] = value
        return json
    except KeyError as e:
        return None

def main() -> None:
    resources_dir = "src\\main\\resources\\2020"

    all_ancestries = set()
    for f in each_file(resources_dir, True):
        ancestries = list_ancestries(read_json(f))
        if ancestries is not None:
            for a in ancestries: all_ancestries.add(a)

    for f in each_file(resources_dir, True):
        cleaned = add_ancestries(read_json(f), all_ancestries)
        if cleaned is not None:
            with open(f, 'w', encoding='utf-8') as out:
                out.write(json.dumps(cleaned, separators=(","," : "), indent="	"))

    print("Done!")
    return

if __name__ == "__main__":
    main()