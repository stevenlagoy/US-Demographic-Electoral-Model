from typing import List, Tuple
from enum import Enum
import sqlite3

CENSUS_FILE = "src\\main\\resources\\2010\\2010_census_race_ethnicity.csv"
DB_FILE = "src\\main\\core\\gathering_data\\census-data\\2010_census.db"

class Sex(Enum):
    MALE = 1
    FEMALE = 2

class Origin(Enum):
    NOT_HISPANIC = 1
    HISPANIC = 2

class AgeGroup(Enum):
    _0_4 = 1
    _5_9 = 2
    _10_14 = 3
    _15_19 = 4
    _20_24 = 5
    _25_29 = 6
    _30_34 = 7
    _35_39 = 8
    _40_44 = 9
    _45_49 = 10
    _50_54 = 11
    _55_59 = 12
    _60_64 = 13
    _65_69 = 14
    _70_74 = 15
    _75_79 = 16
    _80_84 = 17
    _85_up = 18

class Race(Enum):
    WHITE = 1
    BLACK = 2
    AIAN = 3 # American Indian / Alaska Native
    ASIAN = 4
    NHPI = 5 # Native Hawaiian / Pacific Islander

_IMPRACE_MAP = {
    1:  (Race.WHITE,),
    2:  (Race.BLACK,),
    3:  (Race.AIAN,),
    4:  (Race.ASIAN,),
    5:  (Race.NHPI,),
    6:  (Race.WHITE, Race.BLACK),
    7:  (Race.WHITE, Race.AIAN),
    8:  (Race.WHITE, Race.ASIAN),
    9:  (Race.WHITE, Race.NHPI),
    10: (Race.BLACK, Race.AIAN),
    11: (Race.BLACK, Race.ASIAN),
    12: (Race.BLACK, Race.NHPI),
    13: (Race.AIAN, Race.ASIAN),
    14: (Race.AIAN, Race.NHPI),
    15: (Race.ASIAN, Race.NHPI),
    16: (Race.WHITE, Race.BLACK, Race.AIAN),
    17: (Race.WHITE, Race.BLACK, Race.ASIAN),
    18: (Race.WHITE, Race.BLACK, Race.NHPI),
    19: (Race.WHITE, Race.AIAN, Race.ASIAN),
    20: (Race.WHITE, Race.AIAN, Race.NHPI),
    21: (Race.WHITE, Race.ASIAN, Race.NHPI),
    22: (Race.BLACK, Race.AIAN, Race.ASIAN),
    23: (Race.BLACK, Race.AIAN, Race.NHPI),
    24: (Race.BLACK, Race.ASIAN, Race.NHPI),
    25: (Race.AIAN, Race.ASIAN, Race.NHPI),
    26: (Race.WHITE, Race.BLACK, Race.AIAN, Race.ASIAN),
    27: (Race.WHITE, Race.BLACK, Race.AIAN, Race.NHPI),
    28: (Race.WHITE, Race.BLACK, Race.ASIAN, Race.NHPI),
    29: (Race.WHITE, Race.AIAN, Race.ASIAN, Race.NHPI),
    30: (Race.BLACK, Race.AIAN, Race.ASIAN, Race.NHPI),
    31: (Race.WHITE, Race.BLACK, Race.AIAN, Race.ASIAN, Race.NHPI),
}

class Row:
    def __init__(self,
        state_FIPS: str,
        county_FIPS: str,
        state_name: str,
        county_name: str,
        sex: Sex,
        origin: Origin,
        age_group: AgeGroup,
        race: Tuple[Race],
        population: int
    ):
        self.state_FIPS = state_FIPS
        self.county_FIPS = county_FIPS
        self.state_name = state_name
        self.county_name = county_name
        self.sex = sex
        self.origin = origin
        self.age_group = age_group
        self.race = race
        self.population = population

def read_data() -> List[Row]:
    res: List[Row] = []
    with open(CENSUS_FILE, encoding='utf-8') as data:
        content = data.readlines()
    for line in content[1:]:
        data = line.strip().split(',')
        try:
            res.append(Row(data[1], data[2], data[3], data[4], Sex(int(data[5])), Origin(int(data[6])), AgeGroup(int(data[7])), _IMPRACE_MAP[int(data[8])], int(data[9])))
        except ValueError as e:
            print(f'{e}: "{line}"')
    return res

def race_to_bitmask(races: Tuple[Race]) -> int:
    mask = 0
    for r in races:
        mask |= 1 << (r.value - 1)
    return mask

def db_init(cur: sqlite3.Cursor):
    cur.executescript('''
        CREATE TABLE State (
            FIPS CHAR(2) PRIMARY KEY,
            name TEXT NOT NULL        
        );
                
        CREATE TABLE County (
            state CHAR(2) NOT NULL,
            FIPS  CHAR(3) NOT NULL,
            name  TEXT NOT NULL,
            PRIMARY KEY (state, FIPS),
            FOREIGN KEY (state) REFERENCES State(FIPS)
        );
                
        CREATE TABLE Pop (
            state       CHAR(2) NOT NULL,
            county      CHAR(3) NOT NULL,
            is_male     BOOLEAN NOT NULL,
            is_hispanic BOOLEAN NOT NULL,
            age_group   INTEGER NOT NULL CHECK (age_group BETWEEN 1 AND 18),
            race_mask   INTEGER NOT NULL CHECK (race_mask BETWEEN 1 and 31),
            population  INTEGER NOT NULL CHECK (population >= 0),
            FOREIGN KEY (state, county) REFERENCES County(state, FIPS)
        );
    ''')

def db_push_row(cur: sqlite3.Cursor, row: Row) -> bool:
    # Insert state
    cur.execute(
        "INSERT OR IGNORE INTO State (FIPS, name) VALUES (?, ?)",
        (row.state_FIPS, row.state_name)
    )

    # Insert county
    cur.execute(
        "INSERT OR IGNORE INTO County (state, FIPS, name) VALUES (?, ?, ?)",
        (row.state_FIPS, row.county_FIPS, row.county_name)
    )

    # Insert pop row
    cur.execute(
        '''
            INSERT INTO Pop (
                state, county, is_male, is_hispanic, age_group, race_mask, population
            )
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''',
        (
            row.state_FIPS,
            row.county_FIPS,
            1 if row.sex == Sex.MALE else 0,
            1 if row.origin == Origin.HISPANIC else 0,
            row.age_group.value,
            race_to_bitmask(row.race),
            row.population
        )
    )
    return True

def main() -> None:
    # Gather data
    rows = read_data()

    # Connect and create database
    con = sqlite3.connect(DB_FILE)
    cur = con.cursor()
    db_init(cur)
    con.commit()

    for row in rows:
        db_push_row(cur, row)
    con.commit()

if __name__ == "__main__":
    main()