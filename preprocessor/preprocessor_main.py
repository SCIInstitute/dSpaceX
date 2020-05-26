import json
import numpy as np
import os
import pandas as pd
import sys
from sklearn.manifold import TSNE, Isomap, MDS
import yaml

from distances.nrrd_distances import calculate_l1_distance_nrrd, calculate_l2_distance_block
from distances.png_distances import calculate_l1_distance_png, calculate_l2_distance_png
from thumbnails.nanoparticles_thumbnails import generate_thumbnails
from utils import run_external_script


def process_data(config):
    # OUTPUT DIRECTORY AND INITIALIZE OUTPUT CONFIG
    output_directory = config['outputDirectory']
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
    if 'thumbnails' in config:
        if config['thumbnails']['type'] == 'nano':
            print('Generating thumbnails for Nanoparticles.')
            out = output_directory + 'images/'
            if not os.path.exists(out):
                os.makedirs(out)
            # generate_thumbnails(output_directory + config['datasetName'] + '_Parameters.csv', out, add_slices=False)
            output_config['thumbnails'] = {'format': 'png', 'files': 'images/?.png', 'offset': 1, 'padZeros': 'false'}
        else:
            print('Sorry I only know how to make thumbnails for Nanoparticles right now. Skipping Thumbnails.')

    # DISTANCES
    distance_type = config['distance']['type'].lower()
    if distance_type == 'precomputed':
        print('Distance is precomputed, loading from file.')
        distance = np.genfromtxt(config.distance.file, delimiter=',')
    elif distance_type == 'script':
        print('Script for distance provided, calling script.')
        module_name = config['distance']['moduleName']
        method_name = config['distance']['methodName']
        script_directory = config['distance']['script']
        arguments = config['distance']['arguments'] if 'arguments' in config['distance'] else None
        distance = run_external_script(script_directory, module_name, method_name, arguments=arguments)
    elif distance_type == 'l1':
        print('Calculating L1 distance.')
        if config['shapeFormat'] == 'nrrd':
            distance = calculate_l1_distance_nrrd(config['shapeDirectory'])
        elif config['shapeFormat'] == 'png':
            distance = calculate_l1_distance_png(config['shapeDirectory'])
        else:
            print('We\'re sorry we do not currently support ' + config['shapeFormat'] +
                  '. Supported formats include nrrd and png.')
            sys.exit()
    else:
        print('Calculating L2 distance.')
        if config['shapeFormat'] == 'nrrd':
            # distance = calculate_l2_distance_block(config['shapeDirectory'])
            print('test')
        elif config['shapeFormat'] == 'png':
            distance = calculate_l2_distance_png(config['shapeDirectory'])
        else:
            print('We\'re sorry we do not currently support ' + config['shapeFormat'] +
                  '. Supported formats include nrrd and png.')
            sys.exit()
    # np.savetxt(output_directory + config['datasetName'] + '_distance.csv', distance, delimiter=',')
    output_config['distances'] = {'format': 'csv', 'file': config['datasetName'] + '_distance.csv',
                                  'metric': distance_type}

    # EMBEDDINGS
    print('Calculating default 2D embeddings for entire dataset.')
    # tsne = TSNE(n_components=2, metric='precomputed').fit_transform(distance)
    # mds = MDS(n_components=2, dissimilarity='precomputed').fit_transform(distance)
    # isomap = Isomap(n_components=2, metric='precomputed').fit_transform(distance)
    # np.savetxt(output_directory + config['datasetName'] + '_tsne.csv', tsne, delimiter=',')
    # np.savetxt(output_directory + config['datasetName'] + '_mds.csv', mds, delimiter=',')
    # np.savetxt(output_directory + config['datasetName'] + '_isomap.csv', isomap, delimiter=',')

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
                embeddings.append({'name': emb['name'],
                                   'format': 'csv',
                                   'file': config['datasetName'] + '_' + emb['name'] + '.csv'})
            elif embedding_type == 'script':
                print('Script for embedding provided, calling script.')
                module_name = emb['moduleName']
                method_name = emb['methodName']
                script_directory = emb['script']
                arguments = emb['arguments'] if 'arguments' in emb else None
                embedding = run_external_script(script_directory, module_name, method_name, arguments=arguments)
                np.savetxt(output_directory + config['datasetName'] + '_' + emb['name'] + '.csv', embedding,
                           delimiter=',')
                embeddings.append({'name': emb['name'],
                                   'format': 'csv',
                                   'file': config['datasetName'] + '_' + emb['name'] + '.csv'})
    output_config['embeddings'] = embeddings

    print('Generating config.yaml for dataset.')
    with open(config['outputDirectory'] + 'config.yaml', 'w') as file:
        yaml.dump(output_config, file, default_flow_style=False, sort_keys=False, line_break=2)
    print('Data processing complete.')
    print('Run the dSpaceX server with: --datapath ' + output_directory)
    print('Happy Exploring!')


if __name__ == "__main__":
    config_path = sys.argv[1]
    with open(config_path) as json_file:
        _config = json.load(json_file)
    process_data(_config)
