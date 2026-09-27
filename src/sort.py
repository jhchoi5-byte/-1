"""버블 정렬 — 이 저장소가 도는지 확인하는 예제이자, 새 프로젝트의 출발점."""


def bubble_sort(a):
    """a를 제자리에서 오름차순으로 정렬한다."""
    n = len(a)
    for i in range(n - 1):
        swapped = False
        # 한 번 훑을 때마다 가장 큰 값이 뒤로 밀려 자리를 잡는다.
        for j in range(n - 1 - i):
            if a[j] > a[j + 1]:
                a[j], a[j + 1] = a[j + 1], a[j]
                swapped = True
        # 한 바퀴 동안 교환이 없었다면 이미 정렬된 것이다.
        if not swapped:
            return a
    return a


def merge_sort(a):
    """a를 제자리에서 안정적으로 오름차순 정렬한다."""
    if len(a) < 2:
        return a

    temp = [None] * len(a)

    def sort_range(left, right):
        if right - left < 2:
            return
        mid = left + (right - left) // 2
        sort_range(left, mid)
        sort_range(mid, right)

        i, j, k = left, mid, left
        while i < mid and j < right:
            if a[i] <= a[j]:
                temp[k] = a[i]
                i += 1
            else:
                temp[k] = a[j]
                j += 1
            k += 1
        while i < mid:
            temp[k] = a[i]
            i += 1
            k += 1
        while j < right:
            temp[k] = a[j]
            j += 1
            k += 1
        a[left:right] = temp[left:right]

    sort_range(0, len(a))
    return a


def shell_sort(a):
    """a를 제자리에서 오름차순 정렬한다."""
    gap = len(a) // 2
    while gap > 0:
        for i in range(gap, len(a)):
            value = a[i]
            j = i
            while j >= gap and a[j - gap] > value:
                a[j] = a[j - gap]
                j -= gap
            a[j] = value
        gap //= 2
    return a


def library_sort(a):
    """Sort a in place using gaps and periodic rebalancing."""
    if len(a) < 2:
        return a

    capacity = 2 * len(a) + 1
    slots = [None] * capacity

    def insertion_target(value):
        low = 0
        high = capacity
        while low < high:
            middle = low + (high - low) // 2
            if slots[middle] is not None:
                if value < slots[middle]:
                    high = middle
                else:
                    low = middle + 1
                continue

            left = middle
            while left > 0 and slots[left - 1] is None:
                left -= 1
            right = middle
            while right < capacity and slots[right] is None:
                right += 1

            if left > 0 and right < capacity:
                if value < slots[left - 1]:
                    high = left - 1
                elif value < slots[right]:
                    return middle
                else:
                    low = right + 1
            elif left > 0:
                if value < slots[left - 1]:
                    high = left - 1
                else:
                    low = middle + 1
            elif right < capacity:
                if value < slots[right]:
                    high = middle
                else:
                    low = right + 1
            else:
                return middle

        return low

    def target_or_center(count, value):
        if count == 0:
            return capacity // 2
        return insertion_target(value)

    def nearest_empty(target):
        for distance in range(capacity):
            left = target - distance
            if 0 <= left < capacity and slots[left] is None:
                return left, distance
            right = target + distance
            if distance > 0 and 0 <= right < capacity and slots[right] is None:
                return right, distance
        return capacity, capacity

    def rebalance(count):
        values = [value for value in slots if value is not None]
        slots[:] = [None] * capacity

        denominator = count + 1
        step, remainder = divmod(capacity, denominator)
        position = 0
        error = 0
        for value in values:
            position += step
            error += remainder
            if error >= denominator:
                position += 1
                error -= denominator
            slots[position] = value

    def insert(value, count):
        target = target_or_center(count, value)
        empty, distance = nearest_empty(target)
        if empty == capacity:
            return False

        if distance > 2:
            rebalance(count)
            target = target_or_center(count, value)
            empty, distance = nearest_empty(target)
            if empty == capacity:
                return False

        if empty == target:
            slots[target] = value
        elif empty < target:
            for index in range(empty, target - 1):
                slots[index] = slots[index + 1]
            slots[target - 1] = value
        else:
            for index in range(empty, target, -1):
                slots[index] = slots[index - 1]
            slots[target] = value
        return True

    count = 0
    processed = 0
    batch = 1
    while processed < len(a):
        end = min(processed + batch, len(a))
        while processed < end:
            if not insert(a[processed], count):
                return a
            count += 1
            processed += 1
        rebalance(count)
        batch = min(batch * 2, len(a))

    a[:] = [value for value in slots if value is not None]
    return a
