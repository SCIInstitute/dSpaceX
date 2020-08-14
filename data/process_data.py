from glob import glob
from PIL import Image
import json
import numpy as np
import os
import pandas as pd
import re
import shutil
import sys
from sklearn.manifold import TSNE, Isomap, MDS
import yaml

from distances.nrrd_distances import calculate_distance_volume
from distances.png_distances import calculate_distance_png
from distances.mesh_distances import calculate_distance_mesh
from thumbnails.volume_thumbnails import generate_volume_thumbnails
from thumbnails.mesh_thumbnails import generate_mesh_thumbnails
from utils import run_external_script
from models.export_model import write_to_file
from models.png_pca import generate_image_pca_model
from models.mesh_pca import generate_mesh_pca_model
from models.volume_pca import generate_volume_pca_model


def process_data(config):
    if 'generateModel' in config and config['generateModel'] is True:
        generate_model(config)
    elif 'makePredictions' in config and config['makePredictions'] is True:
        print('Need to implement make predictions.')
        # TODO need to implement offline predictions/interpolations
    else:
        preprocess_data(config)


def generate_model(config):
    partition_directory = config['partitionDirectory']
    shape_directory = os.path.join(config['shapeDirectory'], '')
    shape_format = config['shapeFormat']

    with open(partition_directory) as partition_json:
        partition_config = json.load(partition_json)

    # Create output directory
    existing_output_directory = config['existingOutputDirectory']
    existing_output_directory = os.path.join(existing_output_directory, '')

    output_directory = os.path.join(existing_output_directory, 'ms_partitions')
    if 'outputFilename' in config:
        file_name = config['outputFilename']
    else:
        file_name = 'pca_model_' \
                    + partition_config['category'].replace(' ', '') \
                    + '_' + partition_config['field'].replace(' ', '')
    output_directory = os.path.join(output_directory, file_name)
    output_directory = os.path.join(output_directory, '')
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    # Copy partitions json to newly created directory and create a csv
    shutil.copyfile(partition_directory, output_directory + 'ms_partitions.json')

    crystal_membership = []
    for partition in partition_config['crystalPartitions']:
        crystal_membership.append(partition['crystalMembership'])
    crystal_membership = np.array(crystal_membership)
    csv_partition_directory = output_directory + 'ms_partitions.csv'
    np.savetxt(csv_partition_directory, crystal_membership, fmt='%i', delimiter=',')

    # Generate PCA model
    if shape_format == 'image':
        pca_model = generate_image_pca_model(shape_directory, partition_directory)
    elif shape_format == 'mesh':
        pca_model = generate_mesh_pca_model(shape_directory, partition_directory)
    elif shape_format == 'volume':
        pca_model = generate_volume_pca_model(shape_directory, partition_directory)
    write_to_file(pca_model, output_directory)

    # UPDATE config.yaml
    with open(existing_output_directory + 'config.yaml') as yaml_file:
        output_config = yaml.load(yaml_file, Loader=yaml.FullLoader)
    # Add model
    if 'models' in output_config and output_config['models'] is not None:
        models = output_config['models']
    else:
        models = []
    new_model = {
        'fieldname': partition_config['field'],
        'type': 'pca',
        'root': 'ms_partitions' + '/' + file_name,
        'persistences': 'persistence-?',
        'crystals': 'crystal-?',
        'padZeroes': False,
        'partitions': 'ms_partitions.csv',
        'first_partition': partition_config['minPersistence'],
        'rowmajor': True,
        'ms': {
            'knn': partition_config['neighborhoodSize'],
            'sigma': partition_config['sigma'],
            'smooth': partition_config['smoothing'],
            'depth': partition_config['depth'],
            'noise': partition_config['noise'],
            'curvepoints': partition_config['crystalCurvepoints'],
            'normalize': partition_config['normalize']
        }
    }
    # only update config.yaml if a new model was created,
    # don't want to keep adding the same model
    if new_model not in models:
        models.append(new_model)
        models_config = {'models': models}
        output_config.update(models_config)
        print('Updating config.yaml')
        with open(existing_output_directory + 'config.yaml', 'w') as yaml_file:
            noalias_dumper = yaml.dumper.SafeDumper
            noalias_dumper.ignore_aliases = lambda self, data: True
            yaml.dump(output_config, yaml_file, default_flow_style=False, sort_keys=False, line_break=2)


def preprocess_data(config):
    # OUTPUT DIRECTORY AND INITIALIZE OUTPUT CONFIG
    if 'outputDirectory' not in config:
        print('The output directory is a required field. Please, update config and run again.')
        sys.exit()

    output_directory = config['outputDirectory']
    output_directory = os.path.join(output_directory, '')
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)
    output_config = {'name': config['datasetName'], 'samples': {'count': config['numberSamples']}}

    # PARAMETERS AND QOIS
    if 'parametersFile' not in config:
        print('The parameters file is a required field. Please, update config and run again.')
        sys.exit()

    print('Reading parameters.')
    parameters_df = pd.read_csv(config['parametersFile'])
    # TODO calculate summary statistics for each parameter (mean, mode, variance, these could be displayed in client)
    parameters_df.to_csv(output_directory + config['datasetName'] + '_Parameters.csv', index=False)
    output_config['parameters'] = {'format': 'csv', 'file': config['datasetName'] + '_Parameters.csv'}

    if 'qoisFile' not in config:
        print('The QoIs file is a required field. Please, update config and run again.')
        sys.exit()

    print('Reading QoIs.')
    qois_df = pd.read_csv(config['qoisFile'])
    # TODO calculate summary statistics for each qoi (mean, mode, variance, these could be displayed in client)
    qois_df.to_csv(output_directory + config['datasetName'] + '_QoIs.csv', index=False)
    output_config['qois'] = {'format': 'csv', 'file': config['datasetName'] + '_QoIs.csv'}

    # THUMBNAILS
    thumbnail_config = generate_thumbnails(config, output_directory)
    output_config.update(thumbnail_config)

    # DISTANCES
    distance_config, distance = calculate_distance(config, output_directory)
    output_config.update(distance_config)

    # EMBEDDINGS
    embedding_config = calculate_embeddings(distance, config, output_directory)
    output_config.update(embedding_config)

    print('Generating config.yaml for dataset.')
    with open(output_directory + 'config.yaml', 'w') as file:
        yaml.dump(output_config, file, default_flow_style=False, sort_keys=False, line_break=2)
    print('Data processing complete.')
    print('Run the dSpaceX server with: --datapath ' + output_directory)
    print('Happy Exploring!')


def generate_thumbnails(input_config, output_directory):
    """
    Generates the thumbnails specified by input_config and saves the in an 'thumbnails' folder
    in the output_directory.
    Returns a dictionary that contains the thumbnail information for the config.yaml.
    The returned dictionary will need to be merged with the entire_output config.
    :param input_config: Configuration file with data specifications and settings
    :param output_directory: Directory where data should be saved
    :return: A dictionary that contains the thumbnail information for the config.yaml.
    """
    if 'shapeDirectory' not in input_config:
        print('The shape directory is a required field. Please, update config and run again.')
        sys.exit()

    shape_directory = os.path.join(input_config['shapeDirectory'], '')

    output_config = {}
    out = output_directory + 'thumbnails/'
    if not os.path.exists(out):
        os.makedirs(out)

    if 'shapeFormat' not in input_config:
        print('The shape format is a required field. Please, update config and run again.')
        sys.exit()

    shape_format = input_config['shapeFormat'].lower()
    if verify_shape_format(shape_format) is False:
        print('The shape format: ' + shape_format + ' ist not supported.'
                                                    ' Please, update config using mesh, image, or volume and run again')
        return

    # Generate thumbnails
    if shape_format == 'volume':
        print('Generating thumbnails from volume.')
        generate_volume_thumbnails(shape_directory, out)
    elif shape_format == 'mesh':
        print('Generating thumbnails from mesh.')
        generate_mesh_thumbnails(shape_directory, out)
    elif shape_format == 'image':
        print('Generating thumbnails from image.')
        image_files = glob(shape_directory + '*.png')
        for file in image_files:
            image_id = list(map(int, re.findall(r'\d+', file)))[-1]
            img = Image.open(file)
            img.save(output_directory + 'thumbnails/' + str(image_id) + '.png')

    output_config['thumbnails'] = {'format': 'png', 'files': 'thumbnails/?.png', 'offset': 1, 'padZeros': 'false'}
    return output_config


def calculate_distance(input_config, output_directory):
    # Make sure fields are in dictionary
    if 'shapeDirectory' not in input_config:
        print('The shape directory is a required field. Please, update config and run again.')
        sys.exit()

    if 'shapeFormat' not in input_config:
        print('The shape format is a required field. Please, update config and run again.')
        sys.exit()

    if 'distance' not in input_config:
        print('The distance field is a required field. Please, update config and run again.')
        sys.exit()

    if 'type' not in input_config['distance']:
        print('The distance type field is a required field. Please, update config and run again')
        sys.exit()

    # Get values
    shape_directory = os.path.join(input_config['shapeDirectory'], '')
    distance_type = input_config['distance']['type'].lower()
    shape_format = input_config['shapeFormat'].lower()
    output_config = {}

    # Verify we can perform calculation, if we can't clean up output directory and shut down processing pipeline
    if verify_distance_type(distance_type) is False:
        print('We\'re sorry we do not currently support the ' + distance_type +
              ' distance calculations. Supported distance calculations are listed in the README.')
        print('Closing data, please fix config and run again.')
        shutil.rmtree(output_directory)
        sys.exit()

    if verify_shape_format(shape_format) is False:
        print('We\'re sorry we do not currently support the ' + input_config['shapeFormat'] +
              ' shape format. Supported formats include nrrd (volume), mesh (volume) and png (image).')
        print('Closing data, please fix config and run again.')
        shutil.rmtree(output_directory)
        sys.exit()

    if distance_type == 'precomputed':
        print('Distance is precomputed, loading from file.')
        distance = np.genfromtxt(input_config['distance']['file'], delimiter=',')
    elif distance_type == 'script':
        print('Script for distance provided, calling script.')
        script_directory = input_config['distance']['script']
        module_name = input_config['distance']['moduleName']
        method_name = input_config['distance']['methodName']
        arguments = input_config['distance']['arguments'] if 'arguments' in input_config['distance'] else None
        distance = run_external_script(script_directory, module_name, method_name, arguments=arguments)
    else:
        # print('Calculating ' + distance_type + ' distance.')
        # print('\n')
        if shape_format == 'volume':
            distance = calculate_distance_volume(shape_directory, metric=distance_type)
        elif shape_format == 'image':
            distance = calculate_distance_png(shape_directory, metric=distance_type)
        elif shape_format == 'mesh':
            distance = calculate_distance_mesh(shape_directory, metric=distance_type)

    np.savetxt(output_directory + input_config['datasetName'] + '_distance.csv', distance, delimiter=',')
    output_config['distances'] = {'format': 'csv', 'file': input_config['datasetName'] + '_distance.csv',
                                  'metric': distance_type}
    return output_config, distance


def verify_distance_type(distance_type):
    supported_distances = ['precomputed', 'script', 'cityblock', 'cosine', 'euclidean', 'l1', 'l2', 'manhattan',
                           'braycurtis', 'canberra', 'chebyshev', 'correlation', 'dice', 'hamming', 'jaccard',
                           'kulsinski', 'mahalanobis', 'minkowski', 'rogerstanimoto', 'russellrao', 'seuclidean',
                           'sokalmichener', 'sokalsneath', 'sqeuclidean', 'yule']
    if distance_type in supported_distances:
        return True
    else:
        return False


def verify_shape_format(shape_format):
    supported_shapes = ['image', 'volume', 'mesh']
    if shape_format in supported_shapes:
        return True
    else:
        return False


def calculate_embeddings(distance, input_config, output_directory):
    # Calculate default embeddings first
    print('Calculating default 2D embeddings for entire dataset.')
    tsne = TSNE(n_components=2, metric='precomputed').fit_transform(distance)
    mds = MDS(n_components=2, dissimilarity='precomputed').fit_transform(distance)
    isomap = Isomap(n_components=2, metric='precomputed').fit_transform(distance)
    np.savetxt(output_directory + input_config['datasetName'] + '_tsne.csv', tsne, delimiter=',')
    np.savetxt(output_directory + input_config['datasetName'] + '_mds.csv', mds, delimiter=',')
    np.savetxt(output_directory + input_config['datasetName'] + '_isomap.csv', isomap, delimiter=',')

    embeddings = [{'name': 't-SNE', 'format': 'csv', 'file': input_config['datasetName'] + '_tsne.csv'},
                  {'name': 'MDS', 'format': 'csv', 'file': input_config['datasetName'] + '_mds.csv'},
                  {'name': 'Isomap', 'format': 'csv', 'file': input_config['datasetName'] + '_isomap.csv'}]

    # Calculate any user specified embeddings next
    if 'embeddings' in input_config:
        print('Calculating user defined 2D embeddings for entire dataset.')
        for emb in input_config['embeddings']:
            embedding_type = emb['type'].lower()
            if embedding_type == 'precomputed':
                print('Precomputed embedding, loading from file.')
                embedding = np.genfromtxt(emb['file'], delimiter=',')
                np.savetxt(output_directory + input_config['datasetName'] + '_' + emb['name'] + '.csv', embedding,
                           delimiter=',')
            elif embedding_type == 'script':
                print('Script for embedding provided, calling script.')
                script_directory = emb['script']
                module_name = emb['moduleName']
                method_name = emb['methodName']
                arguments = emb['arguments'] if 'arguments' in emb else None
                embedding = run_external_script(script_directory, module_name, method_name, arguments=arguments)
                np.savetxt(output_directory + input_config['datasetName'] + '_' + emb['name'] + '.csv', embedding,
                           delimiter=',')
            embeddings.append({'name': emb['name'],
                               'format': 'csv',
                               'file': input_config['datasetName'] + '_' + emb['name'] + '.csv'})
    return {'embeddings': embeddings}


if __name__ == "__main__":
    config_path = sys.argv[1]
    with open(config_path) as json_file:
        _config = json.load(json_file)
    process_data(_config)
