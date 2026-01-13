from typing import List, Dict, Any
import json
import os

from Descriptor import Descriptor
from Descriptor import demographics as demos

class County:
    def __init__(self, name: str, state: str, population: int, demographics: Dict[str, float], descriptors: List[Descriptor]):
        self.name: str = name
        self.state: str = state
        self.population: int = population
        self.demographics: Dict[str, float] = demographics
        for d in [_ for _ in self.demographics if _ not in demos]:
            demos.append(d)
        self.descriptors: List[Descriptor] = descriptors
        self.recalculate = True
        self._desc_demos: Dict[str, float] | None = None
    
    def descriptor_demographics(self) -> Dict[str, float]:
        if self._desc_demos and not self.recalculate:
            return self._desc_demos
        self._desc_demos = {}
        for descriptor in self.descriptors:
            for demographic in descriptor.effects:
                if demographic not in self._desc_demos:
                    self._desc_demos[demographic] = 0
                self._desc_demos[demographic] += descriptor.effect_on(demographic)
        return self._desc_demos
    
    def __str__(self) -> str:
        return f"{self.name}, {self.state}: {self.population=} {self.demographics=} descriptors={[d.name for d in self.descriptors]}"
    
    def __hash__(self) -> int:
        return hash(self.name) + hash(self.state) + hash(self.population)
    
counties: List[County] = []
def read_counties():
    global counties
    counties = []
    for state in [s for s in os.listdir(f"src\\main\\resources") if '.' not in s]:
        for county in [c for c in os.listdir(f"src\\main\\resources\\{state}\\counties") if '.' in c and c.split(".")[0].isnumeric()]:
            with open(f"src\\main\\resources\\{state}\\counties\\{county}", 'r') as data:
                j: Dict[str, Any] = json.loads(data.read())
                counties.append(County(j['name'], state.replace('_',' ').title(), j['population'], flatten_dict(j['demographics']), []))

def flatten_dict(d: Dict[str, Any], parent_key: str = "", sep: str = "->", result: Dict[str, Any] | None = None) -> Dict[str, float]:
    if result is None:
        result = {}
    for k, v in d.items():
        new_key = f"{parent_key}{sep}{k}" if parent_key else k
        if isinstance(v, dict):
            flatten_dict(v, new_key, sep, result)
        elif isinstance(v, float):
            result[new_key] = v
        else:
            raise TypeError(f"Unsupported type for key {new_key}: {type(v)}")
    return result

# On Import
read_counties()
print("Counties made successfully")