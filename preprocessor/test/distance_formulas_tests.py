import numpy as np
import unittest

from preprocessor.distances.distance_formulas import l1_distance_formula, l2_distance_formula


class TestDistanceFormulas(unittest.TestCase):

    def test_l1(self):
        x1 = np.array([1, 2, 3, 4], dtype=np.float64)
        x2 = np.array([8, 7, 6, 5], dtype=np.float64)
        l1 = l1_distance_formula(x1, x2)
        self.assertEqual(l1, 16)

    def test_l2(self):
        x1 = np.array([1, 2, 3, 4], dtype=np.float64)
        x2 = np.array([5, 6, 7, 8], dtype=np.float64)
        l2 = l2_distance_formula(x1, x2)
        self.assertEqual(l2, 8)


if __name__ == '__main__':
    unittest.main()
