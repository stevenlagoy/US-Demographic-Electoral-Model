from typing import List, Dict, Any, Set, Tuple
import json
import os
import concurrent.futures
import numpy as np
from Utils import flatten_dict
from County import County

def read_json(filename: str) -> Dict[str, Any]:
    with open(filename, 'r', encoding='utf-8') as data:
        return json.load(data)
    return {}

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

def create_counties() -> List[County]:
    files = [_ for _ in list_files("src\\main\\resources\\2020") if _.find(".json") != -1]
    counties: Dict[str, County] = {}
    neighbors: Dict[County, List[str]] = {}
    FIPSCodes: Set[str] = set()
    for file in files:
        countyJson = read_json(file)
        try:
            FIPS: str = countyJson["FIPS"]
            if len(FIPS) < 3 or FIPS in FIPSCodes: continue # Check for county FIPS
            FIPSCodes.add(FIPS) # Make sure no duplicate counties are made
            name: str = countyJson["name"]
            population: int = countyJson["population"]
            demos = flatten_dict(countyJson["demographics"])
            county = County(name, FIPS, population, demos)
            counties[FIPS] = (county)
            neighbors[county] = countyJson["neighbors"]
        except KeyError as e:
            continue
    
    # Resolve county neighbors
    for county, neighborsFIPS in neighbors.items():
        # Find the county with the neighbor FIPS
        for neighborFIPS in neighborsFIPS:
            if neighborFIPS not in counties:
                print(neighborFIPS + " was not found")
                continue
            neighbor: County = counties[neighborFIPS]
            county.neighbors.add(neighbor)
            neighbor.neighbors.add(county) # County relationships are bidirectional / mutual
    
    return [v for _, v in counties.items()]

def main() -> None:
    counties = create_counties()
    vectors = np.stack([c.demographics.as_list() for c in counties])
    n = len(counties)
    similarities = np.zeros((n, n), dtype=np.float64)
    # Normalize vectors
    vectors = vectors / np.clip(vectors.sum(axis=1, keepdims=True), 1e-12, None)

    # Compute pairwise Jensen-Shannon similarities
    for i in range(n):
        vi = vectors[i]
        for j in range(i):
            if (i % 50 == 0 and j == 0): print(i)
            vj = vectors[j]
            m = (vi + vj) / 2
            kl1 = np.sum(vi * np.log2(np.clip(vi / np.clip(m, 1e-12, None), 1e-12, None)))
            kl2 = np.sum(vj * np.log2(np.clip(vj / np.clip(m, 1e-12, None), 1e-12, None)))
            js = (kl1 + kl2) / 2
            sim = 1 - js
            similarities[i, j] = np.clip(sim, 0.0, 1.0)
    
    # Sort values from highest to lowest
    values: Dict[Tuple[int, int], float] = {}

    k = 500 # Groups

    print("Done!")

if __name__ == "__main__":
    main()