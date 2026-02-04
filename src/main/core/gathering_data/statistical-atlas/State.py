from typing import List, Dict, Any

from Paths import ROOT_URL
from MapEntity import MapEntity

class State(MapEntity):
    def __init__(self, name: str, population: int, demographics: Dict[str, Dict[str, Any]]):
        super().__init__(name, population, demographics)

    def __str__(self) -> str:
        return f"State: {super().__str__()}"
    
    def to_json(self) -> Dict[str, Any]:
        result = {}
        result['name'] = self.name
        result['population'] = self.population
        result['demographics'] = self.demographics
        return result
    
    def get_link(self) -> str:
        return f"{ROOT_URL}/state/{self.name.replace(" ","-")}/Overview"