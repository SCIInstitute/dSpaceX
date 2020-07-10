import sys


def test_sum(inputs):
    result = 0
    if inputs is None:
        return result
    for i in inputs:
        result += i
    return result


if __name__ == "__main__":
    arguments = sys.argv[1:]
    test_sum(arguments)