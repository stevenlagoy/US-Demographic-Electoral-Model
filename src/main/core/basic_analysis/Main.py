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

    # Precompute demographic percentages for all counties and all demographics
    demographic_percentages: Dict[str, Dict[str, float]] = {}
    for demographic in demographics:
        demographic_percentages[demographic] = get_demographic_percentages(counties, demographic)

    # Create combinations of demographics
    demographic_combos = list(combinations(demographics, combo_size))
    demographics_electoral: Dict[Tuple[str, ...], Dict[str, float]] = {combo: {} for combo in demographic_combos}

    # Determine average voting results for each demographic combination
    for idx, combo in enumerate(demographic_combos):
        if idx % 10 == 0:
            print(f"Processing combo {idx+1}/{len(demographic_combos)}")
        # For each county, calculate the combined percentage for this combo
        for FIPS in counties:
            if len(FIPS) != 5 or FIPS not in elections or FIPS in ["15005"]:
                continue
            # For each demographic in the combo, get the percentage for this county
            combo_percent = 1.0
            for demographic in combo:
                percent = demographic_percentages[demographic].get(FIPS, 0.0)
                combo_percent *= percent
            if combo_percent == 0.0:
                continue
            electoral = elections[FIPS]
            for year, result in electoral.items():
                for _return in result:
                    party = _return['party']
                    votes = _return['votes']
                    demographics_electoral[combo].setdefault(party, 0.0)
                    demographics_electoral[combo][party] += (votes * combo_percent) / total_votes
    print('Demographics-Electoral behaviors processed')

    # Get average voting result for the nation across all years
    nation_totals: Dict[str, float] = {}
    for FIPS, county in counties.items():
        if FIPS not in elections or len(FIPS) != 5: continue
        election = elections[FIPS]
        for year, _returns in election.items():
            for _return in _returns:
                if _return['party'] not in nation_totals:
                    nation_totals[_return['party']] = 0
                nation_totals[_return['party']] += int(_return['votes'])
    total_votes: int = 0
    for party, votes in nation_totals.items():
        total_votes += int(votes)
    for party, votes in nation_totals.items():
        nation_totals[party] /= total_votes
    
    # Only consider the top two parties by national vote share
    sorted_parties = sorted(nation_totals.items(), key=lambda x: x[1], reverse=True)
    top_parties = [p for p, _ in sorted_parties[:2]]

    accuracy: float = 0.0
    count: int = 0
    for FIPS, county in counties.items():
        if FIPS not in elections or len(FIPS) != 5: continue
        for year, results in elections[FIPS].items():
            year_total_votes = sum(r['votes'] for r in results)
            if year_total_votes == 0:
                continue
            actual_dist = {}
            for result in results:
                party = result['party']
                actual_dist[party] = result['votes'] / year_total_votes
            # Only consider top two parties
            mae = 0.0
            for party in top_parties:
                actual = actual_dist.get(party, 0.0)
                prediction = nation_totals.get(party, 0.0)
                mae += abs(prediction - actual)
            mae /= len(top_parties)
            accuracy += mae
            count += 1
    if count > 0:
        accuracy /= count
    print(f'Mean Error from National Average (top 2 parties): {accuracy}')
    
    # Use numpy for fast MAE calculation
    import numpy as np
    # Precompute all unique parties across all predictions
    all_parties = set()
    for combo in demographic_combos:
        all_parties.update(demographics_electoral[combo].keys())
    all_parties = sorted(all_parties)
    party_idx = {p: i for i, p in enumerate(all_parties)}

    # Precompute actual party distributions for all county-years
    actual_distributions = []  # List of (FIPS, year, np.array of party shares)
    for FIPS, county in counties.items():
        if FIPS not in elections or len(FIPS) != 5:
            continue
        for year, results in elections[FIPS].items():
            year_total_votes = sum(r['votes'] for r in results)
            if year_total_votes == 0:
                continue
            arr = np.zeros(len(all_parties))
            for result in results:
                party = result['party']
                arr[party_idx[party]] = result['votes'] / year_total_votes
            actual_distributions.append((FIPS, year, arr))

    # For each combo, compute MAE using vectorized numpy and correct demographic product logic
    # Calculate the average MAE across all demographic combinations
    n_combos = len(demographic_combos)
    total_mae = 0.0
    valid_combos = 0
    for idx, combo in enumerate(demographic_combos):
        if idx % 10 == 0:
            print(f"Evaluating combo {idx+1}/{n_combos}")
        combo_mae = 0.0
        count = 0
        # Precompute demographic percentages for this combo for all counties
        combo_percentages = {}
        for FIPS in counties:
            if len(FIPS) != 5 or FIPS not in elections or FIPS == "15005":
                continue
            percent = 1.0
            for demographic in combo:
                percent *= demographic_percentages[demographic].get(FIPS, 0.0)
            combo_percentages[FIPS] = percent

        for FIPS, year, actual_arr in actual_distributions:
            if FIPS not in combo_percentages or combo_percentages[FIPS] == 0.0:
                continue
            predicted = np.zeros(len(all_parties))
            for i, party in enumerate(all_parties):
                predicted[i] = demographics_electoral[combo].get(party, 0.0) * combo_percentages[FIPS]
            pred_sum = predicted.sum()
            if pred_sum > 0:
                predicted /= pred_sum
            mae = np.mean(np.abs(predicted - actual_arr))
            combo_mae += mae
            count += 1
        if count > 0:
            combo_mae /= count
            total_mae += combo_mae
            valid_combos += 1
    avg_mae = total_mae / valid_combos if valid_combos > 0 else float('inf')
    print(f'Average mean absolute error across all combos: {avg_mae}')

    print("Done!")

if __name__ == "__main__":
    main()