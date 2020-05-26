import multiprocessing


def square(n):
    a, b = n
    return (a**2, b**3)


if __name__ == "__main__":
    mylist = [(1, 2), (2, 3), (3, 4), (4, 5), (5, 6)]
    p = multiprocessing.Pool()
    result = p.map(square, mylist)
    print(result)