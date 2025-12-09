from typing import Dict, Any

from State import State
from Paths import ROOT_URL
from MapEntity import MapEntity

class County(MapEntity):
    def __init__(self, name: str, population: int, demographics: Dict[str, Dict[str, Any]], state: State):
        super().__init__(name, population, demographics)
        self.state = state

    def __str__(self) -> str:
        return f"County: {super().__str__()}, state: {self.state.name}"

    def get_name_with_state(self) -> str:
        return f"{self.name}, {self.state.name}"
    
    def get_link(self) -> str:
        return f"{ROOT_URL}/county/{self.state.name.replace(" ","-")}/{self.name.replace(" ","-")}/Overview"
    
    def to_json(self) -> Dict[str, Any]:
        result = {}
        result['name'] = self.name
        result['population'] = self.population
        result['demographics'] = self.demographics
        return result