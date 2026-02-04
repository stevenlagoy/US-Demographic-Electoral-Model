from typing import Dict, Any

class MapEntity:
    def __init__(self, name: str, population: int, demographics: Dict[str, Dict[str, Any]]):
        self.name = name
        self.population = population
        self.demographics = demographics
    
    def __str__(self) -> str:
        return f"name: {self.name}, population: {self.population}, demographics: {self.demographics}"
    
    def to_json(self) -> Dict[str, Any]:
        result = {}
        result['name'] = self.name
        result['population'] = self.population
        result['demographics'] = self.demographics
        return result