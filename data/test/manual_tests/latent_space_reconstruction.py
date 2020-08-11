import pandas as pd
import numpy as np
from PIL import Image

partition_directory = '/Users/kylimckay-bishop/dSpaceX/CantileverBeam-1/preprocessed_data/ms_partitions/test_max_stress_pca_model/ms_partitions.csv'
qois_directory = '/Users/kylimckay-bishop/dSpaceX/CantileverBeam-1/preprocessed_data/Cantilever Beam 1_QoIs.csv'

partitions = np.genfromtxt(partition_directory, delimiter=',').reshape(1, -1)
qois_df = pd.read_csv(qois_directory)

crystal_0_directory = '/Users/kylimckay-bishop/dSpaceX/CantileverBeam-1/preprocessed_data/ms_partitions/test_max_stress_pca_model/persistence-6/crystal-0/'
# crystal_1_directory = '/Users/kylimckay-bishop/Temporary/CantileverBeam-1/pca_model_results/pca_model_qoi_max_stress/persistence-22/crystal-1/'

crystal_0_latent = np.genfromtxt(crystal_0_directory + 'z.csv', delimiter=',')
crystal_0_W = np.genfromtxt(crystal_0_directory + 'W.csv', delimiter=',')
crystal_0_w0 = np.genfromtxt(crystal_0_directory + 'w0.csv', delimiter=',')

reconstructed_samples = np.matmul(crystal_0_latent, crystal_0_W) + crystal_0_w0
recon_sample_1 = reconstructed_samples[0].reshape(40, 80)
Image.fromarray(recon_sample_1).convert('L').save('recon_sample_1.png')

# sample_1_directory = '/Users/kylimckay-bishop/Temporary/CantileverBeam-1/processed_data/images/1.png'
# sample_1_image = Image.open(sample_1_directory)
# gt_sample_1 = np.array(sample_1_image, dtype='float64')
#
# crystal_1_latent = np.genfromtxt(crystal_1_directory + 'z.csv', delimiter=',')