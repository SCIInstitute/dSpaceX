import nrrd
import numpy as np


def generate_volumes(output_directory, number_of_samples, volume_size):
    volume = np.zeros((volume_size, volume_size, volume_size), dtype=np.single)

    volume_id = 1
    filename = output_directory + str(volume_id) + '.nrrd'
    nrrd.write(filename, volume, header={'encoding': 'raw'})
    # test_data, test_header = nrrd.read(filename)

    for i in range(volume_size):
        for j in range(volume_size):
            for k in range(volume_size):
                volume[i, j, k] = 1
                volume_id += 1
                filename = output_directory + str(volume_id) + '.nrrd'
                nrrd.write(filename, volume, header={'encoding': 'raw'})
                if volume_id >= number_of_samples:
                    return


def generate_ground_truth_distance(number_of_samples, volume_size):
    volume = volume_size**3
    distances = np.zeros((number_of_samples, number_of_samples))
    for i in range(number_of_samples):
        for j in range(i, number_of_samples):
            if i == j:
                distances[i, j] = 0
            else:
                distances[i, j] = (j - i)/volume
    distances[distances == 0] = distances.T[distances == 0]
    return distances


_output_directory = './test_volumes/'
_number_of_samples = 100
_volume_size = 100
generate_volumes(_output_directory, _number_of_samples, _volume_size)
test = generate_ground_truth_distance(_number_of_samples, _volume_size)