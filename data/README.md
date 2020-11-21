# dSpaceX Processing Tool
The **dSpaceX** data processing tool is intended to make **dSpaceX** more user
friendly. The data processing tool takes a JSON file as input and outputs the
files that the **dSpaceX** server and UI require, including:

- The config.yaml which specifics the location and format of the processed data.
- A distance matrix between the design shape representations.
- The 2D embeddings of the design shape representations.  
  The default embeddings are t-SNE, MDS, and Isomap.
- Thumbnails of the design shape representations.
- The latent space models for design prediction  
  Currently generates PCA models. 

Currently, there are two paths through the tool. Either a user can generate all
the initial data (distances, embeddings, configs, etc) or they can generate a
latent space model based on a Morse-Smale partitioning of the data generated
using the **dSpaceX** GUI.

The JSON file needs to specify the location of the original data, including the
input parameters, the quantities of interest, and the design shape
representation — usually these are images or volumes.

Below we describe how to use the tool and provide helpful examples.

This README assumes you have already cloned the **dSpaceX** repository.

## Setup
In order to run the pre-processing tool the **dSpaceX** conda environment needs to
be created and activated. If you have already set up the server and/or the
client then the conda environment should already exist, if not follow these steps:

1. In the terminal, navigate to the **dSpaceX** directory (this will be wherever you
   cloned the **dSpaceX** repository)
2. Run the following command:

```bash
   source ./conda_installs.sh
   ```

## Running the data processing Tool
1. Activate the `dspacex` conda environment (note that it is all lower case).

```bash
   conda activate dspacex
   ```
2. In the terminal, navigate to the pre-processing tool directory.

```bash
   cd [your dSpaceX directory]/data/
   ```
3. Run the process_main.py python script, this takes one argument a JSON file
   which specifies data locations and settings for the tool, below we provide
   detailed examples of the JSON files.

```bash
   <.../dSpaceX/data> $ python process_data.py <path_to_your_config_file>/<file_name>.json 
   ```

## The JSON file
With the JSON file you can specify as little or as much as you want. There are
intelligent defaults that will get your data processed and ready to use in the
**dSpaceX** server and UI quickly. However, we recognize the defaults may not
provide all the functionality you might desire and have provided extensions to
the pre-processing tool.

### The Minimum
The following fields shown below in the example JSON must be specified for the
pre-processing tool to work.  We explain each field below.

```json
{
  "datasetName": "My Design Data",
  "numberSamples": 20,
  "outputDirectory": "<path>",
  "parametersFile": "<path>/<file_name>.csv",
  "qoisFile": "<path>/<file_name>.csv",
  "shapeDirectory": "<path>",
  "shapeFormat": "image, mesh, or volume",
  "distance": {
    "type": "L1 or L2"
  }
}
```

- *datasetName:* [String] The name used to identify your data set.
- *numberSamples:* [Number] The number of samples in your data set.
- *outputDirectory:* [Path] Where the processed data should be saved.
- *parametersFile:* [Path to CSV] The input parameters for your designs. Headers are expected.
- *qoisFile:* [Path to CSV] The quantities of interest for your designs. Headers are expected.
- *shapeDirectory:* [Path to Shape Representations] The shape representations for your designs.
- *shapeFormat:* [String] Currently, we support image, volume, and mesh shape representations.
- *distance:* [Object] Depending on how you want to calculate distances this
  object will look different. For the minimum JSON it will only have one field
  'type' explained next.
- *distance.type:* [String] For the minimum JSON you need to specify if you want
  the 'L1' or the 'L2' distance calculated.


**A Note on Embeddings:**

Earlier, we mentioned that one of the jobs of the pre-processing tool is to
calculate the 2D embeddings. However, you may have noticed that we didn't
specify anything in the minimum JSON for embeddings. This is because by default
the tool calculates the t-SNE, MDS, and Isomap embedding for the data set. There
is the ability to have the pre-processing tool include additional user defined
embeddings; we will provide an example of this later.

### Using Precomputed Distances or Scripts for Distances
If the L1 or L2 distance do not fulfill your needs there are two possible
options for extending the distance functionality. You can either provides a
precomputed distance matrix in the form of a csv or provide a Python script that
the pre-processing tool will call when calculating distances.

#### Example JSON for Precomputed Distance
To use a precomputed distance matrix set the distance.type to "precomputed" and
provide the path to the csv file.

```json
{
  "datasetName": "My Design Data",
  "numberSamples": 20,
  "outputDirectory": "<path>",
  "parametersFile": "<path>/<file_name>.csv",
  "qoisFile": "<path>/<file_name>.csv",
  "shapeDirectory": "<path>",
  "shapeFormat": "image, mesh, or volume",
  "distance": {
    "type": "precomputed",
    "file": "<path>/<file_name>.csv"
  }
}
```

#### Example JSON for Python Distance Script
To use a Python script that is called when the distance matrix is calculated set
the distance.type to "script". You will also need to provide the module name and
the method name. If your method takes any arguments please provide them as a
field called "arguments". If your method does not take any arguments then
exclude the "arguments" field from the JSON. Please note, the method you create
and that the data processor calls should take a list of arguments and not
individual arguments. The pre-processing tool is simply passing the list from
the JSON file to your method, the list can be heterogeneous.

```json
{
  "datasetName": "My Design Data",
  "numberSamples": 20,
  "outputDirectory": "<path>",
  "parametersFile": "<path>/<file_name>.csv",
  "qoisFile": "<path>/<file_name>.csv",
  "shapeDirectory": "<path>",
  "shapeFormat": "image, mesh, or volume",
  "distance": {
    "type": "script",
    "script": "<path>/<file_name>.py",
    "moduleName": "<module_name> (this is likely the <file_name>",
    "methodName": "<method_name>",
    "arguments": ["<list of arguments - if your method does not take arguments exclude this field)>"]
  }
}
```

### Adding your own embeddings
By default the pre-processing tool calculates the t-SNE, MDS, and Isomap
embeddings for every data set.  If you have an additional embedding (or
embeddings) you would like to include you have two options: First, you can
provide a precomputed embedding, or second, you can provide a Python script that
the pre-processing tool will call. This functionality is the same as the
extension for distance calculations with one exception, you can provide as many
embeddings as you want. As you will see in the example JSON below the value for
the embeddings field is a list of objects, each object describes the type of
embedding you want to provide.

```json
{
  "datasetName": "My Design Data",
  "numberSamples": 20,
  "outputDirectory": "<path>",
  "parametersFile": "<path>/<file_name>.csv",
  "qoisFile": "<path>/<file_name>.csv",
  "shapeDirectory": "<path>",
  "shapeFormat": "image, mesh, or volume",
  "distance": {
    "type": "L1"
  }
  "embeddings": [
    {
      "type": "script",
      "name": "<Name of Embedding - the client will display this>",
      "script": "<path>/<file_name>.py",
      "moduleName": "<module_name> (this is likely the <file_name>)",
      "methodName": "<method_name>",
      "arguments": ["<list of arguments - if your method does not take arguments exclude this field)>"]
    },
    {
      "type": "precomputed",
      "name": "<Name of Embedding - the client will display this>",
      "file": "<path>/<file_name>.csv"
    }
  ]
}
```

## Partitioning datasets

Once the initial processing of the data is complete it is now possible to load and explore the dataset using **dSpaceX**. The first aspect of exploring datasets is to use the Morse-Smale computation on a selected field--Quantity of Interest (QoI) or Design Parameter-- in order to identify various similarities of the dataset samples.

See [Morse-Smale Decomposition](../documentation/configuration.md#morse-smale-decomposition) for details.

## Creating a Latent Space Model
To create a latent space model include the field generateModel to notify the
tool you want to generate a model.  You will also need to provide the data
partitioning, the shape representations, and the exiting output directory.  The
data processor will generate the models and update the existing config.yaml.
Below is an example of the JSON for model generation.

```json
{
  "generateModel": true,
  "partitionDirectory": "Directory for partition exported and downloaded from UI",
  "existingOutputDirectory": "Existing Output Directory",
  "outputFilename": "Name of model; if this field is not provided a name will be generated",
  "shapeDirectory": "Directory of shape representations",
  "shapeFormat": "Shape format, option include image, volume, and mesh"
}
```
