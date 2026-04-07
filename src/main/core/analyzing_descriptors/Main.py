from typing import Dict, Any
import json
import numpy as np

def read_descriptor_data() -> Dict[str, Any]:
    return json.load(open('src\\main\\core\\analyzing_descriptors\\descriptors_18.json', 'r', encoding='utf-8'))['descriptors']

def read_electoral_data() -> Dict[str, Any]:
    return json.load(open('src\\main\\core\\analyzing_descriptors\\elections.json', 'r', encoding='utf-8'))

def main() -> None:

    descriptors = read_descriptor_data()
    elections = read_electoral_data()

    # Keep electoral totals separated by year
    county_electoral_by_year: Dict[str, Dict[str, Dict[str, int]]] = {}
    for FIPS, election in elections.items():
        for year, _returns in election.items():
            for _return in _returns:
                party = _return['party']
                votes = _return['votes']
                county_electoral_by_year.setdefault(FIPS, {})
                county_electoral_by_year[FIPS].setdefault(year, {})
                county_electoral_by_year[FIPS][year].setdefault(party, 0)
                county_electoral_by_year[FIPS][year][party] += votes

    # Descriptor electoral totals aggregated across all years
    descriptor_electoral_totals: Dict[str, Dict[str, float]] = {}
    for descriptor, data in descriptors.items():
        descriptor_electoral_totals[descriptor] = {}
        for FIPS in data['members']:
            if FIPS in ['15005']: continue
            if FIPS not in county_electoral_by_year: continue
            for year, year_results in county_electoral_by_year[FIPS].items():
                for party, votes in year_results.items():
                    descriptor_electoral_totals[descriptor].setdefault(party, 0)
                    descriptor_electoral_totals[descriptor][party] += int(votes)
        desc_total_votes = sum(descriptor_electoral_totals[descriptor].values())
        if desc_total_votes > 0:
            for party in descriptor_electoral_totals[descriptor]:
                descriptor_electoral_totals[descriptor][party] /= desc_total_votes

    # Precompute descriptor memberships for each county
    descriptor_memberships: Dict[str, list] = {}
    for descriptor, data in descriptors.items():
        for FIPS in data['members']:
            descriptor_memberships.setdefault(FIPS, [])
            descriptor_memberships[FIPS].append(descriptor)

    # Evaluate MAE per county-year
    mae: float = 0.0
    count: int = 0
    mae_by_year: Dict[str, float] = {}
    count_by_year: Dict[str, int] = {}

    for FIPS, years in county_electoral_by_year.items():
        if FIPS not in descriptor_memberships:
            continue
        memberships = descriptor_memberships[FIPS]

        # Predicted proportions: average across all descriptors this county belongs to
        predicted_electoral: Dict[str, float] = {}
        for descriptor in memberships:
            for party, prop in descriptor_electoral_totals[descriptor].items():
                predicted_electoral.setdefault(party, 0.0)
                predicted_electoral[party] += prop / len(memberships)

        for year, year_results in years.items():
            total_votes = sum(year_results.values())
            if total_votes == 0:
                continue
            actual_proportions = {
                party: votes / total_votes
                for party, votes in year_results.items()
            }

            # FIXED: compute mean over all parties for this county-year,
            # then increment count once per county-year (not once per party).
            all_parties = set(predicted_electoral) | set(actual_proportions)
            party_mae = np.mean([
                abs(predicted_electoral.get(party, 0.0) - actual_proportions.get(party, 0.0))
                for party in all_parties
            ])

            mae += float(party_mae)
            count += 1
            mae_by_year.setdefault(year, 0.0)
            count_by_year.setdefault(year, 0)
            mae_by_year[year] += float(party_mae)
            count_by_year[year] += 1

    if count > 0:
        mae /= count
    print(f'Mean absolute error across all descriptors (per county-year): {mae:.6f}  ({mae * 100:.4f} pp)')
    print(f'Total county-year observations: {count}')

    print('\nPer-year breakdown:')
    for year in sorted(mae_by_year.keys()):
        year_mae = mae_by_year[year] / count_by_year[year]
        print(f'  {year}: {year_mae:.6f}  ({year_mae * 100:.4f} pp)  (n={count_by_year[year]})')

if __name__ == "__main__":
    main()