import csv
import json
from typing import Any, List, Dict

class Candidate:
    def __init__(self, name: str, party: str) -> None:
        self.name = name
        self.party = party
    def __eq__(self, other: Any) -> bool:
        if isinstance(other, Candidate):
            return self.name == other.name and self.party == other.party
        return False
    def __str__(self) -> str:
        return f"Candidate: name='{self.name}', party='{self.party}';"

class Result:
    def __init__(self, total_votes: int, *votes: tuple[Candidate, int]) -> None:
        self.total_votes = total_votes
        self.votes = list(votes)
    def get_total_votes(self) -> int:
        return sum([vote[1] for vote in self.votes])
    def __str__(self) -> str:
        return f"Result: total_votes={self.total_votes}, votes={ {str(vote[0]): vote[1] for vote in self.votes} };"
    def to_json(self) -> str:
        return "{}"

class County:
    def __init__(self, state: str, county: str, fips: str) -> None:
        self.state = state
        self.county = county
        self.fips = fips
        self.results: Dict[int, Result] = {}
    def add_result(self, year: int, candidate: Candidate, votes: int) -> None:
        if year in self.results:
            self.results[year].votes.append( (candidate, votes) )
        else:
            self.results[year] = Result(0, (candidate, votes) )
    def __str__(self) -> str:
        return f"County: state='{self.state}', county='{self.county}', results={ {result: [str(r[0]) for r in self.results[result].votes] for result in self.results} };"
    def to_json(self) -> str:
        out = {
            "state": self.state,
            "county": self.county,
            "fips": self.fips,
            "results": {},
        }
        for year, votes in self.results.items():
            votes_list = []
            for candidate, vote_count in votes.votes:
                votes_list.append({
                    "party": candidate.party,
                    "candidate": candidate.name,
                    "votes": vote_count,
                })
            year_list = {
                "total_votes": votes.get_total_votes(),
                "result": votes_list,
            }
            out["results"][str(year)] = year_list

        return json.dumps(out, indent=4)


data_file = "src\\main\\resources\\countypres_2000-2024.csv"
def read_elections_file() -> List[List[str]]:
    result = []
    with open(data_file, 'r') as csvfile:
        csv_reader = csv.reader(csvfile)
        for row in csv_reader:
            result.append(row)
    return result

output_file = "src\\main\\core\\gathering_data\\electoral_history.json"
def write_results(counties: List[County]):
    with open(output_file, 'w') as out:
        out.write("{\n")
        for i, county in enumerate(counties):
            out.write(f"\t\"{county.fips}\" : {{\n")
            for line in county.to_json().split("\n")[1:-1]:
                out.write("\t" + line + "\n")
            out.write(f"\t}}{"," if i + 1 < len(counties) else ""}\n")
        out.write("}")

def main() -> None:
    counties: Dict[str, County] = {}
    rows = read_elections_file()
    for row in rows[1:]:
        try:
            year = int(row[0])
            state = row[1]
            county = row[3]
            fips = str(int(float(row[4]))).rjust(5, '0')
            candidate = row[6]
            party = row[7]
            votes = int(row[8])
            total_votes = int(row[9])

            county = counties.setdefault(f"{county}, {state}", County(state, county, fips))
            county.add_result(year, Candidate(candidate, party), votes)
        except ValueError as e:
            print(row)

    write_results([counties[county] for county in counties])

    print("Main Done")

if __name__ == "__main__":
    main()