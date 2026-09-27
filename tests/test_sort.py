"""유닛 테스트 — 표준 라이브러리의 unittest만 쓴다.

실행: make test-py
"""

import sys
import unittest
from pathlib import Path

# src/를 import 경로에 넣는다. 패키지로 만들지 않아도 되도록.
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from sort import bubble_sort, library_sort, merge_sort, shell_sort  # noqa: E402


class TestSort(unittest.TestCase):
    sorts = (bubble_sort, merge_sort, shell_sort, library_sort)

    def assert_all_sorts(self, values, expected):
        for sort in self.sorts:
            with self.subTest(sort=sort.__name__):
                data = values.copy()
                self.assertIs(sort(data), data)
                self.assertEqual(data, expected)

    def test_shuffled(self):
        self.assert_all_sorts(
            [6, 8, 5, 9, 10, 1, 7, 2, 4, 3],
            [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
        )

    def test_already_sorted(self):
        self.assert_all_sorts([1, 2, 3, 4, 5], [1, 2, 3, 4, 5])

    def test_reversed(self):
        self.assert_all_sorts([5, 4, 3, 2, 1], [1, 2, 3, 4, 5])

    def test_duplicates(self):
        self.assert_all_sorts([3, 1, 3, 1, 2], [1, 1, 2, 3, 3])

    def test_single(self):
        self.assert_all_sorts([42], [42])

    def test_empty(self):
        self.assert_all_sorts([], [])


if __name__ == "__main__":
    unittest.main()
