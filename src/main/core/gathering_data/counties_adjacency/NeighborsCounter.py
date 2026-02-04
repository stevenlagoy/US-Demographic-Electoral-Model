from typing import Any, Dict
import json
from collections import defaultdict

def read_adjacencies() -> Any:
    with open("src\\main\\core\\gathering_data\\counties_adjacency\\adjacencies.json", 'r') as adjacencies:
        return json.load(adjacencies)
    
def main() -> None:
    data = read_adjacencies()
    neighbor_counts: Dict[str, int] = {}
    state_county_counts: Dict[str, int] = defaultdict(int)
    state_neighbor_counts: Dict[str, int] = defaultdict(int)
    for fips, neighbors in data.items():
        neighbor_counts[fips] = len(neighbors)
        state_fips = fips[:2]
        state_county_counts[state_fips] += 1
        state_neighbor_counts[state_fips] += len(neighbors)
    state_neighbor_averages = {cf: nc / cc for cf, cc in state_county_counts.items() for nf, nc in state_neighbor_counts.items() if nf == cf}
    state_neighbor_averages = {k: v for k, v in state_neighbor_averages.items() if v > 1.0} # Exclude island states and territories
    print(f"National average neighbor count: {sum(neighbor_counts.values()) / len(neighbor_counts)}")
    print(f"Average neighbor counts for each state: {state_neighbor_averages}")
    print(f"Lowest state avg neighbor count: {min(state_neighbor_averages.values())}")

if __name__ == "__main__":
    main()