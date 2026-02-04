from typing import List, Dict
import json
import math

def main() -> None:
    with open('src\\main\\core\\assigning_descriptors\\py2\\similarities.json', 'r', encoding='utf-8') as file:
        data: Dict[str, Dict[str, float]] = json.load(file)
    values: List[float] = []
    for sims in data.values():
        for sim in sims.values():
            values.append(sim)

    average: float = sum(values) / len(values)

    s2 = sum([(v - average)**2 for v in values]) / len(values)
    std_dev = math.sqrt(s2)

    print(f"{average=}, {s2=}, {std_dev=}")


if __name__ == "__main__":
    main()