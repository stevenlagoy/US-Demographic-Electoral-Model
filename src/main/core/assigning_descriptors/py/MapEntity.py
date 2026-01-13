from typing import List, Dict, Any, Tuple
import json
import os

# from Demographic import Demographic, DemographicGroup
from Descriptor import Descriptor, add_demographics

class MapEntity:
    def __init__(self, name: str, population: int, demographics: Dict[str, float], descriptors: List[Descriptor] | None = None):
        self.name: str = name
        self.population: int = population
        self.demographics: Dict[str, float] = demographics
        add_demographics(*[_ for _ in demographics.keys()])
        self.descriptors: List[Descriptor] = [] if descriptors is None else list(descriptors)

        # Precompute and store keys for faster lookup
        self._demo_keys: Tuple[str, ...] = tuple(demographics.keys())

    def descriptor_demographics(self) -> Dict[str, float]:
        result: Dict[str, float] = {}
        for descriptor in self.descriptors:
            for demographic in descriptor.effects:
                if demographic not in result:
                    result[demographic] = 0
                result[demographic] += descriptor.effect_on(demographic)
        return result
    
    def _recompute_normalized(self):
        s = sum(self.demographics.values())
        if s == 0:
            self._normalized = {k : 0.0 for k in self._demo_keys}
        else:
            self._normalized = {k: v/s for k, v in self.demographics.items()}
        
    @property
    def normalized_demographics(self) -> Dict[str, float]:
        # Lazy recomputation if missing
        if not hasattr(self, "_normalized"):
            self._recompute_normalized()
        return self._normalized
    
    def update_demographics(self, new: Dict[str, float]):
        ''' Call when demographics change '''
        self.demographics = new
        self._demo_keys = tuple(new.keys())
        self._recompute_normalized()

class Nation(MapEntity):
    _instance = None

    @staticmethod
    def get_instance() -> "Nation":
        if Nation._instance is None:
            Nation._instance = Nation(0, {}, [])
        return Nation._instance

    @staticmethod
    def from_json(json: Dict[str, Any]) -> "Nation | None":
        try:
            nation = Nation.get_instance()
            nation.population = json['population']
            nation.descriptors = []
            nation.demographics = flatten_dict(json['demographics'])
        except KeyError as e:
            print(f"Encountered an error while reading Nation json: {e}")
            return None
    
    def __init__(self, population: int, demographics: Dict[str, float], descriptors: List[Descriptor] | None = None):
        super().__init__("United States", population, demographics, descriptors)

    def __str__(self) -> str:
        return f"{self.name} - population: {self.population}, descriptors: {[descriptor.name for descriptor in self.descriptors]}"

class State(MapEntity):
    def __init__(self, name: str, population: int, demographics: Dict[str, float], descriptors: List[Descriptor] | None = None):
        super().__init__(name, population, demographics, descriptors)
        self.nation = Nation.get_instance()

    def __str__(self):
        return f"{self.name} - population: {self.population}, descriptors: {[descriptor.name for descriptor in self.descriptors]}"
    
    def descriptor_demographics(self) -> Dict[str, float]:
        this_demographics = super().descriptor_demographics()
        nation_demographics = self.nation.descriptor_demographics()
        return {k : this_demographics.get(k, 0) + nation_demographics.get(k, 0) for k in set(this_demographics) | set(nation_demographics)} # Combine dictionaries and take the sum when keys overlap

class County(MapEntity):
    def __init__(self, state: State, name: str, population: int, demographics: Dict[str, float], descriptors: List[Descriptor] | None = None):
        super().__init__(name, population, demographics, descriptors)
        self.state = state

    def __str__(self):
        return f"{self.name}, {self.state.name} - population: {self.population}, descriptors: {[descriptor.name for descriptor in self.descriptors]}"
    
    def descriptor_demographics(self) -> Dict[str, float]:
        this_demographics = super().descriptor_demographics()
        state_demographics = self.state.descriptor_demographics()
        return {k : this_demographics.get(k, 0) + state_demographics.get(k, 0) for k in set(this_demographics) | set(state_demographics)} # Combine dictionaries and take the sum when keys overlap

def _data_to_json(filename) -> Dict[str, Any]:
    with open(filename) as file:
        contents = file.read()
    data = json.loads(contents)
    return data

def read_nation():
    data = _data_to_json("src\\main\\resources\\nation.json")
    Nation.from_json(data) # Saves to Nation._instance

states: List[State] = []
def read_states():
    # TODO: Determine how to store demographics: as dictionary of subdictionaries, or as flat list? 
    global states
    states = []
    for directory in os.listdir("src\\main\\resources"):
        if not '.' in directory:
            data_file = [file for file in os.listdir("src\\main\\resources\\" + directory) if '.' in file][0]
            with open("src\\main\\resources\\" + directory + "\\" + data_file) as data:
                j: Dict[str, Any] = json.loads(data.read())
                states.append(State(j["name"], j["population"], flatten_dict(j["demographics"])))

counties: List[County] = []
def read_counties():
    global counties
    counties = []
    if not len(states):
        return None 
    for state_name in os.listdir("src\\main\\resources"):
        if not '.' in state_name:
            state = [s for s in states if s.name.lower().replace(" ","_") == state_name][0]
            for county_file in [i for i in os.listdir("src\\main\\resources\\" + state_name + "\\counties") if '.' in i]:
                with open("src\\main\\resources\\" + state_name + "\\counties\\" + county_file) as data:
                    j: Dict[str, Any] = json.loads(data.read())
                    counties.append(County(state, j["name"], j["population"], flatten_dict(j["demographics"])))

def flatten_dict(d: Dict[str, Any], parent_key: str = "", sep: str = "->", result = None):
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
print("Reading Map Entities...")
read_nation()
read_states()
read_counties()
print(f"{len(states)+len(counties)+1} Map Entities read successfully")