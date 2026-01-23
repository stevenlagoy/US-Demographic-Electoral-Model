from typing import List, Dict, Set
from HasDemographics import HasDemographics

class County(HasDemographics):
    def __init__(self, name: str, FIPS: str, population: int, demos: Dict[str, float]):
        super().__init__(demos)
        self.name = name
        self.FIPS = FIPS
        self.population = population
        self.neighbors: Set[County] = set()