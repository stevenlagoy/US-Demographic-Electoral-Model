'''
Steven LaGoy
Created: 14 October 2025
Modified: 14 October 2025
ElectionStats.py

This file contains functions to analyze electoral history statistics.
History is held as a Dict[str, Any] and can be made with read_history(history_file: str) -> Dict[str, Any]
Functions include those to analyze at a national, state, and county level. Can sort by year, party, or candidate.

History:
{ '{FIPS}': { 'state': str, 'county': str, 'fips': str[5], 'results': {'{YEAR}': {'total_votes': int, 'result': [{'party': str, 'candidate': str, 'votes': int}, ...]}}, ...}, ...}

County
name: str
state: State
fips: str[5]

State
name: str
fips: str[2]

Get counties in a state: [county for county in counties if county.state == states['01']]

Nationwide vote total for each year
Statewide vote total for each year
    for specific state
County vote totals for each year
    for specific county
Nationwide votes for each candidate for each year
Statewide votes for each candidate for each year
    for specific state
County votes for each candidate for each year
    for specific county
Nationwide votes for each party for each year
Statewide votes for each party for each year
    for specific state
County votes for each party for each year
    for specific county
'''

import json
from typing import Any, Dict, List

def read_history(history_file: str) -> Dict[str, Any]:
    ''' Read the electoral history file. '''
    with open(history_file, 'r') as data:
        contents = data.read()
    history = json.loads(contents)
    return history

def national_total_votes(history: Dict[str, Any]) -> Dict[int, int]:
    ''' Get the national votes totals by year for the supplied history. '''
    totals: Dict[int, int] = {}
    for fips, value in history.items():
        for year, results in value['results'].items():
            totals.setdefault(int(year), 0)
            totals[int(year)] += int(results['total_votes'])
    return totals

def states_total_votes(history: Dict[str, Any]) -> Dict[str, Dict[int, int]]:
    ''' Get the vote totals by year sorted by state for the supplied history. '''
    totals: Dict[str, Dict[int, int]] = {}
    for fips, value in history.items():
        state = value['state']
        totals.setdefault(state, {})
        for year, result in value['results'].items():
            totals[state].setdefault(year, 0)
            totals[state][year] += result['total_votes']
    return totals

def state_total_votes(state: str, history: Dict[str, Any]) -> Dict[int, int]:
    ''' Get the vote totals by year for a specific state for the supplied history. '''
    all_results = states_total_votes(history)
    for s, result in all_results.items():
        if s == state:
            return result
    return {}

def year_total_votes(year: int, history: Dict[str, Any]) -> Dict[str, int]:
    ''' Get the vote totals by state in a specific year for the supplied history. '''
    totals: Dict[str, int] = {}
    all_results = states_total_votes(history)
    for s, result in all_results.items():
        for y, votes in result.items():
            if int(y) == year:
                totals[s] = votes
    return totals

def candidate_states_votes(candidate: str, history: Dict[str, Any]) -> Dict[int, Dict[str, int]]:
    ''' Get the vote total for a specific candidate by state by year for the supplied history. <br>
        Only years in which the candidate received votes will be included. All states will be included in each record. '''
    totals: Dict[int, Dict[str, int]] = {}
    for fips, value in history.items():
        state = value['state']
        for year, details in value['results'].items():
            for result in details['result']:
                if result['candidate'] == candidate:
                    totals.setdefault(year, {})
                    totals[year].setdefault(state, 0)
                    totals[year][state] += result['votes']
    return totals

def candidate_state_votes(candidate: str, state: str, history: Dict[str,Any]) -> Dict[int, int]:
    ''' Get the vote total for a specific candidate in a specific state by year for the supplied history. <br>
        Only years in which the candidate received votes will be included. '''
    totals: Dict[int, int] = {}
    for fips, value in history.items():
        if value['state'] == state:
            ...
    return totals

def main() -> None:
    history_file = "src\\main\\core\\gathering_data\\electoral_history.json"
    history = read_history(history_file)
    print(f"{national_total_votes(history)}")
    print(f"{state_total_votes("WYOMING", history)}")
    for state, votes in year_total_votes(2024, history).items():
        print(f"{state.rjust(20, ' ')}\t{votes}")
    print(f"{candidate_states_votes("JO JORGENSEN", history)}")

if __name__ == "__main__":
    main()