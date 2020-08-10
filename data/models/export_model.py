import numpy as np
import os


def write_to_file(model_data, output_directory, output_file_name='pca_model'):
    """
    Write pca model to output directory
    :param model_data: Data to write
    :param output_directory: place to write the data
    :return:
    """
    # create output directory
    model_output_directory = os.path.join(output_directory, output_file_name)
    if not os.path.exists(model_output_directory):
        os.makedirs(model_output_directory)
    for p_level_data in model_data:
        p_level = p_level_data['pLevel']
        # create directory for persistence level
        p_level_output_directory = os.path.join(model_output_directory, 'persistence-' + str(p_level))
        if not os.path.exists(p_level_output_directory):
            os.makedirs(p_level_output_directory)
        # export crystal ids for that persistence level
        np.savetxt(os.path.join(p_level_output_directory, 'crystalID.csv'), p_level_data['crystalIDs'], fmt='%i',
                   delimiter=',')
        for c_id, crystal_data in enumerate(p_level_data['models']):
            # create directory for crystal
            crystal_output_directory = os.path.join(p_level_output_directory, 'crystal-' + str(c_id))
            if not os.path.exists(crystal_output_directory):
                os.makedirs(crystal_output_directory)
            # export W, w0, and z
            np.savetxt(os.path.join(crystal_output_directory, 'W.csv'), crystal_data['W'], delimiter=',')
            np.savetxt(os.path.join(crystal_output_directory, 'w0.csv'), crystal_data['w0'], delimiter=',')
            np.savetxt(os.path.join(crystal_output_directory, 'z.csv'), crystal_data['z'], delimiter=',')