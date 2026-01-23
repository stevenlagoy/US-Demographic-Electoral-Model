from typing import Dict
from collections import defaultdict

from County import County
from Demographics import Demographics
from HasDemographics import HasDemographics

class CountySet(HasDemographics):
    def __init__(self, *counties: County):
        super().__init__({})
        self.counties = counties
        self.update_total_pop()
        self.average_counties()
        if len(self.counties) == 1:
            self.similarities = self.counties[0].similarities.copy()

    def add_counties(self, *counties: County) -> None:
        self.counties = (*counties, *self.counties)
        self.update_total_pop()

    def update_total_pop(self) -> None:
        self.total_pop = sum([county.population for county in self.counties])

    def average_counties(self) -> None:
        self.update_total_pop()
        demos: Dict[str, float] = {}
        if len(self.counties) == 1: # Special case where only one county is a member
            self.demographics = self.counties[0].demographics
            self.similarirites = self.counties[0].similarities.copy()
            return
        
        # print("Merging")
        # Weighted average demographics
        for county in self.counties:
            for k, v in county.demographics.demographics.items():
                # Scale by county's pop relative to total pop of collection
                demos.setdefault(k, 0.0)
                demos[k] += v * (county.population / self.total_pop)
        self.demographics = Demographics(demos)  

        # Merge similarities between counties
        
        merged_sims: Dict[HasDemographics, float] = {}
        members_set = set(self.counties)
        for c1 in self.counties:
            for c2, sim in c1.similarities.items():
                if c2 in members_set:
                    continue # Skip similarities to self
                merged_sims.setdefault(c2, 0.0)
                merged_sims[c2] += sim * (c1.population / self.total_pop)
        self.similarirites = merged_sims