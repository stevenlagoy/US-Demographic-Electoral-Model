from typing import List, Dict, Any, Set, Tuple
import json
import os
import numpy as np
import heapq

from Utils import flatten_dict
from HasDemographics import HasDemographics
from County import County
from CountySet import CountySet

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
    files = [_ for _ in list_files("src\\main\\resources\\2020") if _.find('.json') != -1]
    counties: Dict[str, County] = {}
    neighbors: Dict[County, List[str]] = {}
    FIPSCodes: Set[str] = set()
    for file in files:
        countyJson = read_json(file)
        try:
            FIPS: str = countyJson['FIPS']
            if len(FIPS) < 3 or FIPS in FIPSCodes: continue # Check for county FIPS
            FIPSCodes.add(FIPS) # Make sure no duplicate counties are made
            name: str = countyJson['name']
            population: int = countyJson['population']
            demos = flatten_dict(countyJson['demographics'])
            county = County(name, FIPS, population, demos)
            counties[FIPS] = (county)
            neighbors[county] = countyJson['neighbors']
        except KeyError as e:
            continue
    
    # Resolve county neighbors
    for county, neighborsFIPS in neighbors.items():
        # Find the county with the neighbor FIPS
        for neighborFIPS in neighborsFIPS:
            if neighborFIPS not in counties:
                print(f"{neighborFIPS} was not found")
                continue
            neighbor: County = counties[neighborFIPS]
            county.neighbors.add(neighbor)
            neighbor.neighbors.add(county) # County relationships are bidirectional / mutual
    
    # Renormalize county demographics
    # Use square of difference from national average demographics
    # Find national average
    print("Normalizing county demographics...")
    national_demographics: Dict[str, float] = {}
    national_population: int = 0
    for FIPS, county in counties.items():
        national_population += county.population
        for demographic, value in county.demographics.demographics.items():
            national_demographics.setdefault(demographic, 0.0)
            national_demographics[demographic] += value * county.population
    for demographic in national_demographics:
        national_demographics[demographic] /= national_population
    # Recompute county demographics as square of difference
    for FIPS, county in counties.items():
        for demographic, value in county.demographics.demographics.items():
            diffsq = (value - national_demographics[demographic]) ** 2
            county.demographics.demographics[demographic] = diffsq

    # Read county similarities
    try:
        with open('src\\main\\core\\assigning_descriptors\\py2\\similarities.json', 'r', encoding='utf-8') as similarities_file:
            similarities_data: Dict[str, Dict[str, float]] = json.load(similarities_file)
            for FIPS, sims in similarities_data.items():
                similarities: Dict[HasDemographics, float] = {}
                for o_FIPS, sim in sims.items():
                    similarities[counties[o_FIPS]] = sim
                counties[FIPS].similarities = similarities
    except FileNotFoundError as e:
        precompute_county_similarities([v for _, v in counties.items()])

    return [v for _, v in counties.items()]

def precompute_county_similarities(counties: List[County]) -> None:

    print("Computing county similarities...")

    for i, c1 in enumerate(counties):
        if i % 50 == 0: print(i)
        for j, c2 in enumerate(counties):
            if i <= j: continue # Lower triangle
            c1.compare_to(c2) # Comparison is symmetric, cached in both c1 & c2

    # Prepare json to write
    json_data: Dict[str, Any] = json.loads('{}')
    for c in counties:
        json_data[c.FIPS] = {o.FIPS: sim for o, sim in c.similarities.items() if isinstance(o, County)}

    # Write json
    with open('src\\main\\core\\assigning_descriptors\\py2\\similarities.json', 'w', encoding='utf-8') as out:
        json.dump(json_data, out, indent='	', separators=(', ', ' : '))

def main():

    # L1
    # MEAN_SIMILARITY = 0.852854096943476
    # STDDEV_SIMILARITY = 0.04941103134279599

    # L2
    MEAN_SIMILARITY = 0.5267051127808406
    STDDEV_SIMILARITY = 0.14870511593212166
    SIMILARITY_THRESHOLD = MEAN_SIMILARITY + (0.5 * STDDEV_SIMILARITY)

    DESIRED_GROUPS = 1

    print("Creating counties...")
    counties: List[County] = create_counties() # Treat counties as immutable
    # Create initial singleton county sets
    countySets: List[CountySet] = [CountySet(i, c) for i, c in enumerate(counties)]
    print("Counties created!")

    # Build an initial heap of pairwise similarities
    print("Building similarities heap...")
    heap = [] # Max heap of similarities
    counter: int = 0 # Tie-breaker
    for i, cs1 in enumerate(countySets):
        if (i % (len(countySets) // 10) == 0): print(f"{round(i * 100 / len(countySets))}%")
        for j in range(i):
            cs2 = countySets[j]
            sim = cs1.compare_to(cs2)
            heapq.heappush(heap, (-sim, counter, cs1, cs2)) # Negative sim for max heap
            counter += 1

    iteration: int = 0
    # K-Means Cluster counties with k = DESIRED_GROUPS
    while not len(countySets) < DESIRED_GROUPS:

        # Find similarity scores between each pair of sets
        max_sim: Tuple[Tuple[int, int], float] = ((0, 0), 0.0)

        # print("Finding max sim")
        while heap:
            # Pop the highest similarity pair
            neg_sim, _, cs1, cs2 = heapq.heappop(heap)
            sim = -neg_sim
            if cs1 in countySets and cs2 in countySets:
                break # Check validity
        else:
            break # Heap is empty

        # Finish if the maximum similarity is lower than the acceptable threshold
        if sim < SIMILARITY_THRESHOLD: break

        new_cs = CountySet(min(cs1.group_num, cs2.group_num), *cs1.counties, *cs2.counties)
        countySets.append(new_cs)
        countySets.remove(cs1)
        countySets.remove(cs2)

        # Update heap with similarities between new_cs and others
        for cs in countySets[:-1]:
            new_sim = new_cs.compare_to(cs)
            heapq.heappush(heap, (-new_sim, counter, new_cs, cs))
            counter += 1

        print(f"{iteration} (k={len(countySets)}): ({cs1.group_num, cs2.group_num}) {sim}")

        iteration += 1

    # Print the results
    json_data: Dict[str, Dict[str, Any]] = {}
    for cs in countySets:
        set_name = str(cs.group_num)
        members = [c.FIPS for c in cs.counties]
        demos: Dict[str, float] = {}
        for FIPS, val in cs.demographics.demographics.items():
            demos[FIPS] = val
        json_data[set_name] = ({'num_members': len(members), 'members': members, 'demographics': demos})

    with open('src\\main\\core\\assigning_descriptors\\py2\\results.json', 'w', encoding='utf-8') as out:
        json.dump(json_data, out, indent='	', separators=(', ', ' : '))
    
    print("Done!")

if __name__ == "__main__":
    main()