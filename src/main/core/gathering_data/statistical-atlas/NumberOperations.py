def formatted_number_to_int(number: str) -> int | None:
    ''' Turn a formatted number (I.E. 1.5M 10,394 54k) into an integer. '''
    try:
        return int(number.replace(",",""))
    except ValueError as e:
        pass
    try:
        value = int(number[:-2].replace(",",""))
        letter_part = number[-1]
        if letter_part == 'M':
            value *= 1000000
        elif letter_part == "k":
            value *= 1000
        return value
    except ValueError as e:
        print(str(e))
        return None


def percent_to_float(percent: str) -> float | None:
    ''' Turn a percent (I.E. 54.32% 0.0432%) into a float. '''
    try:
        return float(percent.replace("%","")) / 100
    except ValueError as e:
        print(str(e))
        return None
