import sqlite3
from typing import Any, Dict
from ElectionStats import read_history

def create_schema(cursor: sqlite3.Cursor):
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS states_t (
            fips VARCHAR2(2) PRIMARY KEY,
            name TEXT NOT NULL
        );
    ''')
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS counties_t (
            fips VARCHAR2(5) PRIMARY KEY,
            name TEXT NOT NULL,
            state VARCHAR2(2),
            FOREIGN KEY (state) REFERENCES states_t (fips)
        );
    ''')
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS results_t (
            year INT NOT NULL,
            county VARCHAR2(5) NOT NULL,
            party TEXT NOT NULL,
            candidate TEXT NOT NULL,
            votes INT UNSIGNED,
            PRIMARY KEY (year, county, party)
        );
    ''')

def clear_db(cursor: sqlite3.Cursor):
    cursor.execute("DELETE FROM results_t;")
    cursor.execute("DELETE FROM counties_t;")
    cursor.execute("DELETE FROM states_t;")

def create_state(state_fips: str, state_name: str, cursor: sqlite3.Cursor):
    cursor.execute("INSERT OR REPLACE INTO states_t (fips, name) VALUES (?, ?)", (state_fips, state_name))

def create_county(county_fips: str, county_name: str, state_fips: str, cursor: sqlite3.Cursor):
    cursor.execute("INSERT OR REPLACE INTO counties_t (fips, name, state) VALUES(?, ?, ?)", (county_fips, county_name, state_fips))

def create_result(year: int, county_fips: str, party: str, candidate: str, votes: int, cursor: sqlite3.Cursor):
    cursor.execute("INSERT OR REPLACE INTO results_t (year, county, party, candidate, votes) VALUES (?, ?, ?, ?, ?)", (year, county_fips, party, candidate, votes))

def generate_from_history(history: Dict[str, Any], cursor: sqlite3.Cursor):
    for county_fips, v in history.items():
        state_name: str = v['state']
        state_fips: str = county_fips[:2]
        create_state(state_fips, state_name, cursor)
        county_name = v['county']
        create_county(county_fips, county_name, state_fips, cursor)
        for year in v['results']:
            y = int(year)
            for result in v['results'][year]['result']:
                party = result['party']
                candidate = result['candidate']
                votes = result['votes']
                create_result(y, county_fips, party, candidate, votes, cursor)

def create_and_generate(history_file, database: sqlite3.Connection):
    # Create connection and build clean schema
    cursor = database.cursor()
    create_schema(cursor)
    clear_db(cursor)
    # Write history to DB
    history = read_history(history_file)
    generate_from_history(history, cursor)
    database.commit()

def main() -> None:
    history_file = "src\\main\\core\\gathering_data\\electoral_history.json"
    database_file = "src\\main\\core\\gathering_data\\electoral_history.db"
    
    conn = sqlite3.connect(database_file)
    cursor = conn.cursor()
    create_and_generate(history_file, conn)

    # Find all states called 'New...'
    cursor.execute("SELECT * FROM states_t WHERE name LIKE 'NEW %' ORDER BY fips;")
    for row in cursor.fetchall():
        print(row)

    # Find all counties called 'Wayne...'
    cursor.execute('''
        SELECT c.name AS county_name, s.name AS state_name
        FROM counties_t c
        JOIN states_t s ON c.state = s.fips
        WHERE c.name LIKE '%WAYNE%'
        ORDER BY c.fips;
    ''')
    for row in cursor.fetchall():
        print(row)
    
    # Find all counties where the democratic candidate received no votes in 2020
    cursor.execute('''
        SELECT * FROM results_t WHERE party = 'DEMOCRAT' AND year = 2020 AND votes = 0;
    ''')
    for row in cursor.fetchall():
        print(row)

    conn.close()

if __name__ == "__main__":
    main()