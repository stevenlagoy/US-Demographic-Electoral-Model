from typing import Dict

import Demographics
from Utils import jensen_shannon_divergence

class HasDemographics:

    def __init__(self, demos: Dict[str, float]):
        self.similarities: Dict[HasDemographics, float] = {}
        self.demographics: Demographics.Demographics = Demographics.Demographics(demos)

    def compare_to(self, other: 'HasDemographics') -> float:
        from CountySet import CountySet
        if (
            isinstance(self, CountySet) and len(self.counties) == 1 and
            isinstance(other, CountySet) and len(other.counties) == 1
        ):
            # Special case where both self and other are singletons: similarity equals similarity between single members
            return self.counties[0].compare_to(other.counties[0])
        
        if other in self.similarities: return self.similarities[other]
        if isinstance(other, CountySet) and len(other.counties) == 1:
            # Special case where other is a CountySet and contains just one county: sim is same as sim to the one member
            sim = self.compare_to(other.counties[0])
        else:
            sim = jensen_shannon_divergence(self.demographics.as_list(), other.demographics.as_list())
        self.similarities[other] = sim
        other.similarities[self] = sim
        return sim