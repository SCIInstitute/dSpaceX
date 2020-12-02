import process

#
# Example initial_input.json that can produce distances, embeddings, and thumbnails.
# This will be used to explore decompositions from which models can be learned.
#
# {
#   "generateThumbnails": false,
#   "generateDistancesAndEmbeddings": true,

#   "datasetName": "Cantilever Beam 2",
#   "numberSamples": 250,
#   "parametersFile": "../../../orig/CantileverBeam-2/CantileverBeam-2-Parameters.csv",
#   "qoisFile": "../../../orig/CantileverBeam-2/CantileverBeam-2-QoIs.csv",
#   "shapeDirectory": "../../../orig/CantileverBeam-2/shape_representations/",
#   "shapeFormat": "image",

#   "distances": ["pca", "cityblock", "cosine", "euclidean", "l1", "l2", "manhattan", "braycurtis", "canberra", "chebyshev", "correlation", "dice", "hamming", "jaccard", "kulsinski", "minkowski", "rogerstanimoto", "russellrao", "sokalmichener", "sokalsneath", "sqeuclidean", "yule"],

#   "outputDirectory": ".."
# }

#
# Open these in the GUI and explore. When good decompositions are discovered, save the partitionings and go to the next step to learn the models. Here is a sample config for model learning:
#
# TODO
#

if __name__ == "__main__":
    process.process_cfg(sys.argv[1])
