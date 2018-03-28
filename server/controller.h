#pragma once

#include "Dataset.h"
#include <jsoncpp/json.h>

void configureAvailableDatasets();
Dataset* loadConcreteDataset();
Dataset* loadCrimesDataset();
Dataset* loadGaussianDataset();
Dataset* loadColoradoDataset();

void maybeLoadDataset(int datasetId);
void maybeProcessData(int k);

// Command Handlers
void fetchDatasetList(void *wsi, int messageId, const Json::Value &request);
void fetchDataset(void *wsi, int messageId, const Json::Value &request);
void fetchKNeighbors(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmalePersistence(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmalePersistenceLevel(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmaleCrystal(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request);