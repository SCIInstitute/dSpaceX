#include "SimpleHDVizDataImpl.h"
#include <stdexcept>

// TODO: Move these constants into a shared location.
const std::string k_defaultPath = "./";
const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";
const int k_defaultSamplesCount = 50;

/**
 * SimpleHDVizDataImpl constuctor
 */
SimpleHDVizDataImpl::SimpleHDVizDataImpl(HDProcessResult *result) : m_data(result) {

  // Create Normalized Extrema Values and Mins/Maxs
  extremaNormalized.resize(m_data->scaledPersistence.N());
  efmin.resize(m_data->scaledPersistence.N());
  efmax.resize(m_data->scaledPersistence.N());
  for (unsigned int level = 0; level < m_data->scaledPersistence.N(); level++) {
    auto ef = m_data->extremaValues[level];
    efmin[level] = FortranLinalg::Linalg<Precision>::Min(ef);
    efmax[level] = FortranLinalg::Linalg<Precision>::Max(ef);
    auto ez = FortranLinalg::DenseVector<Precision>(ef.N());
    FortranLinalg::Linalg<Precision>::Subtract(ef, efmin[level], ez);
    FortranLinalg::Linalg<Precision>::Scale(ez, 1.f/(efmax[level] - efmin[level]), ez);
    extremaNormalized[level] = ez;
  }

  // Calculate Geom min/max
  Rmin = FortranLinalg::Linalg<Precision>::RowMin(m_data->X);
  Rmax = FortranLinalg::Linalg<Precision>::RowMax(m_data->X);

  // Calculate Reconstruction min/max and Gradients min/max
  Rsmin.resize(m_data->scaledPersistence.N());
  Rsmax.resize(m_data->scaledPersistence.N());
  gRmin.resize(m_data->scaledPersistence.N());
  gRmin.resize(m_data->scaledPersistence.N());
  for (unsigned int level = 0; level < m_data->scaledPersistence.N(); level++) {
    Rsmin[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->R[level][0], 0);
    Rsmax[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->R[level][0], 0);
    gRmin[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->gradR[level][0], 0);
    gRmax[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->gradR[level][0], 0);
  }
};

/**
 *
 */
int SimpleHDVizDataImpl::getNumberOfSamples() {
  return k_defaultSamplesCount;
}

/**
 *
 */
Precision SimpleHDVizDataImpl::getSelectedCoordinate(
    int persistenceLevel, int selectedCell, int selectedPoint, int index) {
  return m_data->R[persistenceLevel][selectedCell](index, selectedPoint);
}

/**
 *
 */
Precision SimpleHDVizDataImpl::getSelectedVariance(
    int persistenceLevel, int selectedCell, int selectedPoint, int index) {
  return m_data->Rvar[persistenceLevel][selectedCell](index, selectedPoint);
}

/**
 *
 */
FortranLinalg::DenseMatrix<int>& SimpleHDVizDataImpl::getEdges(int persistenceLevel) {  
  m_data->crystals[persistenceLevel];
}    

/**
 *
 */
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

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getPersistence() {
  return m_data->scaledPersistence;
}

/**
 *
 */
FortranLinalg::DenseVector<std::string>& SimpleHDVizDataImpl::getNames() {
  return m_data->names;
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaValues(int persistenceLevel) {
  return m_data->extremaValues[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaNormalized(int persistenceLevel) {
  return extremaNormalized[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaWidths(int persistenceLevel) {
  return m_data->extremaWidths[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseMatrix<Precision>& SimpleHDVizDataImpl::getExtremaLayout(
    HDVizLayout layout, int persistenceLevel) {
  switch (layout) {
    case HDVizLayout::ISOMAP : 
      return m_data->IsoExtremaLayout[persistenceLevel];
      break;
    case HDVizLayout::PCA : 
      return m_data->PCAExtremaLayout[persistenceLevel];
      break;
    case HDVizLayout::PCA2 :
      return  m_data->PCA2ExtremaLayout[persistenceLevel];
      break;
    default:
      throw std::invalid_argument("Unrecognized HDVizlayout specified.");
      break;
  }  
}

/**
 *
 */
std::vector<FortranLinalg::DenseMatrix<Precision>>& SimpleHDVizDataImpl::getReconstruction(
    int persistenceLevel) {
  return m_data->R[persistenceLevel];
}

/**
 *
 */
std::vector<FortranLinalg::DenseMatrix<Precision>>& SimpleHDVizDataImpl::getVariance(
    int persistenceLevel) {
  return m_data->Rvar[persistenceLevel];
}

/**
 *
 */
std::vector<FortranLinalg::DenseMatrix<Precision>>& SimpleHDVizDataImpl::getGradient(
    int persistenceLevel) {
  return m_data->gradR[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRMin() {
  return Rmin;
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRMax() {
  return Rmax;
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRsMin(int persistenceLevel) {
  return Rsmin[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getRsMax(int persistenceLevel) {
  return Rsmax[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getGradientMin(int persistenceLevel) {
  return gRmin[persistenceLevel];
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getGradientMax(int persistenceLevel) {
  return gRmax[persistenceLevel];
}

/**
 *
 */
Precision SimpleHDVizDataImpl::getExtremaMinValue(int persistenceLevel) {
  return efmin[persistenceLevel];
}
  
/**
 *
 */
Precision SimpleHDVizDataImpl::getExtremaMaxValue(int persistenceLevel) {
  return efmax[persistenceLevel];
}

/**
 *
 */
Precision SimpleHDVizDataImpl::getZMin() {
  // TODO: Replace with real implementation
  return 0;  
}

/**
 *
 */
Precision SimpleHDVizDataImpl::getZMax() {
  // TODO: Replace with real implementation
  return 0;  
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getValueColor() {
  // TODO: Replace with real implementation
  return nullptr;
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getZ() {
  // TODO: Replace with real implementation
  return nullptr;
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getWidth() {
  // TODO: Replace with real implementation
  return nullptr;
}

/**
 *
 */
FortranLinalg::DenseVector<Precision>* SimpleHDVizDataImpl::getDensity() {
  // TODO: Replace with real implementation
  return nullptr;
}

/**
 *
 */
ColorMapper<Precision>& SimpleHDVizDataImpl::getColorMap() {
  // TODO: Replace with real implementation
  auto fake = ColorMapper<Precision>();
  return fake;
}
 
 /**
 *
 */ 
ColorMapper<Precision>& SimpleHDVizDataImpl::getDColorMap() {
  // TODO: Replace with real implementation
  auto fake = ColorMapper<Precision>();
  return fake;
}

/**
 *
 */
int SimpleHDVizDataImpl::getMinPersistenceLevel() { 
  // TODO: Replace with real implementation
  return 0;
}

/**
 *
 */
int SimpleHDVizDataImpl::getMaxPersistenceLevel() { 
  // TODO: Replace with real implementation
  return 0;
}

/**
 *
 */
void SimpleHDVizDataImpl::setLayout(HDVizLayout layout, int level) {
  /* No work necessary. */
};

/**
 *
 */
void SimpleHDVizDataImpl::loadData(int level) {
  /* No work necessary. */
};
