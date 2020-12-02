import numpy as np
import pandas as pd
import os


def write_to_file(model_data, output_directory, write_csv = False, precision = np.float32):
    """
    Write pca model to output directory
    :param output_file_name:
    :param model_data: Data to write
    :param output_directory: place to write the data
    :return:
    """
    for p_level_data in model_data:
        p_level = p_level_data['pLevel']
        # create directory for persistence level
        p_level_output_directory = os.path.join(output_directory, 'persistence-' + str(p_level))
        if not os.path.exists(p_level_output_directory):
            os.makedirs(p_level_output_directory)
        for c_id, crystal_data in enumerate(p_level_data['models']):
            # create directory for crystal
            crystal_output_directory = os.path.join(p_level_output_directory, 'crystal-' + str(c_id))
            if not os.path.exists(crystal_output_directory):
                os.makedirs(crystal_output_directory)

            # export W, w0, and z...
            for V in ['W', 'w0', 'z']:
                # ...as csvs
                if (write_csv):
                    np.savetxt(os.path.join(crystal_output_directory, V + '.csv'), crystal_data[V], delimiter=',')

                # ...and bins (with associated dims files)
                np.tofile(os.path.join(crystal_output_directory, V + '.bin'), precision(crystal_data[V]))
                dims = open(V + '.bin.dims', 'w')
                dims.write(str(crystal_data[V].shape[0]) + ' ' + str(crystal_data[V].shape[1]) + ' ')
                dims.write("float32") if precision == np.float32 else dims.write("float64")
