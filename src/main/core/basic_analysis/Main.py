from typing import List, Dict, Tuple, Any
from itertools import combinations
import json

def flatten_dict(dict: dict) -> Dict[str, float]:
    result: Dict[str, float] = {}
    for k, v in dict.items():
        key = k
        if isinstance(v, float):
            result[key] = float(v)
        elif isinstance(v, Dict):
            nested = flatten_dict(v)
            for kk, vv in nested.items():
                key_nested = k + "->" + kk
                result[key_nested] = float(vv)
        else:
            result[key] = float(v) if isinstance(v, (int, float)) else 0.0
    return result

def read_county_data() -> Dict[str, Any]:
    raw = json.load(open('src\\main\\core\\basic_analysis\\counties.json', 'r', encoding='utf-8'))
    normalized: Dict[str, Any] = {}
    
    for FIPS, county in raw.items():
        if len(FIPS) != 5: continue
        normalized[FIPS] = {}
        normalized[FIPS]['name'] = raw[FIPS]['name']
        normalized[FIPS]['FIPS'] = raw[FIPS]['FIPS']
        normalized[FIPS]['population'] = raw[FIPS]['population']
        normalized[FIPS]['demographics'] = {}

        for category in raw[FIPS]['demographics']:
            normalized[FIPS]['demographics'][category] = {}
            for demographic in raw[FIPS]['demographics'][category]:
                if isinstance(raw[FIPS]['demographics'][category][demographic], Dict):
                    normalized[FIPS]['demographics'][category] = flatten_dict(raw[FIPS]['demographics'][category])
                else:
                    normalized[FIPS]['demographics'][category][demographic] = raw[FIPS]['demographics'][category][demographic]
    return normalized

def read_electoral_data() -> Dict[str, Any]:
    return json.load(open('src\\main\\core\\basic_analysis\\elections.json', 'r', encoding='utf-8'))

def get_demographic_percentages(counties: Dict[str, Any], demographic: str) -> Dict[str, float]:
    """ Get the percentage of each county identifying with the given demographic. Returns a dict of FIPS codes to percentages. """
    res: Dict[str, float] = {}
    for FIPS, county_data in counties.items():
        demographics: Dict[str, Any] = county_data['demographics']
        for category in demographics.keys():
            if demographic in demographics[category]:
                res[FIPS] = demographics[category][demographic]
                break
        else:
            # print(f'Could not find demographic {demographic} for {FIPS}')
            continue
    return res

def main() -> None:
    
    # How many demographics are included in each combination
    # O(#demographics ^ combo_size) combinations will be made
    combo_size: int = 2

    # Read in county and electoral data
    counties: Dict[str, Any] = read_county_data()
    elections: Dict[str, Any] = read_electoral_data()
    total_votes: int = 0
    for FIPS, electoral in elections.items():
        for year, _returns in electoral.items():
            for _return in _returns:
                total_votes += int(_return['votes'])

    # Determine demographics data from county data
    categories: List[str] = counties['01001']['demographics'].keys()
    demographics: List[str] = []
    for category in categories:
        for d in counties['01001']['demographics'][category]:
            demographics.append(d)

    # Create combinations of demographics
    demographic_combos = list(combinations(demographics, combo_size))
    demographics_electoral: Dict[Tuple[str, ...], Dict[str, float]] = {combo: {} for combo in demographic_combos}

    # Determine average voting results for each demographic combination
    i: int = 0
    for combo in demographic_combos:
        i += 1
        if not i % 10: print(i)
        for demographic in combo:
            county_percentages: Dict[str, float] = get_demographic_percentages(counties, demographic)
            for FIPS, percentage in county_percentages.items():
                if len(FIPS) != 5: continue # Only look at counties (states are composed of counties)
                if FIPS in ["15005"]: continue # Kalawao County, HI does not have electoral history in the dataset 
                electoral = elections[FIPS]
                for year, result in electoral.items():
                    for _return in result:
                        party = _return['party']
                        votes = _return['votes']
                        demographics_electoral[combo].setdefault(party, 0.0)
                        demographics_electoral[combo][party] += (votes * percentage) / total_votes
    print('Demographics-Electoral behaviors processed')

    # Determine accuracy of the demographic combination averages
    accuracy: float = 0.0
    count: int = 0
    for FIPS, county in counties.items():
        if FIPS not in elections or len(FIPS) != 5: continue
        # Determine expected electoral results based on demographics
        # For each demographic combination, predict party votes
        for combo in demographic_combos:
            predicted: Dict[str, float] = {}
            for party in demographics_electoral[combo]:
                prediction = 0.0
                for demographic in combo:
                    percentage = 0.0
                    for category in county['demographics']:
                        if demographic in county['demographics'][category]:
                            percentage = county['demographics'][category][demographic]
                            break
                    prediction += demographics_electoral[combo][party] * percentage
                predicted[party] = prediction
            # Compare to real electoral results for each year
            for year, results in elections[FIPS].items():
                for result in results:
                    party = result['party']
                    year_total_votes = sum(r['votes'] for r in results)
                    actual = result['votes'] / year_total_votes if year_total_votes > 0 else 0.0
                    prediction = predicted.get(party, 0.0)
                    accuracy += abs(prediction - actual)
                    count += 1
    if count > 0:
        accuracy /= count
    print(f'Mean absolute error: {accuracy}')

    print("Done!")

if __name__ == "__main__":
    main()