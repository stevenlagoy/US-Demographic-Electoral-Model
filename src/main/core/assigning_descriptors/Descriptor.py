from typing import Dict, List, Set

'''
Every area in the nation has at least two descriptors: one base for the nation itself, and one for the state it is in.
Descriptors are additive. Adding all the descriptors' effects for an area should give simple percentages for each demographic in that area.
The addition of all descriptors should not result in a value below zero.

nation = { "demographic" : 0.5 }
state = { "demographic" : -0.25 }
county = { "demographic" : 0.125 }
In this case, "demographic" in the county will have a percentage membership of 0.375, or 37.5%.

Any given county is likely to have many more descriptors than this.

'''

class Descriptor:

    def __init__(self, name: str, effects: Dict[str, float] | None = None, fixed: bool = False):
        self.name: str = name
        self.effects: Dict[str, float] = {} if effects is None else effects
        add_demographics(*[_ for _ in self.effects.keys()])
        for demographic in demographics:
            self.effects.setdefault(demographic, 0.0)
        self.fixed: bool = fixed
        descriptors.add(self)
    
    def effect_on(self, demographic: str) -> float:
        return self.effects[demographic]
    
    def fill_effects(self):
        for demographic in demographics:
            if demographic not in self.effects:
                self.effects[demographic] = 0.0
    
    def __hash__(self) -> int:
        return hash(self.name) + sum([hash(_) for _ in self.effects])
    
    def __eq__(self, other) -> bool:
        return type(self) == type(other) and \
               self.name == other.name and \
               self.effects == other.effects
    
    def __str__(self) -> str:
        nonzeroes = {effect: self.effects[effect] for effect in self.effects if self.effects[effect]}
        return f"{self.name}: {nonzeroes}"

descriptors: Set[Descriptor] = set()

demographics: List[str] = []

def add_demographics(*d: str) -> List[str]:
    global demographics
    for demographic in d:
        if demographic not in demographics:
            demographics.append(demographic)
    return demographics