#include "SimpleHDVizDataImpl.h"
#include <stdexcept>

// TODO: Move these constants into a shared location.
const std::string k_defaultPath = "./";
const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";
const int k_defaultSamplesCount = 50;

SimpleHDVizDataImpl::SimpleHDVizDataImpl(HDProcessResult *result) : m_data(result) {
  // No work to do.
};

int SimpleHDVizDataImpl::getNumberOfSamples() {
  return k_defaultSamplesCount;
}

Precision SimpleHDVizDataImpl::getSelectedCoordinate(
    int persistenceLevel, int selectedCell, int selectedPoint, int index) {
  return m_data->R[persistenceLevel][selectedCell](index, selectedPoint);
}

Precision SimpleHDVizDataImpl::getSelectedVariance(
    int persistenceLevel, int selectedCell, int selectedPoint, int index) {
  return m_data->Rvar[persistenceLevel][selectedCell](index, selectedPoint);
}

FortranLinalg::DenseMatrix<int>& SimpleHDVizDataImpl::getEdges(int persistenceLevel) {  
  m_data->crystals[persistenceLevel];
}    

std::vector<FortranLinalg::DenseMatrix<Precision>>& SimpleHDVizDataImpl::getLayout(
    HDVizLayout layout, int persistenceLevel) {  
  switch (layout) {
    case HDVizLayout::ISOMAP : 
      return m_data->IsoLayout[persistenceLevel];
      break;
    case HDVizLayout::PCA : 
      return m_data->PCALayout[persistenceLevel];
      break;
    case HDVizLayout::PCA2 :
      return  m_data->PCA2Layout[persistenceLevel];
      break;
    default:
      throw std::invalid_argument("Unrecognized HDVizlayout specified.");
      break;
  }  
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getPersistence() {
  return m_data->scaledPersistence;
}

FortranLinalg::DenseVector<std::string>& SimpleHDVizDataImpl::getNames() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<std::string>();
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaValues() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaNormalized() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaWidths() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();
  return fake;
}

FortranLinalg::DenseMatrix<Precision>& SimpleHDVizDataImpl::getExtremaLayout() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseMatrix<Precision>();  
  return fake;
}

FortranLinalg::DenseMatrix<Precision>* SimpleHDVizDataImpl::getReconstruction() {
  // TODO: Replace with real implementation
  return nullptr;
}

FortranLinalg::DenseMatrix<Precision>* SimpleHDVizDataImpl::getVariance() {
  // TODO: Replace with real implementation
  return nullptr;
}

FortranLinalg::DenseMatrix<Precision>* SimpleHDVizDataImpl::getGradient() {
  // TODO: Replace with real implementation
  return nullptr;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRMin() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRMax() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();  
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRsMin() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();  
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRsMax() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();  
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getGradientMin() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();
  return fake;
}

FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getGradientMax() {
  // TODO: Replace with real implementation
  auto fake = FortranLinalg::DenseVector<Precision>();  
  return fake;
}

Precision SimpleHDVizDataImpl::getExtremaMinValue() {
  // TODO: Replace with real implementation
  return 0;
}
  
Precision SimpleHDVizDataImpl::getExtremaMaxValue() {
  // TODO: Replace with real implementation
  return 0;
}

Precision SimpleHDVizDataImpl::getZMin() {
  // TODO: Replace with real implementation
  return 0;  
}

Precision SimpleHDVizDataImpl::getZMax() {
  // TODO: Replace with real implementation
  return 0;  
}

FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getValueColor() {
  // TODO: Replace with real implementation
  return nullptr;
}

FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getZ() {
  // TODO: Replace with real implementation
  return nullptr;
}

FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getWidth() {
  // TODO: Replace with real implementation
  return nullptr;
}

FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getDensity() {
  // TODO: Replace with real implementation
  return nullptr;
}

ColorMapper<Precision>& SimpleHDVizDataImpl::getColorMap() {
  // TODO: Replace with real implementation
  auto fake = ColorMapper<Precision>();
  return fake;
}
  
ColorMapper<Precision>& SimpleHDVizDataImpl::getDColorMap() {
  // TODO: Replace with real implementation
  auto fake = ColorMapper<Precision>();
  return fake;
}

int SimpleHDVizDataImpl::getMinPersistenceLevel() { 
  // TODO: Replace with real implementation
  return 0;
}

int SimpleHDVizDataImpl::getMaxPersistenceLevel() { 
  // TODO: Replace with real implementation
  return 0;
}

void SimpleHDVizDataImpl::setLayout(HDVizLayout layout, int level) {
  /* No work necessary. */
};

void SimpleHDVizDataImpl::loadData(int level) {
  /* No work necessary. */
};