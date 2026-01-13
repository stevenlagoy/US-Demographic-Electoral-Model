'''
Steven LaGoy
assigning_descriptors/Main.py
Created: 09 September 2025
Modified: 15 October 2025

This is the Python version of the Simmulated Anneal model.
'''

from typing import List, Dict
import numpy as np
from random import choice, random
import os
import time

from County import County, counties
from Descriptor import Descriptor, descriptors

MAX_DESCRIPTORS = 300 # Max # of descriptors (including fixed Nation and State descriptors)
TEMPERATURE = 0.5 # How likely a change is to be accepted which decreases score
MAX_PERMUTE_CHANGE = 0.1 # Maximum amount a descriptor's effect could be changed by in one permutation

def initialize():
    ''' Assign base descriptors (nation, state) to each county, and create blank descriptors up to MAX_DESCRIPTORS. '''
    nation = Descriptor("Nation", fixed=True)
    for county in counties:
        county.descriptors.append(nation)
        county.descriptors.append(Descriptor(county.state, fixed=True))
        score_county(county)
    while len(descriptors) < MAX_DESCRIPTORS:
        Descriptor(f"Descriptor {len(descriptors)}")

def normalize(vec: List[float]) -> List[float]:
    ''' Normalize a positive vector of any length. '''
    s = sum(vec)
    if s == 0:
        return [0 for _ in vec]
    return [x/s for x in vec]

def compare_demographics(expected: Dict[str, float], actual: Dict[str, float], method: str = "l1") -> float:
    ''' Compare an expected and actual demographics using the passed method. <br>
        Methods: <ul>
            <li>'l1' -- Manhattan distance</li>
            <li>'l2' -- Eucledian distance</li>
            <li>'cosine' -- Cosine similarity</li>
            <li>'js' -- Jensen-Shannon Divergence</li>
        </ul> '''

    keys = set(expected) | set(actual)
    e = [expected.get(k, 0.0) for k in keys]
    a = [actual.get(k, 0.0) for k in keys]
    e = normalize(e)
    a = normalize(a)

    if method == "l1": # L1 Norm - Sum of absolute differences (Manhattan Distance)
        if sum(a) == 0 and sum(e) != 0:
            return 0.0
        dist = sum(abs(x - y) for x, y in zip(e, a))
        return 1 - dist / 2 # Normalize to [0, 1]
        
    if method == "l2": #  L2 Norm - Euclidian distance
        dist = np.sqrt(sum((x - y) ** 2 for x, y in zip(e, a))) # Euclidian norm: √(Σ{i=1,n}(|x_i|^2)) : x_i = expected_i - actual_i
        return 1 - dist / 2 # normalize to [0, 1]

    if method == "cosine": # Cosine similarities
        # Both vectors are normalized, so cosine similarity is just the dot product
        return np.dot(np.array(e), np.array(a))

    if method == "js": # Jensen-Shannon Divergence
        def kl(p, q): # KL-divergence function NOTE: Asymmetric- KL(P,Q) != KL(Q,P)
            return sum(
                p[i] * np.log2(p[i]/q[i])
                for i in range(len(p))
                if p[i] > 0 and q[i] > 0
            )
        # calculate J-S Divergence
        m = [(x+y)/2 for x, y in zip(e, a)]
        js = (kl(e, m) + kl(a, m)) / 2
        sim = 1 - js # Invert
        return max(0.0, min(1.0, sim)) # Clamp to [0, 1]

    raise ValueError(f"Unknown method: {method}")

class Change:
    def __init__(self, undo_fn):
        self.undo_fn = undo_fn
    def undo(self):
        self.undo_fn()

def permute_descriptors() -> Change | None:
    ''' Permute the descriptors by modifying the effects of one descriptor. <br>
        If successful, returns a Change object which can .undo() the change made. '''
    if len(descriptors) == 0:
        return None
    # Select a descriptor to modify
    descriptor = choice(list(descriptors))
    # Select an effect in the descriptor to modify
    if not descriptor.effects:
        return None
    effect = choice(list(descriptor.effects.keys()))
    # Choose how much to modify
    mod = (random() - 0.5) * 2 * MAX_PERMUTE_CHANGE # Range: [-MAX_PERMUTE_CHANCE, MAX_PERMUTE_CHANCE]
    old_value = descriptor.effects[effect]
    descriptor.effects[effect] = min(1.0, max(0.0, old_value + mod)) # Clamp to [0.0, 1.0]
    for county in counties:
        if descriptor in county.descriptors:
            county.recalculate = True
            score_county(county)
    def undo():
        descriptor.effects[effect] = old_value
        for county in counties:
            if descriptor in county.descriptors:
                score_county(county)
    return Change(undo)

def permute_counties() -> Change | None:
    ''' Permute the counties by adding or removing one modifiable descriptor from one county.
        If successful, returns a Change object which can .undo() the change made. '''
    if len(counties) == 0:
        return None
    # Select a county to modify
    county = choice(counties)
    # Select a modifiable descriptor
    descriptor = choice([_ for _ in descriptors if not _.fixed])
    if descriptor in county.descriptors:
        county.descriptors.remove(descriptor)
        county.recalculate = True
    else:
        county.descriptors.append(descriptor)
        county.recalculate = True
    def undo():
        if descriptor in county.descriptors:
            county.descriptors.remove(descriptor)
            county.recalculate = True
        else:
            county.descriptors.append(descriptor)
            county.recalculate = True
    return Change(undo)
    

top_score: int = 0
counties_scores = {county: 0.0 for county in counties}    
def score_county(county: County) -> float:
    ''' Score a county's descriptor arrangement by comparing its expected democraphics to its descriptor-derived demographics. '''
    county_score = compare_demographics(county.demographics, county.descriptor_demographics(), "l1")
    counties_scores[county] = county_score
    return county_score

def score() -> float:
    ''' Get the average score of all counties. See :func:`score_county` '''
    if not counties_scores:
        return 0.0
    avg_score = sum(counties_scores.values()) / len(counties_scores)
    return avg_score

def run():
    time_file = open("logs/times.csv","w",encoding="utf-8")
    time_file.write("time,score\n")
    start_time = time.time()
    prev_score = 0
    iteration = 0
    while True:
        change = permute_descriptors()
        new_score = score()
        if change and prev_score > new_score:
            change.undo()
            # print("Undone!")
        else:
            prev_score = new_score
            current_time = time.time() - start_time
            time_file.write(str(current_time) + "," + str(new_score) + "\n")
            print(f"{iteration} D {new_score}")

        change = permute_counties()
        new_score = score()
        if change and prev_score > new_score:
            change.undo()
        else:
            prev_score = new_score
            current_time = time.time() - start_time
            time_file.write(str(current_time) + "," + str(new_score) + "\n")
            print(f"{iteration} C {new_score}")
        iteration += 1

def write_output():
    ''' Write the current state of MapEntities and Descriptors to `logs\\log.out`.'''
    os.makedirs("logs", exist_ok=True)
    with open("logs\\log.out",'w',encoding='utf-8') as out:
        for county in counties:
            out.write(f"{county.name}, {county.state}: {[d.name for d in county.descriptors]}\n")
        for descriptor in descriptors:
            out.write(str(descriptor) + "\n")

def main() -> None:
    try:
        initialize()
        run()
    except KeyboardInterrupt:
        write_output()

if __name__ == "__main__":
    main()