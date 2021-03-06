"""
This is the main script for the data processing pipeline.
This is the script that is called from the command line and passed the JSON config.
"""
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
import time

from distances.nrrd_distances import calculate_distance_volume
from distances.png_distances import calculate_distance_png
from distances.mesh_distances import calculate_distance_mesh
from distances.pca_distances import calculate_pca_distance_volume
from distances.pca_distances import calculate_pca_distance_png
from distances.pca_distances import calculate_pca_distance_mesh

from thumbnails.volume_thumbnails import generate_volume_thumbnails
from thumbnails.mesh_thumbnails import generate_mesh_thumbnails
from utils import run_external_script
from models.export_model import write_to_file
from models.png_pca import generate_image_pca_model
from models.mesh_pca import generate_mesh_pca_model
from models.volume_pca import generate_volume_pca_model


def process_cfg(config_file):
    with open(config_file) as json_file:
        _config = json.load(json_file)
    os.chdir(os.path.dirname(config_file))
    process_data(_config)

def process_data(config):
    """
    Main function. Depending on config will either run initial data processing pipeline or
    generate a new model.
    :param config: dictionary containing configurations
    """
    tic = time.perf_counter()

    if 'generateModel' in config and config['generateModel'] is True:
        generate_model(config)
    elif 'makePredictions' in config and config['makePredictions'] is True:
        print('Need to implement make predictions.')
        # TODO need to implement offline predictions/interpolations
    else:
        preprocess_data(config)

    toc = time.perf_counter()
    print(f"Data processing completed in {toc - tic:0.4f} seconds")


def generate_model(config):
    """
    Generates a new latent space or prediction model for a dataset.
    :param config: dictionary containing configurations
    :return:
    """
    partition_directory = config['partitionDirectory']
    shape_directory = os.path.join(config['shapeDirectory'], '')
    shape_format = config['shapeFormat']

    with open(partition_directory) as partition_json:
        partition_config = json.load(partition_json)

    # Create output directory
    config_directory = config['outputDirectory']
    output_directory = os.path.join(config_directory, 'models')
    if 'outputFilename' in config:
        file_name = config['outputFilename']
    else:
        file_name = 'pca_model_' \
                    + partition_config['category'].replace(' ', '') \
                    + '_' + partition_config['field'].replace(' ', '')
    os.makedirs(os.path.join(output_directory, file_name), exist_ok=True)

    # Copy partitions json to newly created directory and create a csv
    shutil.copyfile(partition_directory, output_directory + 'ms_partitions.json')

    crystal_membership = []
    for partition in partition_config['crystalPartitions']:
        crystal_membership.append(partition['crystalMembership'])
    crystal_membership = np.array(crystal_membership)
    csv_partition_directory = os.path.join(output_directory, 'ms_partitions.csv')
    np.savetxt(csv_partition_directory, crystal_membership, fmt='%i', delimiter=',')

    # Generate PCA model
    print('Generating PCA model')
    if shape_format == 'image':
        pca_model = generate_image_pca_model(shape_directory, partition_directory)
    elif shape_format == 'mesh':
        pca_model = generate_mesh_pca_model(shape_directory, partition_directory)
    elif shape_format == 'volume':
        pca_model = generate_volume_pca_model(shape_directory, partition_directory)
    write_to_file(pca_model, output_directory, write_csv = False)

    # UPDATE config.yaml
    with open(os.path.join(config_directory, 'config.yaml')) as yaml_file:
        output_config = yaml.load(yaml_file, Loader=yaml.FullLoader)
    # Add model
    if 'models' in output_config and output_config['models'] is not None:
        models = output_config['models']
    else:
        models = []
    new_model = {
        'fieldname': partition_config['field'],
        'type': 'pca',
        'root': 'models' + '/' + file_name,
        'persistences': 'persistence-?',
        'crystals': 'crystal-?',
        'padZeroes': False,
        'partitions': 'ms_partitions.csv',
        'first_partition': partition_config['minPersistence'],
        'rowmajor': True,
        'ms': {
            'knn': partition_config['neighborhoodSize'],
            'datasigma': partition_config['datasigma'],
            'curvesigma': partition_config['curvesigma'],
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
        with open(os.path.join(config_directory, 'config.yaml'), 'w') as yaml_file:
            noalias_dumper = yaml.dumper.SafeDumper
            noalias_dumper.ignore_aliases = lambda self, data: True
            yaml.dump(output_config, yaml_file, default_flow_style=False, sort_keys=False, line_break=2)


def preprocess_data(config):
    """
    Initial data processing pipeline.
    Calculates distances and embeddings and generates thumbnails and config.yaml.
    :param config: dictionary containing configurations
    :return:
    """
    # OUTPUT DIRECTORY AND INITIALIZE OUTPUT CONFIG
    if 'outputDirectory' not in config:
        print('The output directory is a required field. Please, update config and run again.')
        sys.exit()

    output_directory = config['outputDirectory']
    os.makedirs(os.path.join(output_directory, "embeddings"), exist_ok=True)
    os.makedirs(os.path.join(output_directory, "distances"), exist_ok=True)
    os.makedirs(os.path.join(output_directory, "thumbnails"), exist_ok=True)

    print("current directory: " + os.getcwd())
    print("output directory: " + output_directory)
    
    output_config = {}
    cfg_filename = os.path.join(output_directory, 'config.yaml')
    if os.path.exists(cfg_filename):
        output_config = yaml.load(open(cfg_filename), Loader=yaml.FullLoader)
    output_config['name'] = config['datasetName']

    # PARAMETERS AND QOIS
    print('Reading parameters and qois...')
    if 'parametersFile' not in config or 'qoisFile' not in config:
        print('Please specify both parametersFile and qoisFile in the config and run again.')
        sys.exit()

    parameters_df = pd.read_csv(config['parametersFile'])
    parameters_df.to_csv(os.path.join(output_directory, config['datasetName'] + '_Parameters.csv'), index=False, header=True)
    output_config['parameters'] = {'file': config['datasetName'] + '_Parameters.csv'}

    qois_df = pd.read_csv(config['qoisFile'])
    qois_df.to_csv(os.path.join(output_directory, config['datasetName'] + '_QoIs.csv'), index=False, header=True)
    output_config['qois'] = {'file': config['datasetName'] + '_QoIs.csv'}

    # calculate summary statistics for each parameter and qoi (mean, mode, variance, these could be displayed in client)
    # TODO
    
    # NUM_SAMPLES
    output_config['samples'] = {'count': len(qois_df)}

    # THUMBNAILS
    if config['generateThumbnails']:
        print('Generating Thumbnails...')
        thumbnail_config = generate_thumbnails(config, os.path.join(output_directory, 'thumbnails'))
        output_config.update(thumbnail_config)

    # DISTANCES
    if config['generateDistancesAndEmbeddings']:
        print('Calculating Distances...')
        distance_config, distances = calculate_distances(config, os.path.join(output_directory, 'distances'))
        output_config.update(distance_config)

        print('Calculating Embeddings...')
        metric_names = list(map(lambda dist : dist['metric'], distance_config['distances']))
        metrics = dict(zip(metric_names, distances))
        embedding_config = calculate_embeddings(metrics, config, os.path.join(output_directory, 'embeddings'))
        output_config.update(embedding_config)

    print('Generating config.yaml for dataset...')
    with open(cfg_filename, 'w') as file:
        yaml.dump(output_config, file, default_flow_style=False, sort_keys=False, line_break=2)
    print('Data processing complete.')
    print('Run the dSpaceX server with: --datapath ' + os.path.abspath(output_directory))
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

    if 'shapeFormat' not in input_config:
        print('The shape format is a required field. Please, update config and run again.')
        sys.exit()

    shape_format = input_config['shapeFormat'].lower()
    if verify_shape_format(shape_format) is False:
        print('The shape format: ' + shape_format + ' is not supported.'
                                                    ' Please, update config using mesh, image, or volume and run again')
        return

    resolution = [300,300]
    if 'thumbnailResolution' in input_config:
        resolution = input_config['thumbnailResolution']
        
    scale = 1.0
    if 'thumbnailScale' in input_config:
        scale = input_config['thumbnailScale']

    silouettes = True
    if 'thumbnailSilouettes' in input_config:
        silouettes = input_config['thumbnailSilouettes']

    # Generate thumbnails
    if shape_format == 'volume':
        print('Generating thumbnails from volume.')
        generate_volume_thumbnails(shape_directory, output_directory, resolution=resolution, scale=scale, silouettes=silouettes)
    elif shape_format == 'mesh':
        print('Generating thumbnails from mesh.')
        generate_mesh_thumbnails(shape_directory, output_directory, resolution=resolution, scale=scale, silouettes=silouettes)
    elif shape_format == 'image':
        print('Generating thumbnails from image.')
        image_files = glob(shape_directory + '*.png')
        padding = '0' + str(len(image_files)) + 'd'
        for file in image_files:
            image_id = list(map(int, re.findall(r'\d+', file)))[-1]
            img = Image.open(file)
            img.save(os.path.join('thumbnails', format(image_id, padding) + '.png'))

    output_config['thumbnails'] = {'format': 'png', 'files': os.path.join('thumbnails', '?.png'), 'offset': 1, 'padZeros': True}
    return output_config


def calculate_distances(input_config, output_directory, precision = np.float32):
    """
    Calculates the distances between designs shapes depending on shape_format
    :param input_config: dictionary containing configurations
    :param output_directory: directory where distance matrix should be saved.
    :return:
    """
    # Make sure fields are in dictionary
    if 'shapeDirectory' not in input_config:
        print('The shape directory is a required field. Please, update config and run again.')
        sys.exit()

    if 'shapeFormat' not in input_config:
        print('The shape format is a required field. Please, update config and run again.')
        sys.exit()

    if 'distances' not in input_config:
        print('The distances array is a required field. Please, update config and run again.')
        sys.exit()

    # Get values
    shape_directory = os.path.join(input_config['shapeDirectory'], '')
    shape_format = input_config['shapeFormat'].lower()
    if verify_shape_format(shape_format) is False:
        print('We\'re sorry we do not currently support the ' + input_config['shapeFormat'] +
              ' shape format. Supported formats include nrrd (volume), mesh (volume) and png (image).')
        print('Closing data, please fix config and run again.')
        sys.exit()

    distances = []
    output_config = {}
    if 'distances_config' in output_config and output_config['distances'] is not None:
        distances_config = output_config['distances']
    else:
        distances_config = []

    for distNode in input_config['distances']:
        distance_type = None

        if type(distNode) == dict:
            if distNode['type'].startswith('precomputed'):
                print('Distance is precomputed, loading from file.')
                distance_type = distNode['type']
                distance = np.genfromtxt(distNode['file'], delimiter=',')
            elif distNode['type'] == 'script':
                print('Script for distance provided, calling script.')
                script_directory = distNode['script']
                module_name = distNode['moduleName']
                method_name = distNode['methodName']
                distance_type = module_name+'.'+methodName
                arguments = distNode['arguments'] if 'arguments' in distNode else None
                distance = run_external_script(script_directory, module_name, method_name, arguments=arguments)
        else:
            distance_type = distNode.lower()
            if verify_distance_type(distance_type):
                if distance_type == 'pca':
                    print('Calculating pca distance...\n')
                    if shape_format == 'volume':
                        distance = calculate_pca_distance_volume(shape_directory)
                    elif shape_format == 'image':
                        distance = calculate_pca_distance_png(shape_directory)
                    elif shape_format == 'mesh':
                        distance = calculate_pca_distance_mesh(shape_directory)
                else:
                    print('Calculating ' + distance_type + ' distance...\n')
                    if shape_format == 'volume':
                        distance = calculate_distance_volume(shape_directory, metric=distance_type)
                    elif shape_format == 'image':
                        distance = calculate_distance_png(shape_directory, metric=distance_type)
                    elif shape_format == 'mesh':
                        distance = calculate_distance_mesh(shape_directory, metric=distance_type)
            else:
                print('We\'re sorry we do not currently support ' + string(distNode) +
                      ' distance calculations. Supported distance calculations are listed in the README.')
                return None
                
        # save distance
        filename = os.path.join(output_directory, distance_type + '_distance')

        if 'saveDistancesDebug' in output_config:
            np.savetxt(filename + '.csv', distance, delimiter=',')

        # save as bin w/ bin.dims
        np.asarray(precision(distance)).tofile(filename + '.bin')
        dims = open(filename + '.bin.dims', 'w')
        dims.write(str(distance.shape[0]) + ' ' + str(distance.shape[1]) + ' ')
        dims.write("float32") if precision == np.float32 else dims.write("float64")

        distances.append(distance)
        new_distance = {'metric': distance_type,
                        'file': os.path.join('distances', distance_type + '_distance.bin')}

        if new_distance in distances_config:
            print('WARNING: ' + distance_type + ' already computed! Using most recent version')
        distances_config.append(new_distance)

    output_config.update({'distances': distances_config})
    return output_config, distances


def verify_distance_type(distance_type):
    """
    Verifies requested distances is supported.
    :param distance_type: Type of distance to calculate
    :return:
    """
    supported_distances = ['pca','precomputed', 'script', 'cityblock', 'cosine', 'euclidean', 'l1', 'l2', 'manhattan',
                           'braycurtis', 'canberra', 'chebyshev', 'correlation', 'dice', 'hamming', 'jaccard',
                           'kulsinski', 'mahalanobis', 'minkowski', 'rogerstanimoto', 'russellrao', 'seuclidean',
                           'sokalmichener', 'sokalsneath', 'sqeuclidean', 'yule']
    if distance_type in supported_distances:
        return True
    else:
        return False


def verify_shape_format(shape_format):
    """
    Verifies shape format is supported.
    :param shape_format: The shape format.
    :return:
    """
    supported_shapes = ['image', 'volume', 'mesh']
    if shape_format in supported_shapes:
        return True
    else:
        return False


def calculate_embeddings(metrics, input_config, output_directory):
    """
    Calculates 2D embeddings of design shapes.
    :param distance: Distance matrix
    :param input_config: dictionary containing configurations
    :param output_directory: Directory where embeddings should be saved
    """
    embeddings = []

    # Calculate default embeddings for each distance first
    for metric in metrics:
        print('Calculating default 2D embeddings for ' + metric + '...')
        distance = metrics[metric]
        tsne = TSNE(n_components=2, metric='precomputed').fit_transform(distance)
        mds = MDS(n_components=2, dissimilarity='precomputed').fit_transform(distance)
        isomap = Isomap(n_components=2, metric='precomputed').fit_transform(distance)
        np.savetxt(os.path.join(output_directory, metric + '_tsne.csv'), tsne, delimiter=',')
        np.savetxt(os.path.join(output_directory, metric + '_mds.csv'), mds, delimiter=',')
        np.savetxt(os.path.join(output_directory, metric + '_isomap.csv'), isomap, delimiter=',')

        embedding_metric = []
        embedding_metric.append({'name': 'Isomap', 'file': 'embeddings/' + metric + '_isomap.csv'})
        embedding_metric.append({'name': 't-SNE', 'file': 'embeddings/' + metric + '_tsne.csv'})
        embedding_metric.append({'name': 'MDS', 'file': 'embeddings/' + metric + '_mds.csv'})
        embeddings.append({'metric': metric, 'embeddings': embedding_metric})

    # Calculate any user specified embeddings next
    if 'embeddings' in input_config:
        print('Calculating user defined 2D embeddings for entire dataset.')
        for emb in input_config['embeddings']:
            embedding_type = emb['type'].lower()
            if embedding_type == 'precomputed':
                print('Precomputed embedding, loading from file.')
                embedding = np.genfromtxt(emb['file'], delimiter=',')
                np.savetxt(os.path.join(output_directory, input_config['datasetName'] + '_' + emb['name'] + '.csv'),
                           embedding, delimiter=',')
            elif embedding_type == 'script':
                print('Script for embedding provided, calling script.')
                script_directory = emb['script']
                module_name = emb['moduleName']
                method_name = emb['methodName']
                arguments = emb['arguments'] if 'arguments' in emb else None
                embedding = run_external_script(script_directory, module_name, method_name, arguments=arguments)
                np.savetxt(os.path.join(output_directory, input_config['datasetName'] + '_' + emb['name'] + '.csv'),
                           embedding, delimiter=',')
            embeddings.append({'name': emb['name'],
                               'file': input_config['datasetName'] + '_' + emb['name'] + '.csv'})

    return {'embeddings': embeddings}

