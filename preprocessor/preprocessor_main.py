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

from distances.nrrd_distances import calculate_l1_distance_nrrd, calculate_l2_distance_nrrd
from distances.png_distances import calculate_l1_distance_png, calculate_l2_distance_png
from thumbnails.nanoparticles_thumbnails import generate_nano_thumbnails
from utils import run_external_script


def process_data(config):
    # OUTPUT DIRECTORY AND INITIALIZE OUTPUT CONFIG
    output_directory = config['outputDirectory']
    output_directory = os.path.join(output_directory, '')
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)
    output_config = {'name': config['datasetName'], 'samples': {'count': config['numberSamples']}}

    # PARAMETERS AND QOIS
    print('Reading parameters.')
    parameters_df = pd.read_csv(config['parametersFile'])
    # TODO calculate summary statistics for each parameter (mean, mode, variance, these could be displayed in client)
    parameters_df.to_csv(output_directory + config['datasetName'] + '_Parameters.csv', index=False)
    output_config['parameters'] = {'format': 'csv', 'file': config['datasetName'] + '_Parameters.csv'}

    print('Reading QoIs.')
    qois_df = pd.read_csv(config['qoisFile'])
    # TODO calculate summary statistics for each qoi (mean, mode, variance, these could be displayed in client)
    qois_df.to_csv(output_directory + config['datasetName'] + '_QoIs.csv', index=False)
    output_config['qois'] = {'format': 'csv', 'file': config['datasetName'] + '_QoIs.csv'}

    # THUMBNAILS
    shape_directory = config['shapeDirectory']
    shape_directory = os.path.join(shape_directory, '')
    if 'thumbnails' in config:
        # Make sure output directory exists
        out = output_directory + 'images/'
        if not os.path.exists(out):
            os.makedirs(out)
        # Generate thumbnails
        if config['thumbnails'] == 'nano':
            print('Generating thumbnails for Nanoparticles.')
            generate_nano_thumbnails(output_directory + config['datasetName'] + '_Parameters.csv', out, add_slices=False)
            output_config['thumbnails'] = {'format': 'png', 'files': 'images/?.png', 'offset': 1, 'padZeros': 'false'}
        elif config['thumbnails'] == 'png':
            print('Generating png thumbnails.')
            image_files = glob(shape_directory + '*.png')
            for file in image_files:
                image_id = list(map(int, re.findall(r'\d+', file)))[-1]
                img = Image.open(file)
                img.save(output_directory + 'images/' + str(image_id) + '.png')
            output_config['thumbnails'] = {'format': 'png', 'files': 'images/?.png', 'offset': 1, 'padZeros': 'false'}
        else:
            print('Sorry I only know how to make thumbnails for Nanoparticles and png images. Skipping Thumbnails.')

    # DISTANCES
    distance_type = config['distance']['type'].lower()
    if distance_type == 'precomputed':
        print('Distance is precomputed, loading from file.')
        distance = np.genfromtxt(config['distance']['file'], delimiter=',')
    elif distance_type == 'script':
        print('Script for distance provided, calling script.')
        script_directory = config['distance']['script']
        module_name = config['distance']['moduleName']
        method_name = config['distance']['methodName']
        arguments = config['distance']['arguments'] if 'arguments' in config['distance'] else None
        distance = run_external_script(script_directory, module_name, method_name, arguments=arguments)
    elif distance_type == 'l1':
        print('Calculating L1 distance.')
        if config['shapeFormat'] == 'nrrd':
            distance = calculate_l1_distance_nrrd(shape_directory)
        elif config['shapeFormat'] == 'png':
            distance = calculate_l1_distance_png(shape_directory)
        else:
            print('We\'re sorry we do not currently support the ' + config['shapeFormat'] +
                  ' shape format. Supported formats include nrrd and png.')
            shutil.rmtree(output_directory)
            sys.exit()
    elif distance_type == 'l2':
        print('Calculating L2 distance.')
        if config['shapeFormat'] == 'nrrd':
            distance = calculate_l2_distance_nrrd(shape_directory)
        elif config['shapeFormat'] == 'png':
            distance = calculate_l2_distance_png(shape_directory)
        else:
            print('We\'re sorry we do not currently support ' + config['shapeFormat'] +
                  '. Supported formats include nrrd and png.')
            shutil.rmtree(output_directory)
            sys.exit()
    else:
        print('We\'re sorry we do not recognize the distance type ' + distance_type
              + '. To calculate this distance type try using the \'precomputed\' or \'script\' functionality')
        shutil.rmtree(output_directory)
        sys.exit()

    np.savetxt(output_directory + config['datasetName'] + '_distance.csv', distance, delimiter=',')
    output_config['distances'] = {'format': 'csv', 'file': config['datasetName'] + '_distance.csv',
                              'metric': distance_type}

    # EMBEDDINGS
    print('Calculating default 2D embeddings for entire dataset.')
    tsne = TSNE(n_components=2, metric='precomputed').fit_transform(distance)
    mds = MDS(n_components=2, dissimilarity='precomputed').fit_transform(distance)
    isomap = Isomap(n_components=2, metric='precomputed').fit_transform(distance)
    np.savetxt(output_directory + config['datasetName'] + '_tsne.csv', tsne, delimiter=',')
    np.savetxt(output_directory + config['datasetName'] + '_mds.csv', mds, delimiter=',')
    np.savetxt(output_directory + config['datasetName'] + '_isomap.csv', isomap, delimiter=',')

    embeddings = [{'name': 't-SNE', 'format': 'csv', 'file': config['datasetName'] + '_tsne.csv'},
                  {'name': 'MDS', 'format': 'csv', 'file': config['datasetName'] + '_mds.csv'},
                  {'name': 'Isomap', 'format': 'csv', 'file': config['datasetName'] + '_isomap.csv'}]

    if 'embeddings' in config:
        print('Calculating user defined 2D embeddings for entire dataset.')
        for emb in config['embeddings']:
            embedding_type = emb['type'].lower()
            if embedding_type == 'precomputed':
                print('Precomputed embedding, loading from file.')
                embedding = np.genfromtxt(emb['file'], delimiter=',')
                np.savetxt(output_directory + config['datasetName'] + '_' + emb['name'] + '.csv', embedding,
                           delimiter=',')
            elif embedding_type == 'script':
                print('Script for embedding provided, calling script.')
                script_directory = emb['script']
                module_name = emb['moduleName']
                method_name = emb['methodName']
                arguments = emb['arguments'] if 'arguments' in emb else None
                embedding = run_external_script(script_directory, module_name, method_name, arguments=arguments)
                np.savetxt(output_directory + config['datasetName'] + '_' + emb['name'] + '.csv', embedding,
                           delimiter=',')
            embeddings.append({'name': emb['name'],
                               'format': 'csv',
                               'file': config['datasetName'] + '_' + emb['name'] + '.csv'})
    output_config['embeddings'] = embeddings

    print('Generating config.yaml for dataset.')
    with open(output_directory + 'config.yaml', 'w') as file:
        yaml.dump(output_config, file, default_flow_style=False, sort_keys=False, line_break=2)
    print('Data processing complete.')
    print('Run the dSpaceX server with: --datapath ' + output_directory)
    print('Happy Exploring!')


if __name__ == "__main__":
    config_path = sys.argv[1]
    with open(config_path) as json_file:
        _config = json.load(json_file)
    process_data(_config)
