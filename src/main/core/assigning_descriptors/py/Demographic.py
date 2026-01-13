'''
Steven LaGoy
Demographic.py
Created: 06 September 2025

This class was originally used to track demographic groups and blocs.
It is no longer in use because subblocs were better represented with other
    data structures. It is maintained to show creative process.

'''

# from typing import List, Dict, Any
# import json

# class DemographicGroup:
#     def __init__(self, name: str):
#         self.name = name

#     def __eq__(self, other) -> bool:
#         return self.name == other.name

# class Demographic:
#     def __init__(self, name: str, group: DemographicGroup):
#         self.name = name
#         self.group = group

#     def __eq__(self, other) -> bool:
#         return self.name == other.name and self.group == other.group
    
#     def __hash__(self):
#         return hash(self.name)

# # demographics: List[Demographic] = []
# # def create_demographics(data: Dict[str, Any]):
# #     global demographics
# #     demographics = []
# #     for group in data:
# #         for name in data[group]:
# #             demographics.append(Demographic(name, DemographicGroup(group)))

# # with open("src\\main\\resources\\data.json") as file:
# #     data = json.loads(file.read())['demographics']
# #     create_demographics(data)