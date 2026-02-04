from typing import List, Dict, Tuple, Any
import json

def gather_county_and_descriptor_data(filepath: str) -> Tuple[Dict[str, Any], Dict[str, Any]]:
    county_data: Dict[str, Any] = {}
    descriptor_data: Dict[str, Any] = {}
    with open(filepath, 'r', encoding='utf-8') as data_file:
        json_data = json.load(data_file)
    county_data = json_data['counties']
    descriptor_data = json_data['descriptors']
    return (county_data, descriptor_data)

def gather_electoral_data(filepath: str) -> List[Dict[str, Any]]:
    electoral_data = []
    with open(filepath, 'r', encoding='utf-8') as data_file:
        value_line = data_file.readline().split(',');
        for row in data_file:
            row_data: Dict[str, Any] = {}
            for i, value in enumerate(row.split(',')):
                row_data[value_line[i]] = value
            electoral_data.append(row_data)
    return electoral_data

def main() -> None:
    county_data, descriptor_data = gather_county_and_descriptor_data('logs\\cpplog_17.json')

    counties: List[Dict[str, Any]] = [c for c in county_data.values()]
    descriptors: List[Dict[str, Any]] = [d for d in descriptor_data.values()]

    electoral_data = gather_electoral_data('src\\main\\resources\\countypres_2000-2024.csv')

    descriptor_candidate_vote_totals: Dict[str, Dict[str, int]] = {}
    descriptor_party_vote_totals: Dict[str, Dict[str, int]] = {}

    valid: int = 0
    total: int = 0
    for election in electoral_data:
        total += 1
        try:
            countyFIPS = ('0' + str(int(float(election['county_fips']))))
            countyFIPS = countyFIPS[len(countyFIPS) - 5 :]
        except ValueError as e:
            # print(election)
            # print(e)
            continue
        try:
            county = [c for c in counties if c['FIPS'] == countyFIPS][0]
        except IndexError as e:
            # print(countyFIPS)
            # print(e)
            continue
        valid += 1

        candidate = election['candidate']
        if candidate == 'OVERVOTES' or candidate == 'UNDERVOTES' or candidate == 'OTHER': continue
        party = election['party']
        if party == '' or party == 'OTHER': continue
        votes = int(election['candidatevotes'])
        for descriptor in county['descriptors']:
            if descriptor not in descriptor_candidate_vote_totals: descriptor_candidate_vote_totals[descriptor] = {}
            if candidate not in descriptor_candidate_vote_totals[descriptor]: descriptor_candidate_vote_totals[descriptor][candidate] = 0
            descriptor_candidate_vote_totals[descriptor][candidate] += votes
            if descriptor not in descriptor_party_vote_totals: descriptor_party_vote_totals[descriptor] = {}
            if party not in descriptor_party_vote_totals[descriptor]: descriptor_party_vote_totals[descriptor][party] = 0
            descriptor_party_vote_totals[descriptor][party] += votes
            
    print(f"Valid: {valid} Total: {total} ({valid / total})")
    
    for k, v in descriptor_candidate_vote_totals.items():
        print(f"{k}: {v}")

    print("\n---\n")

    for k, v in descriptor_party_vote_totals.items():
        print(f"{k}: {v}")

if __name__ == '__main__':
    main()