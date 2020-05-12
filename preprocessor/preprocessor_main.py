import importlib.util
import json
import numpy as np
import pandas as pd
import sys
from sklearn.manifold import TSNE, Isomap, MDS
import yaml

from distances.nrrd_distances import calculate_l1_distance_nrrd, calculate_l2_distance_nrrd
from distances.png_distances import calculate_l1_distance_png, calculate_l2_distance_png

def process_data(config):
    output_directory = config['outputDirectory']
    output_config = {}
    output_config['name'] = config['datasetName']
    output_config['samples'] = {'count': config['numberSamples']}

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

    print('Calculating distances between shapes.')
    distance_type = config['distance']['type'].lower()
    if distance_type == 'precomputed':
        print('Distance is precomputed, loading from file.')
        distance = np.genfromtxt(config.distance.file, delimiter=',')
    elif distance_type == 'script':
        print('Script for distance provided, calling script.')
        spec = importlib.util.spec_from_file_location(config.distance.module, config.distance.script)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        if config.distance.arguments in config:
            distance = module.distance_calculation(config.distance.arguments)
        else:
            distance = module.distance_calculation()
    elif distance_type == 'l1':
        print('Calculating L1 distance.')
        if config['shapeFormat'] == 'nrrd':
            distance = calculate_l1_distance_nrrd(config['shapeDirectory'])
        elif config['shapeFormat'] == 'png':
            distance = calculate_l1_distance_png(config['shapeDirectory'])
        else:
            print('We\'re sorry we do not currently support the ' + config.shapeFormat +
                  '. Supported formats include nrrd and png.')
            sys.exit()
    else:
        print('Calculating L2 distance.')
        if config['shapeFormat'] == 'nrrd':
            distance = calculate_l2_distance_nrrd(config['shapeDirectory'])
        elif config['shapeFormat'] == 'png':
            distance = calculate_l2_distance_png(config['shapeDirectory'])
        else:
            print('We\'re sorry we do not currently support the ' + config.shapeFormat +
                  '. Supported formats include nrrd and png.')
            sys.exit()
    np.savetxt(output_directory + config['datasetName'] + '_distance.csv', distance, delimiter=',')
    output_config['distances'] = {'format': 'csv', 'file': config['datasetName'] + '_distance.csv', 'metric': distance_type}

    print('Calculating default 2D embeddings for entire dataset.')
    tsne = TSNE(n_components=2, metric='precomputed').fit_transform(distance)
    mds = MDS(n_components=2, dissimilarity='precomputed').fit_transform(distance)
    isomap = Isomap(n_components=2, metric='precomputed').fit_transform(distance)
    np.savetxt(output_directory + config['datasetName'] + '_tsne.csv', tsne, delimiter=',')
    np.savetxt(output_directory + config['datasetName'] + '_mds.csv', mds, delimiter=',')
    np.savetxt(output_directory + config['datasetName'] + '_isomap.csv', isomap, delimiter=',')

    embeddings = [{'name': 't-SNE', 'format': 'csv', 'file': config['datasetName'] + '_tsne.csv'},
                  {'name': 'MDS', 'format': 'csv', 'file': config['datasetName'] + '_distance.csv'},
                  {'name': 'Isomap', 'format': 'csv', 'file': config['datasetName'] + '_isomap.csv'}]

    if 'embeddings' in config:
        print('Calculating user defined 2D embeddings for entire dataset.')
        for emb in config['embeddings']:
            embedding_type = emb['type'].lower()
            if embedding_type == 'precomputed':
                print('Precomputed embedding, loading from file.')
                embedding = np.genfromtxt(emb['file'], delimiter=',')
                np.savetxt(output_directory + config['datasetName'] + '_' + emb['name'] + '.csv', embedding, delimiter=',')
                embeddings.append({'name': emb['name'],
                                   'format': 'csv',
                                   'file': output_directory + config['datasetName'] + '_' + emb['name'] + '.csv'})
            elif embedding_type == 'script':
                print('Script for distance provided, calling script.')
                spec = importlib.util.spec_from_file_location(emb['module'], emb['script'])
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)
                if 'arguments' in emb:
                    embedding = module.distance_calculation(emb['arguments'])
                else:
                    embedding = module.distance_calculation()
                embeddings.append({'name': emb.name, 'coordinates': embedding})
    output_config['embeddings'] = embeddings

    with open(config['outputDirectory'] + 'config.yaml', 'w') as file:
        yaml.dump(output_config, file, default_flow_style=False, sort_keys=False, line_break=2)



if __name__ == "__main__":
    config_path = sys.argv[1]
    with open(config_path) as json_file:
        _config = json.load(json_file)
    process_data(_config)
