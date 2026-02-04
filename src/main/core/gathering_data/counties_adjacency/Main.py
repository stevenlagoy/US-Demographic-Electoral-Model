import json
import math
from typing import Any, List, Dict, Tuple, Set

def flatten_points(points: List[List[float] | Any]) -> List[Tuple[float, float]]:
    res: List[Tuple[float, float]] = []
    # Base case
    if len(points) == 2 and isinstance(points[0], float) and isinstance(points[1], float):
        res.append((points[0], points[1]))
    # Recursive case
    else:
        for l in points:
            for point in flatten_points(l):
                res.append(point)
    return res

def get_average_point(*points: Tuple[float, float]) -> Tuple[float, float]:
    x_avg = 0.
    y_avg = 0.
    for point in points:
        x_avg += point[0]
        y_avg += point[1]
    x_avg /= len(points)
    y_avg /= len(points)
    return (x_avg, y_avg)

def get_center_and_furthest_distance(*points: Tuple[float, float]) -> Tuple[Tuple[float, float], float]:
    center = get_average_point(*points)
    furthest_dist = get_furthest_distance(center, *points)
    return (center, furthest_dist)

def get_furthest_point_and_distance(center: Tuple[float, float], *points: Tuple[float, float]) -> Tuple[Tuple[float, float], float]:
    if len(points) == 0:
        return ((0., 0.), 0.)
    furthest = points[0]
    furthest_dist = 0.
    for point in points:
        dist = get_distance(center, point)
        if dist > furthest_dist:
            furthest_dist = dist
            furthest = point
    return (furthest, min(furthest_dist, 10)) # Cap at 10, as Aleutians West Census Area (02016) wraps the date line and ends up with a very large radius

def get_furthest_point(center: Tuple[float, float], *points: Tuple[float, float]) -> Tuple[float, float]:
    return get_furthest_point_and_distance(center, *points)[0]

def get_furthest_distance(center: Tuple[float, float], *points: Tuple[float, float]) -> float:
    return get_furthest_point_and_distance(center, *points)[1]

def get_distance(point1: Tuple[float, float], point2: Tuple[float, float]) -> float:
    return math.sqrt((point1[0] - point2[0]) ** 2 + (point1[1] - point2[1]) ** 2)

def main() -> None:
    # Read data from counties shapefile
    with open('src\\main\\core\\gathering_data\\counties_adjacency\\counties.json', 'r') as file:
        data = json.loads(file.read())['features']
    # Create empty adjacencies: map of county id to list of neighbor ids
    adjacencies: Dict[str, Set[str]] = {str(county['id']): set() for county in data}
    circles = {
        str(county['id']): ((0., 0.), 0.)
        for county in data
    }

    # Get the average point and radius for each county
    for county in data:
        try:
            circles[county['id']] = (get_center_and_furthest_distance(*flatten_points(county['geometry']['coordinates'])))
        except (TypeError, IndexError) as e:
            print(e)
            continue

    print(circles)
    max_dist = 0.
    max_id = None
    for id, circle in circles.items():
        if circle[1] > max_dist:
            max_dist = circle[1]
            max_id = id
    print(max_id, max_dist)

    print(max(circle[1] for id, circle in circles.items()))

    # Find each county's neighbors
    i = 0
    for this_id, circle in circles.items():
        center = circle[0]
        radius = circle[1]
        for other_id, other_circle in circles.items():
            if this_id == other_id: continue # Skip self
            if other_id in adjacencies[this_id]: continue # Skip known adjacencies
            if (this_id[:2] == '02' and other_id[:2] != '02') or (this_id[:2] == '15' and other_id[:2] != '15'): continue # Skip comparing counties in Alaska or Hawaii to counties not in the same state
            if (this_id[:2] != '02' and other_id[:2] == '02') or (this_id[:2] != '15' and other_id[:2] == '15'): continue
            other_center = other_circle[0]
            other_radius = other_circle[1]

            total_radius = radius + other_radius
            if get_distance(center, other_center) < total_radius:
                adjacencies[this_id].add(other_id)
                adjacencies[other_id].add(this_id) # Symmetric relationship
            
        print(str(round(i / len(circles) * 100)) + "%")
        i += 1

    # Write output
    with open('src\\main\\core\\gathering_data\\counties_adjacency\\adjacencies.json', 'w', encoding='utf-8') as out:
        serializable = {key: list(value) for key, value in sorted(adjacencies.items())}
        out.write(json.dumps(serializable, indent=4))

    print("Done!")

if __name__ == "__main__":
    main()