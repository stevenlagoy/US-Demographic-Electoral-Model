from typing import Dict, Any
import json

def read_descriptor_data() -> Dict[str, Any]:
    return json.load(open('src\\main\\core\\analyzing_descriptors\\descriptors_18.json', 'r', encoding='utf-8'))['descriptors']

def read_electoral_data() -> Dict[str, Any]:
    return json.load(open('src\\main\\core\\analyzing_descriptors\\elections.json', 'r', encoding='utf-8'))

def main() -> None:

    # Read descriptor and electoral data
    descriptors = read_descriptor_data()
    elections = read_electoral_data()

    # Total electoral data for counties
    county_electoral_totals: Dict[str, Dict[str, int]] = {}
    for FIPS, election in elections.items():
        for year, _returns in election.items():
            for _return in _returns:
                party = _return['party']
                votes = _return['votes']
                if FIPS not in county_electoral_totals:
                    county_electoral_totals[FIPS] = {}
                if party not in county_electoral_totals[FIPS]:
                    county_electoral_totals[FIPS][party] = 0
                county_electoral_totals[FIPS][party] += votes
    
    # Average electoral data for descriptors by county members
    descriptor_electoral_totals: Dict[str, Dict[str, float]] = {}
    for descriptor, data in descriptors.items():
        if descriptor not in descriptor_electoral_totals:
            descriptor_electoral_totals[descriptor] = {}
        for FIPS in data['members']:
            if FIPS in ['15005']: continue # Kalawao County, HI does not have electoral data
            electoral = county_electoral_totals[FIPS]
            for party, votes in electoral.items():
                if party not in descriptor_electoral_totals:
                    descriptor_electoral_totals[descriptor][party] = 0
                descriptor_electoral_totals[descriptor][party] += int(votes)
        desc_total_votes = 0
        for party, votes in descriptor_electoral_totals[descriptor].items():
            desc_total_votes += votes
        for party in descriptor_electoral_totals[descriptor].keys():
            descriptor_electoral_totals[descriptor][party] /= desc_total_votes

    # Compare counties' composite demographic electoral result predictions to actual (proportions)
    mae: float = 0.0
    count: int = 0
    for FIPS, results in county_electoral_totals.items():
        # Get the county's descriptors
        descriptor_memberships = [name for name, descriptor in descriptors.items() if FIPS in descriptor['members']]
        if not descriptor_memberships:
            continue
        # Predicted party proportions (average of descriptor proportions)
        predicted_electoral = {}
        for descriptor in descriptor_memberships:
            for party, prop in descriptor_electoral_totals[descriptor].items():
                predicted_electoral.setdefault(party, 0.0)
                predicted_electoral[party] += (prop / len(descriptor_memberships))
        # Actual party proportions
        total_votes = sum(results.values())
        if total_votes == 0:
            continue
        actual_proportions = {party: votes / total_votes for party, votes in results.items()}
        # Compare predicted and actual proportions
        for party, predicted in predicted_electoral.items():
            if party not in actual_proportions:
                continue
            actual = actual_proportions[party]
            mae += abs(predicted - actual)
            count += 1
    if count > 0:
        mae /= count
    print(f'Mean absolute error across all descriptors: {mae}')


if __name__ == "__main__":
    main()