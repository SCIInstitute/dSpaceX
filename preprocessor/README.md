# dSpaceX Pre-processing Tool
The dSpaceX pre-processing tool is intended to make dSpaceX more user friendly. The pre-processing tool takes a JSON file as
input and outputs the files that the dSpaceX server and UI require which includes:
- The config.yaml which specifics the location and format of the processed data.
- A distance matrix between the design shape representations.
- The 2D embeddings of the design shape representations. The default embeddings are t-SNE, MDS, and Isomap.
- Thumbnails of the design shape representations.
- (TODO) The Morse-Smale decomposition.
- (TODO) The interpolation models for design prediction. 

The JSON file needs to specify the location of the original data, including, the input parameters, the quantities of interest,
and the design shape representation (usually these are images or volumes). Below we describe how to use the tool and the 
functionality a long with helpful examples.

This README assumes you have already cloned the dSpaceX repository.

## Setup
In order to run the pre-processing tool the dSpaceX conda environment needs to be created and activated. If you have
already set up the server and/or the client then the conda environment should be created, if not follow these steps:
1. In the terminal, navigate to the dSpaceX directory (this will be wherever you cloned the dSpaceX repository)
2. Run the following command:
```bash
   source ./conda_installs.sh
   ```

## Running the Pre-processing Tool
1. Activate the dSpaceX conda environment (Notice that the dSpaceX conda environment is all lower case).
```bash
   conda activate dspacex
   ```
2. In the terminal, navigate to the pre-processing tool directory.
```bash
   cd [your dSpaceX directory]/preprocessor/
   ```
3. Run the preprocess_main.py python script, this takes one argument a JSON file which specifies data locations and settings
for the tool, below we provide detailed examples of the JSON files.
```bash
   <.../dSpaceX/preprocessor>$ python preprocessor_main.py <path_to_your_config_file>/<file_name>.json 
   ```

## The JSON file
With the JSON file you can specify as little or as much as you want. There are intelligent defaults that will get your
data processed and ready to use in the tool quickly. However, we recognize the defaults may not provide all the functionality 
you might desire and have provided hooks that you can leverage to extend the functionality of the pre-processing tool.

### The Minimum
The following fields shown below in the example JSON must be specified for the pre-processing tool to work. 
We explain each field below.
```json
{
  "datasetName": "My Design Data",
  "numberSamples": 20,
  "outputDirectory": "<path>",
  "parametersFile": "<path>/<file_name>.csv",
  "qoisFile": "<path>/<file_name>.csv",
  "shapeDirectory": "<path>",
  "shapeFormat": "png or nrrd",
  "distance": {
    "type": "L1 or L2"
  },
  "thumbnails": "png or nano"
}
```

- *datasetName:* [String] The name used to identify your data set.
- *numberSamples:* [Number] The number of samples in your data set.
- *outputDirectory:* [Path] Where the processed data should be saved.
- *parametersFile:* [Path to CSV] The input parameters for your designs. Headers are expected.
- *qoisFile:* [Path to CSV] The quantities of interest for your designs. Headers are expected.
- *shapeDirectory:* [Path to Shape Representations] The shape representations for your designs.
- *shapeFormat:* [String] Currently, we support "png" or "nrrd" shape representations
- *distance:* [Object] Depending on how you want to calculate distances this object will look different. For the minimum JSON
it will only have one field 'type' explained next.
- *distance.type:* [String] For the minimum JSON you need to specify if you want the 'L1' or the 'L2' distance calculated.
- *thumbnails:* [String] Type of thumbnails to generate. You must specify either 'png' or 'nano'. Currently, we only generate
thumbnails for 3D volumes if it is the Nanoparticles data set. 

**A Note on Embeddings:**

Earlier, we mentioned that one of the jobs of the pre-processing tool is to calculate the 2D embeddings. However, you may have
noticed that we didn't specify anything in the minimum JSON for embeddings. This is because by default the tool calculates the
t-SNE, MDS, and Isomap embedding for the data set. There is the ability to have the pre-processing tool include additional user
defined embeddings; we will provide an example of this later. 
