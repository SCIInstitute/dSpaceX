# dSpaceX Configuration

- [Creating datasets](#creating-datasets)  
- [Starting the server](#starting-the-server)  
- [Connecting the client](#connecting-the-client)  
- [Exploration](#exploration)  

## Creating datasets

### Preprocessing
Preprocessing script usage... (TODO)

### Decomposition
At some point the dataset can be loaded into the client and various partitionings can be explored.  
Appropriate decomposition of the data is important _before_ generating models since if the data isn't sensibyl partitioned, the "garbage in, garbage out" rule of machine learning will take precedence. 


### Configuring Datasets

The *dSpaceX* server reads datasets consisting of _images (samples)_, _design parameters (parameters)_, and _quantities of interest (QoIs)_. These must be organized into a single directory with a `config.yaml` that specifies the name of the dataset, its number of samples, and the names, locations, and formats of its images, parameters, QoIs, distance matrices, embeddings (e.g., a tsne layout), and probabilistic models. The currently supported formats are csv, json, and yaml (comma-separated values, JavaScript object notation, and "YAML ain't markup language"), and png images. Here is an example of a the yaml configuration:

```yaml
name: CantileverBeam

samples:
  count: 1000

parameters:
  format: csv
  file: CantileverBeam_design_parameters.csv

qois:
  format: csv
  file: CantileverBeam_QoIs.csv

thumbnails:
  format: png
  files: images/?.png
  offset: 1                     # base-1 image names (0th name is 1; if offset by 1000, names would start at 1000)
  padZeroes: false              # padded image names (min chars needed must represent offset + num_files)

distances:
  format: csv
  file: CantileverBeam_distance_matrix.csv
  metric: euclidean

embeddings:
  - name: tsne
    format: csv
    file: CantileverBeam_tsne_layout.csv

models:
  - fieldname: maxStress
    type: shapeodds                                            # could be shapeodds or sharedgp
    root: shapeodds_models_maxStress                           # directory of models for this field
    persistences: persistence-?                                # persistence files
    crystals: crystal-?                                        # in each persistence dir are its crystals
    padZeroes: false                                           # for both persistence and crystal dirs/files
    #format: csv   (just use extension of most files to determine format) # lots of csv files in each crystal: Z, crystalIds, W, wo
    partitions: CantileverBeam_CrystalPartitions_maxStress.csv # has 20 lines of varying length and 20 persistence levels
    embeddings: shapeodds_global_embedding.csv                 # a tsne embedding? Global for each p-lvl, and local for each crystal
```

## Starting the server
See [Running the Server](server.md#running-the-server) for instructions on starting the server.

## Connecting the client
See [Running the Client](../client/README.md#running) for details on starting the web client.

## Exploration
It's time to explore. See [Using dSpaceX](using.md) for guidance using the application.
