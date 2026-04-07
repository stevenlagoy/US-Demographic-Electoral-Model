from typing import List, Dict, Tuple, Any
from itertools import combinations
import json
import numpy as np

def flatten_dict(d: dict) -> Dict[str, float]:
    result: Dict[str, float] = {}
    for k, v in d.items():
        if isinstance(v, float):
            result[k] = float(v)
        elif isinstance(v, dict):
            nested = flatten_dict(v)
            for kk, vv in nested.items():
                result[k + "->" + kk] = float(vv)
        else:
            result[k] = float(v) if isinstance(v, (int, float)) else 0.0
    return result

def read_county_data() -> Dict[str, Any]:
    raw = json.load(open('src\\main\\core\\basic_analysis\\counties.json', 'r', encoding='utf-8'))
    normalized: Dict[str, Any] = {}
    for FIPS, county in raw.items():
        if len(FIPS) != 5: continue
        normalized[FIPS] = {
            'name': county['name'],
            'FIPS': county['FIPS'],
            'population': county['population'],
            'demographics': {}
        }
        for category in county['demographics']:
            normalized[FIPS]['demographics'][category] = {}
            for demographic in county['demographics'][category]:
                if isinstance(county['demographics'][category][demographic], dict):
                    normalized[FIPS]['demographics'][category] = flatten_dict(county['demographics'][category])
                else:
                    normalized[FIPS]['demographics'][category][demographic] = county['demographics'][category][demographic]
    return normalized

def read_electoral_data() -> Dict[str, Any]:
    return json.load(open('src\\main\\core\\basic_analysis\\elections.json', 'r', encoding='utf-8'))

def get_demographic_percentages(counties: Dict[str, Any], demographic: str) -> Dict[str, float]:
    res: Dict[str, float] = {}
    for FIPS, county_data in counties.items():
        for category in county_data['demographics'].values():
            if demographic in category:
                res[FIPS] = category[demographic]
                break
    return res

def compute_actual_distributions(
    counties: Dict[str, Any],
    elections: Dict[str, Any],
    all_parties: List[str],
    top_parties: List[str]
) -> Tuple[List[Tuple[str, str, np.ndarray]], Dict[str, List[Tuple[str, np.ndarray]]]]:
    """
    Returns:
        all_distributions: list of (FIPS, year, full party share array) for all county-years
        by_year: dict of year -> list of (FIPS, full party share array)
    """
    party_idx = {p: i for i, p in enumerate(all_parties)}
    all_distributions: List[Tuple[str, str, np.ndarray]] = []
    by_year: Dict[str, List[Tuple[str, np.ndarray]]] = {}

    for FIPS in counties:
        if FIPS not in elections or len(FIPS) != 5:
            continue
        for year, results in elections[FIPS].items():
            year_total = sum(r['votes'] for r in results)
            if year_total == 0:
                continue
            arr = np.zeros(len(all_parties))
            for r in results:
                if r['party'] in party_idx:
                    arr[party_idx[r['party']]] += r['votes'] / year_total
            all_distributions.append((FIPS, year, arr))
            by_year.setdefault(year, [])
            by_year[year].append((FIPS, arr))

    return all_distributions, by_year

def compute_national_average_mae(
    all_distributions: List[Tuple[str, str, np.ndarray]],
    nation_totals: Dict[str, float],
    all_parties: List[str],
    by_year: Dict[str, List[Tuple[str, np.ndarray]]]
) -> None:
    """
    Baseline: predict every county-year using the national average party distribution.
    Also prints per-year breakdown.
    """
    party_idx = {p: i for i, p in enumerate(all_parties)}
    national_arr = np.array([nation_totals.get(p, 0.0) for p in all_parties])

    total_mae = np.mean([
        np.mean(np.abs(national_arr - actual))
        for _, _, actual in all_distributions
    ])
    print(f'\nNational average baseline MAE (all county-years): {total_mae:.6f}  ({total_mae * 100:.4f} pp)')

    print('  Per-year breakdown:')
    for year in sorted(by_year.keys()):
        year_mae = np.mean([
            np.mean(np.abs(national_arr - actual))
            for _, actual in by_year[year]
        ])
        print(f'    {year}: {year_mae:.6f}  (n={len(by_year[year])})')

def compute_demographic_mae(
    demographic_combos: List[Tuple[str, ...]],
    demographics_electoral: Dict[Tuple[str, ...], Dict[str, float]],
    demographic_percentages: Dict[str, Dict[str, float]],
    counties: Dict[str, Any],
    elections: Dict[str, Any],
    all_distributions: List[Tuple[str, str, np.ndarray]],
    all_parties: List[str],
    by_year: Dict[str, List[Tuple[str, np.ndarray]]],
    combo_size: int
) -> None:
    """
    For each demographic combo, compute per-county-year MAE, then average across combos.
    Also prints the best-performing individual demographic or pair, and per-year breakdown
    of the average MAE.
    """
    party_idx = {p: i for i, p in enumerate(all_parties)}
    n_combos = len(demographic_combos)

    # Precompute combo percentages for all counties
    print(f'\nPrecomputing demographic percentages for {n_combos} combo(s)...')
    combo_percentages: Dict[Tuple[str, ...], Dict[str, float]] = {}
    for idx, combo in enumerate(demographic_combos):
        if idx % 500 == 0:
            print(f'  {idx}/{n_combos}')
        combo_percentages[combo] = {}
        for FIPS in counties:
            if len(FIPS) != 5 or FIPS not in elections or FIPS == '15005':
                continue
            pct = 1.0
            for demographic in combo:
                pct *= demographic_percentages[demographic].get(FIPS, 0.0)
            if pct > 0.0:
                combo_percentages[combo][FIPS] = pct

    # Compute per-combo MAE
    print(f'\nEvaluating {n_combos} combo(s)...')
    combo_maes: Dict[Tuple[str, ...], float] = {}

    # For per-year tracking across all combos
    year_mae_totals: Dict[str, float] = {}
    year_combo_counts: Dict[str, int] = {}

    for idx, combo in enumerate(demographic_combos):
        if idx % 500 == 0:
            print(f'  {idx}/{n_combos}')

        predicted_arr = np.array([demographics_electoral[combo].get(p, 0.0) for p in all_parties])
        pred_sum = predicted_arr.sum()
        if pred_sum == 0:
            continue
        predicted_arr /= pred_sum

        # Track per-year MAE for this combo
        year_mae_this_combo: Dict[str, float] = {}

        combo_mae = 0.0
        count = 0
        for FIPS, year, actual_arr in all_distributions:
            pct = combo_percentages[combo].get(FIPS, 0.0)
            if pct == 0.0:
                continue
            scaled = predicted_arr * pct
            scaled_sum = scaled.sum()
            if scaled_sum == 0:
                continue
            scaled /= scaled_sum
            mae = np.mean(np.abs(scaled - actual_arr))
            combo_mae += mae
            count += 1
            year_mae_this_combo.setdefault(year, 0.0)
            year_mae_this_combo[year] = year_mae_this_combo.get(year, 0.0) + mae

        if count > 0:
            combo_maes[combo] = combo_mae / count
            for year, year_total in year_mae_this_combo.items():
                year_count = sum(1 for f, y, _ in all_distributions
                                 if y == year and combo_percentages[combo].get(f, 0.0) > 0.0)
                if year_count > 0:
                    year_mae_totals.setdefault(year, 0.0)
                    year_mae_totals[year] += year_total / year_count
                    year_combo_counts.setdefault(year, 0)
                    year_combo_counts[year] += 1

    if not combo_maes:
        print('No valid combos found.')
        return

    avg_mae = np.mean(list(combo_maes.values()))
    label = 'single-demographic' if combo_size == 1 else f'{combo_size}-demographic-combination'
    print(f'\nAverage MAE across all {label} models: {avg_mae:.6f}  ({avg_mae * 100:.4f} pp)')

    # Best and worst performing combos
    best_combo = min(combo_maes)
    worst_combo = max(combo_maes)
    print(f'  Best combo:  {best_combo} -> MAE {combo_maes[best_combo]:.6f}  ({combo_maes[best_combo] * 100:.4f} pp)')
    print(f'  Worst combo: {worst_combo} -> MAE {combo_maes[worst_combo]:.6f}  ({combo_maes[worst_combo] * 100:.4f} pp)')

    # Per-year average MAE across all combos
    print(f'  Per-year average MAE:')
    for year in sorted(year_mae_totals.keys()):
        y_avg = year_mae_totals[year] / year_combo_counts[year] if year_combo_counts[year] > 0 else 0.0
        print(f'    {year}: {y_avg:.6f}  (across {year_combo_counts[year]} combos)')

def main() -> None:

    combo_size: int = 2  # Change to 1 for single-demographic analysis

    counties = read_county_data()
    elections = read_electoral_data()

    # Compute national vote totals (used for national average baseline and top-party selection)
    print('Computing national vote totals...')
    nation_totals: Dict[str, float] = {}
    grand_total: int = 0
    for FIPS, election in elections.items():
        if len(FIPS) != 5: continue
        for year, _returns in election.items():
            for _return in _returns:
                nation_totals.setdefault(_return['party'], 0)
                nation_totals[_return['party']] += int(_return['votes'])
                grand_total += int(_return['votes'])
    for party in nation_totals:
        nation_totals[party] /= grand_total

    sorted_parties = sorted(nation_totals.items(), key=lambda x: x[1], reverse=True)
    top_parties = [p for p, _ in sorted_parties[:2]]
    all_parties = sorted(nation_totals.keys())
    print(f'Top two parties: {top_parties}')

    # Precompute actual distributions for all county-years
    print('Computing actual distributions...')
    all_distributions, by_year = compute_actual_distributions(
        counties, elections, all_parties, top_parties
    )
    print(f'Total county-year observations: {len(all_distributions)}')

    # National average baseline
    compute_national_average_mae(all_distributions, nation_totals, all_parties, by_year)

    # Build demographic feature list
    categories = counties['01001']['demographics'].keys()
    demographics: List[str] = [
        d for category in categories
        for d in counties['01001']['demographics'][category]
    ]

    # Precompute demographic percentages
    print(f'\nPrecomputing demographic percentages for {len(demographics)} demographics...')
    demographic_percentages: Dict[str, Dict[str, float]] = {
        d: get_demographic_percentages(counties, d) for d in demographics
    }

    # Compute vote-weighted electoral signal for each combo
    total_votes_global: int = sum(
        int(_return['votes'])
        for FIPS, election in elections.items()
        for year, _returns in election.items()
        for _return in _returns
    )

    demographic_combos = list(combinations(demographics, combo_size))
    demographics_electoral: Dict[Tuple[str, ...], Dict[str, float]] = {
        combo: {} for combo in demographic_combos
    }

    print(f'\nAccumulating electoral signal for {len(demographic_combos)} combo(s)...')
    for idx, combo in enumerate(demographic_combos):
        if idx % 500 == 0:
            print(f'  {idx}/{len(demographic_combos)}')
        for FIPS in counties:
            if len(FIPS) != 5 or FIPS not in elections or FIPS == '15005':
                continue
            combo_percent = 1.0
            for demographic in combo:
                combo_percent *= demographic_percentages[demographic].get(FIPS, 0.0)
            if combo_percent == 0.0:
                continue
            for year, _returns in elections[FIPS].items():
                for _return in _returns:
                    party = _return['party']
                    votes = int(_return['votes'])
                    demographics_electoral[combo].setdefault(party, 0.0)
                    demographics_electoral[combo][party] += (votes * combo_percent) / total_votes_global

    # Evaluate demographic model MAE
    compute_demographic_mae(
        demographic_combos, demographics_electoral, demographic_percentages,
        counties, elections, all_distributions, all_parties, by_year, combo_size
    )

    print('\nDone!')

if __name__ == '__main__':
    main()