'''
Steven LaGoy
Created: 14 October 2025
Modified: 14 October 2025
ElectionStats.py

This file contains functions to analyze electoral history statistics.
History is held as a Dict[str, Any] and can be made with read_history(history_file: str) -> Dict[str, Any]
Functions include those to analyze at a national, state, and county level. Can sort by year, party, or candidate.
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

def candidate_total_votes(candidate: str, history: Dict[str, Any]) -> Dict[int, int]:
    ''' Get the vote total for a specific candidate by year for the supplied history. <br>
        Only years in which the candidate received votes will be included. '''
    totals: Dict[int, int] = {}
    return totals

def main() -> None:
    history_file = "src\\main\\core\\gathering_data\\electoral_history.json"
    history = read_history(history_file)
    print(f"{national_total_votes(history)}")
    print(f"{state_total_votes("WYOMING", history)}")
    for state, votes in year_total_votes(2024, history).items():
        print(f"{state.rjust(20, ' ')}\t{votes}")

if __name__ == "__main__":
    main()