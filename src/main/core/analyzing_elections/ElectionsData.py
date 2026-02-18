import json

fips = {
    "ALABAMA": "01",
    "ALASKA": "02",
    "ARIZONA": "04",
    "ARKANSAS": "05",
    "CALIFORNIA": "06",
    "COLORADO": "08",
    "CONNECTICUT": "09",
    "DELAWARE": "10",
    "DISTRICT OF COLUMBIA": "11",
    "FLORIDA": "12",
    "GEORGIA": "13",
    "HAWAII": "15",
    "IDAHO": "16",
    "ILLINOIS": "17",
    "INDIANA": "18",
    "IOWA": "19",
    "KANSAS": "20",
    "KENTUCKY": "21",
    "LOUISIANA": "22",
    "MAINE": "23",
    "MARYLAND": "24",
    "MASSACHUSETTS": "25",
    "MICHIGAN": "26",
    "MINNESOTA": "27",
    "MISSISSIPPI": "28",
    "MISSOURI": "29",
    "MONTANA": "30",
    "NEBRASKA": "31",
    "NEVADA": "32",
    "NEW HAMPSHIRE": "33",
    "NEW JERSEY": "34",
    "NEW MEXICO": "35",
    "NEW YORK": "36",
    "NORTH CAROLINA": "37",
    "NORTH DAKOTA": "38",
    "OHIO": "39",
    "OKLAHOMA": "40",
    "OREGON": "41",
    "PENNSYLVANIA": "42",
    "RHODE ISLAND": "44",
    "SOUTH CAROLINA": "45",
    "SOUTH DAKOTA": "46",
    "TENNESSEE": "47",
    "TEXAS": "48",
    "UTAH": "49",
    "VERMONT": "50",
    "VIRGINIA": "51",
    "WASHINGTON": "53",
    "WEST VIRGINIA": "54",
    "WISCONSIN": "55",
    "WYOMING": "56",
}

def main() -> None:
    res = {}
    
    with open("src\\main\\resources\\countypres_2000-2024.csv", 'r', encoding='utf-8') as data:
        content = data.readlines()

    headers = content[0].strip().split(",")
    print(headers)

    for row in content[1:]:
        fields = row.strip().split(",")
        key = ""
        try:
            key = ('000' + str(int(float(fields[headers.index("county_fips")]))))[-5:]
        except ValueError as e:
            key = fips[fields[headers.index("state")]]
        if key == "38000": key = "29095"
        name = fields[headers.index("county_name")]
        state = fields[headers.index("state")]
        if "DISTRICT" in name and state == "ALASKA": continue
        res.setdefault(key, {})
        year = fields[headers.index("year")]
        res[key].setdefault(year, [])
        candidate = fields[headers.index("candidate")]
        party = fields[headers.index("party")]
        votes = int(fields[headers.index("candidatevotes")])
        for entry in res[key][year]:
            if entry["candidate"] == candidate and entry["party"] == party:
                index = res[key][year].index(entry)
                votes += int(entry["votes"])
                res[key][year].pop(index)
        res[key][year].append({
            "candidate": candidate,
            "party": party,
            "votes": votes,
        })
    
    json.dump(res, open("src\\main\\core\\visualization\\elections.json", 'w', encoding='utf-8'), indent=4, separators=(", ", " : "))

if __name__ == "__main__":
    main()
