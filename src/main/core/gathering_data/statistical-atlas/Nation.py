from typing import Dict, Any

from MapEntity import MapEntity

class Nation(MapEntity):
    def __init__(self, name: str, population: int, demographics: Dict[str, Dict[str, Any]]):
        super().__init__(name, population, demographics)

    def __str__(self) -> str:
        return f"Nation: {super().__str__()}"
    
    def to_json(self) -> Dict[str, Any]:
        result = {}
        result['name'] = self.name
        result['population'] = self.population
        result['demographics'] = self.demographics
        return result