from typing import List, Dict, Set
from collections import defaultdict
import numpy as np
from Utils import jensen_shannon_divergence
from Descriptor import Descriptor

class County:
    def __init__(self, name: str, FIPS: str, population: int, demos: Dict[str, float]):
        self.name = name
        self.FIPS = FIPS
        self.population = population
        self.descriptors: List[Descriptor] = []
        self.neighbors: Set[County] = set()
        self.demographics: Demographics = Demographics(demos)

    def compare_to(self, other: 'County') -> float:
        return jensen_shannon_divergence(self.demographics.as_list(), other.demographics.as_list())
    
class Demographics:
    demographic_names: Set[str] = set()

    def __init__(self, demos: Dict[str, float]):
        self.demographics: defaultdict[str, float] = defaultdict(float)
        for k, v in demos.items():
            self.demographics[k] = v
            Demographics.demographic_names.add(k)
        self.vector = None

    def as_list(self) -> np.ndarray:
        if self.vector is not None:
            return self.vector
        names = sorted(Demographics.demographic_names)
        self.vector = np.array([self.demographics.get(name, 0.0) for name in names], dtype=np.float64)
        return self.vector