from typing import Set, Dict
from collections import defaultdict
import numpy as np

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