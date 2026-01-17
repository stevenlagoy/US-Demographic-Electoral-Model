from typing import List
from County import County

class Descriptor():
    def __init__(self, name: str):
        self.name = name
        self.members: List[County] = []