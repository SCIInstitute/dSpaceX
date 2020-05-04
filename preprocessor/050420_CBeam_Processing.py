from glob import glob
import numpy as np
from PIL import Image
import re
from sklearn.manifold import TSNE, Isomap, MDS

from preprocessor.distances.png_distances import calculate_l1_l2_distance_png

open_directory = '/Users/kylimckay-bishop/Temporary/CantileverMayFourth/TO_W_Support_RAW_IMG/'
save_directory = '/Users/kylimckay-bishop/Temporary/CantileverMayFourth/'
image_files = glob(open_directory + '*.png')

print('Saving images with expected numbering.')
for file in image_files:
    image_id = list(map(int, re.findall(r'\d+', file)))[0]
    img = Image.open(file)
    img.save(save_directory + 'images/' + str(image_id) + '.png')

print('Calculating Distance')
l1_distance, l2_distance = calculate_l1_l2_distance_png(open_directory)
np.savetxt(save_directory + 'distance_l1.csv', l1_distance, delimiter=',')
np.savetxt(save_directory + 'distance_l2.csv', l2_distance, delimiter=',')

print('Calculating Embeddings - t-SNE, MDS, Isomap')
tsne_l1 = TSNE(n_components=2, metric='precomputed').fit_transform(l1_distance)
tsne_l2 = TSNE(n_components=2, metric='precomputed').fit_transform(l2_distance)
np.savetxt(save_directory + 'tsne_l1.csv', tsne_l1, delimiter=',')
np.savetxt(save_directory + 'tsne_l2.csv', tsne_l2, delimiter=',')

mds_l1 = MDS(n_components=2, dissimilarity='precomputed').fit_transform(l1_distance)
mds_l2 = MDS(n_components=2, dissimilarity='precomputed').fit_transform(l2_distance)
np.savetxt(save_directory + 'mds_l1.csv', mds_l1, delimiter=',')
np.savetxt(save_directory + 'mds_l2.csv', mds_l2, delimiter=',')

isomap_l1 = Isomap(n_components=2, metric='precomputed').fit_transform(l1_distance)
isomap_l2 = Isomap(n_components=2, metric='precomputed').fit_transform(l2_distance)
np.savetxt(save_directory + 'isomap_l1.csv', isomap_l1, delimiter=',')
np.savetxt(save_directory + 'isomap_l2.csv', isomap_l2, delimiter=',')

