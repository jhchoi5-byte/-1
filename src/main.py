"""실행: make run-py"""

from time import perf_counter

from sort import merge_sort, shell_sort

if __name__ == "__main__":
    size = 20000
    values = [(i * 7919 % size) - size // 2 for i in range(size)]
    merge_values = values.copy()
    shell_values = values.copy()

    start = perf_counter()
    merge_sort(merge_values)
    merge_ms = (perf_counter() - start) * 1000

    start = perf_counter()
    shell_sort(shell_values)
    shell_ms = (perf_counter() - start) * 1000

    print(f"n = {size}")
    print(f"merge sort: {merge_ms:.3f} ms")
    print(f"shell sort: {shell_ms:.3f} ms")
    expected = sorted(values)
    correct = merge_values == shell_values == expected
    print(f"results match: {'yes' if correct else 'no'}")
