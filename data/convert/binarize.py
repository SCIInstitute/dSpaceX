"""

Binarize existing csv models and distance matrices.
Binary data for these items loads 10x faster.

"""

import os
import numpy as np
import pandas as pd
import glob

def binarize_distance_matrix(infile, dtype = np.float32):
    """
    Creates .bin and .bin.dims version of the .csv file.
    Transposes in order to read as Fortran order to fill Linalg::DenseMatrix.
    :param infile: path to .csv distance matrix
    :param dtype: can specify output dtype
    """

    df = pd.read_csv(infile, header=None, dtype=dtype)
    outfile = os.path.splitext(infile)[0] + ".bin"
    print(infile + " to " + outfile + "(shape: " + str(df.shape) + ")")
    bins = open(outfile, 'wb')
    bins.write(np.asarray(df.values).T.data)   # transpose to be Fortran order
    dims = open(outfile + ".dims", 'w')
    dims.write(str(df.shape[0]) + ' ' + str(df.shape[1]) + ' ' + "float32")

def binarize_models(model_dir, dtype = np.float32):
    """
    Creates .bin and .bin.dims version of the .csv files comprising a model.
    :param model_dir: path to model dir 
                      (e.g., /dataset/processed_data/ms_partitions/fieldname_pca_model)
    :param dtype: can specify output dtype
    """

    import time
    from datetime import timedelta
    start = time.time()

    csvfiles=glob.glob(model_dir + "/**/*.csv", recursive=True)
    for index, infile in enumerate(csvfiles):
        elapsed = time.time() - start
        print('Model binarization %.2f percent complete (%s elapsed). Converting file %i of %i. ' %
              ((100 * index / len(csvfiles)), timedelta(seconds=elapsed), index, len(csvfiles)), end='\r')

        df = pd.read_csv(infile, header=None, dtype=dtype)
        outfile = os.path.splitext(infile)[0] + ".bin"
        #print(infile + " to " + outfile + " (shape: " + str(df.shape) + ")")
        np.asarray(df.values).tofile(outfile)
        dims = open(outfile + ".dims", 'w')
        dims.write(str(df.shape[0]) + ' ' + str(df.shape[1]) + ' ' + "float32")

    print('', end='\n')
    print("done!")
