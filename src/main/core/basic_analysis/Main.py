'''
Steven LaGoy
basic_analysis/Main.py
Created: 15 October 2025
Modified: 15 October 2025

This script performs some basic analysis of county demographic information and electoral history
'''

import sqlite3
import json
from typing import Any, Dict

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

def main() -> None:
    electoral_history_db = 'src\\main\\core\\basic_analysis\\electoral_history.db'
    counties_data = 'src\\main\\core\\basic_analysis\\counties.json'

    conn = sqlite3.connect(electoral_history_db)
    cursor = conn.cursor()

    counties: Dict[str, Any] = {}
    with open(counties_data, 'r') as data:
        counties = json.loads(data.read())

    parties = [row[0] for row in cursor.execute("SELECT DISTINCT party FROM results_t WHERE party IS NOT NULL AND party <> '';").fetchall()]
    parties.sort()

    county_results: Dict[str, Dict[str, int]] = {}

    cursor.execute("""
        SELECT
            c.fips,
            c.name AS county_name,
            s.name AS state_name,
            r.party,
            sum(r.votes) AS total_votes
        FROM results_t r
            JOIN counties_t c ON r.county = c.fips
            JOIN states_t s ON c.state = s.fips
        WHERE r.party <> ''
        GROUP BY c.fips, r.party
        ORDER BY c.fips, total_votes DESC;
    """)
    for row in cursor.fetchall():
        fips: str = row[0]
        party: str = row[3]
        votes: int = row[4]
        county_results.setdefault(fips, {})
        county_results[fips][party] = votes

    county_demographics: Dict[str, Dict[str, float]] = {}
    demographic_totals: Dict[str, int] = {}

    for county, data in counties.items():
        if len(county) != 5:
            continue
        FIPS = data['FIPS']
        population = data['population']
        demographics = flatten_dict(data['demographics'])
        county_demographics[FIPS] = demographics
        for bloc, percent in demographics.items():
            demographic_totals.setdefault(bloc, 0)
            demographic_totals[bloc] += population * percent
    for bloc, number in demographic_totals.items():
        demographic_totals[bloc] = round(number)

    demographics_to_parties: Dict[str, Dict[str, float]] = {bloc: {} for bloc in demographic_totals}
    nationwide_totals: Dict[str, float] = {party: 0.0 for party in parties}

    for fips, data in county_demographics.items():
        try:
            # Get election results
            results = county_results[fips]
            # Get demographics
            for bloc, percentage in data.items():
                for party in results:
                    demographics_to_parties[bloc].setdefault(party, 0.0)
                    demographics_to_parties[bloc][party] += results[party] * percentage
                    nationwide_totals[party] += results[party]
        except KeyError as e:
            print('KeyError: ' + fips)
            pass

    for bloc, total in demographic_totals.items():
        # Bloc is the name of a bloc
        # total is the number of bloc members nationwide
        for demographic, vote in demographics_to_parties.items():
            # Demographic is name of bloc
            # Vote is map of party name to total votes by the demographic
            if (bloc == demographic):
                for party in vote:
                    demographics_to_parties[demographic][party] /= total

    # Normalize
    for bloc, vote in demographics_to_parties.items():
        total_votes = sum(vote.values())
        if total_votes > 0:
            for party in vote:
                vote[party] /= total_votes

    total_votes_nationwide = sum(nationwide_totals.values())
    nationwide_shares = {party: votes / total_votes_nationwide for party, votes in nationwide_totals.items()}

    with open('src\\main\\core\\basic_analysis\\data.out', 'w') as out:
        out.write("Nation\n")
        for party in parties:
            out.write("\t" + party + ": " + str(nationwide_shares.get(party, 0.0)) + "\n")
        for demographic, vote in demographics_to_parties.items():
            out.write(demographic + "\n")
            for party in parties:
                out.write("\t" + party + ": " + str(vote.get(party, 0.0)) + "\n")

    print(demographic_totals)

    conn.close()

if __name__ == "__main__":
    main()