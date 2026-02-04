from typing import List, Dict, Any
import numpy as np
from math import log2

def flatten_dict(base: Dict[str, Any], sep: str = "->") -> Dict[str, Any]:
    res: Dict[str, Any] = {}
    for k, v in base.items():
        if isinstance(v, dict):
            # recurse
            for kk, vv in flatten_dict(v).items():
                res[f"{k}{sep}{kk}"] = vv
        else:
            res[k] = v
    return res

def normalize(arr: List[float] | np.ndarray, level: int = 1) -> np.ndarray:
    ''' Normalize an array of float values to sum to 1.0.
        Uses L1 normalization by default, change with the level param. '''
    arr_np = np.array(arr, dtype=np.float64)
    total = np.sum(arr_np ** level)
    if total == 0.0: return arr_np
    return arr_np / total

def kullback_leibler_divergence(p: List[float] | np.ndarray, q: List[float] | np.ndarray) -> float:
    p = np.array(p, dtype=np.float64)
    q = np.array(q, dtype=np.float64)
    mask = (p > 0) & (q > 0)
    return float(np.sum(p[mask] * np.log2(p[mask] / q[mask])))

def jensen_shannon_divergence(expected: List[float] | np.ndarray, actual: List[float] | np.ndarray) -> float:
    e = normalize(expected)
    a = normalize(actual)
    m = (e + a) / 2
    js = (kullback_leibler_divergence(e, m) + kullback_leibler_divergence(a, m)) / 2
    sim = 1 - js # Invert
    return float(np.clip(sim, 0.0, 1.0)) # Clamp to [0, 1]