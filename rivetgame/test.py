from ArduinoInterface import BaseArduinoInterface
from launcher import main

if __name__ == '__main__':
    main(BaseArduinoInterface({
        "S": 3,
        "P": 20,
        "p": 30,
        "M": 15,
        "m": 4,
        "C": 12,
        "c": 2,
        "A": 10,
        "a": 30,
        "T": 3
    }))
