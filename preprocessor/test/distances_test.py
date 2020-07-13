import glob
import numpy as np
import os
import unittest
from preprocessor.test.generate_volumes import generate_volumes, generate_ground_truth_distance
from preprocessor.distances.nrrd_distances import calculate_hamming_distance_nrrd


class TestDistances(unittest.TestCase):
    def setUp(self):
        self.directory = './test_volumes/'
        number_of_samples = 100
        volume_size = 100
        generate_volumes(self.directory, number_of_samples, volume_size)
        self.gt_distance = generate_ground_truth_distance(number_of_samples, volume_size)

    def test_volume_hamming_distance(self):
        calc_distance = calculate_hamming_distance_nrrd(self.directory)
        self.assertTrue(np.allclose(calc_distance, self.gt_distance))

    def tearDown(self):
        files = glob.glob(self.directory + '*')
        for f in files:
            os.remove(f)


if __name__ == '__main__':
    unittest.main()


