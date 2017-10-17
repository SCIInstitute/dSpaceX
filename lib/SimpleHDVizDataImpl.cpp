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

  // Create Normalized Extrema Values, Mean Values, and Mins/Maxs
  extremaNormalized.resize(m_data->scaledPersistence.N());
  extremaWidthScaled.resize(m_data->scaledPersistence.N());
  meanNormalized.resize(m_data->scaledPersistence.N());
  efmin.resize(m_data->scaledPersistence.N());
  efmax.resize(m_data->scaledPersistence.N());  
  colormap.resize(m_data->scaledPersistence.N());
  dcolormap.resize(m_data->scaledPersistence.N());
  widthScaled.resize(m_data->scaledPersistence.N());

  // Resize unspecified quantity vectors
  // Note: The last value they are set to in FileCachedHDVizDataImpl
  //       before being used in OpenGL calls is min/max density values.
  //       HOWEVER, those are shadowed variables. So really they come from
  //       min/max width values. Yikes.
  widthMin.resize(m_data->scaledPersistence.N());
  widthMax.resize(m_data->scaledPersistence.N());

  for (unsigned int level = 0; level < m_data->scaledPersistence.N(); level++) {
    auto ef = m_data->extremaValues[level];
    efmin[level] = FortranLinalg::Linalg<Precision>::Min(ef);
    efmax[level] = FortranLinalg::Linalg<Precision>::Max(ef);
    
    auto ez = FortranLinalg::DenseVector<Precision>(ef.N());
    FortranLinalg::Linalg<Precision>::Subtract(ef, efmin[level], ez);
    FortranLinalg::Linalg<Precision>::Scale(ez, 1.f/(efmax[level] - efmin[level]), ez);
    extremaNormalized[level] = ez;

    auto yc = m_data->fmean[level];    
    auto z = std::vector<FortranLinalg::DenseVector<Precision>>(getEdges(level).N());
    auto yw = m_data->mdists[level];
    widthMin[level] = std::numeric_limits<Precision>::max();
    widthMax[level] = std::numeric_limits<Precision>::min();
    for (unsigned int i=0; i < getEdges(level).N(); i++) {      
      z[i] = FortranLinalg::DenseVector<Precision>(yc[i].N());
      FortranLinalg::Linalg<Precision>::Subtract(yc[i], efmin[level], z[i]);
      FortranLinalg::Linalg<Precision>::Scale(z[i], 1.f/(efmax[level]- efmin[level]), z[i]);            

      for(unsigned int k=0; k< yw[i].N(); k++){  
        if(yw[i](k) < widthMin[level]){
          widthMin[level] = yw[i]( k);
        }      
        if(yw[i](k) > widthMax[level]){
          widthMax[level] = yw[i](k);
        }
      }
    }
    meanNormalized[level] = z;

    widthScaled[level].resize(getEdges(level).N());
    for (unsigned int i=0; i < getEdges(level).N(); i++) { 
      auto width = FortranLinalg::Linalg<Precision>::Copy(yw[i]);
      FortranLinalg::Linalg<Precision>::Scale(width, 0.3/ widthMax[level], width);
      FortranLinalg::Linalg<Precision>::Add(width, 0.03, width);
      widthScaled[level][i] = width;
    }

    auto ew = m_data->extremaWidths[level];
    auto extremaWidth = FortranLinalg::Linalg<Precision>::Copy(ew);
    FortranLinalg::Linalg<Precision>::Scale(extremaWidth, 0.3/widthMax[level], extremaWidth);
    FortranLinalg::Linalg<Precision>::Add(extremaWidth, 0.03, extremaWidth);
    extremaWidthScaled[level] = extremaWidth;
     

    // Set up Color Maps
    colormap[level] = ColorMapper<Precision>(efmin[level], efmax[level]);
    colormap[level].set(0, 204.f/255.f, 210.f/255.f, 102.f/255.f, 204.f/255.f,
      41.f/255.f, 204.f/255.f, 0, 5.f/255.f);  

    // Set up Density Color Maps
    Precision densityMax = std::numeric_limits<Precision>::min();
    auto density = getDensity(level);
    for (unsigned int i=0; i < getEdges(level).N(); i++) {      
      for(unsigned int k=0; k < density[i].N(); k++){      
        if(density[i](k) > densityMax){
          densityMax = density[i](k);
        }
      }
    }     
    // TODO: Move color map creation completely outside of HDVizData impls.
    //    Expose densityMax via a class method and construct at viz time.
    dcolormap[level] = ColorMapper<Precision>(0, densityMax); 
    dcolormap[level].set(1, 0.5, 0, 1, 0.5, 0 , 1, 0.5, 0);  
  }

  // Calculate Geom min/max
  Rmin = FortranLinalg::Linalg<Precision>::RowMin(m_data->X);
  Rmax = FortranLinalg::Linalg<Precision>::RowMax(m_data->X);

  // Calculate Reconstruction min/max and Gradients min/max
  Rsmin.resize(m_data->scaledPersistence.N());
  Rsmax.resize(m_data->scaledPersistence.N());
  gRmin.resize(m_data->scaledPersistence.N());
  gRmax.resize(m_data->scaledPersistence.N());
  for (unsigned int level = 0; level < m_data->scaledPersistence.N(); level++) {
    Rsmin[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->R[level][0], 0);
    Rsmax[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->R[level][0], 0);
    gRmin[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->gradR[level][0], 0);
    gRmax[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->gradR[level][0], 0);

    for(unsigned int e = 0; e < getEdges(level).N(); e++){
      for(unsigned int i = 0; i < m_data->R[level][e].N(); i++){
        for(unsigned int j = 0; j < m_data->R[level][e].M(); j++){
          if(Rsmin[level](j) > m_data->R[level][e](j, i) - m_data->Rvar[level][e](j, i)){
            Rsmin[level](j) = m_data->R[level][e](j, i) - m_data->Rvar[level][e](j, i);
          }
          if(Rsmax[level](j) < m_data->R[level][e](j, i) + m_data->Rvar[level][e](j, i)){
            Rsmax[level](j) = m_data->R[level][e](j, i) + m_data->Rvar[level][e](j, i);
          }

          if(gRmin[level](j) > m_data->gradR[level][e](j, i)){
            gRmin[level](j) = m_data->gradR[level][e](j, i);
          }
          if(gRmax[level](j) < m_data->gradR[level][e](j, i)){
            gRmax[level](j) = m_data->gradR[level][e](j, i);
          }
        }
      }
    }
  }

  computeScaledLayouts();
  //
};

void SimpleHDVizDataImpl::computeScaledLayouts() {
  // Resize vectors
  scaledIsoLayout.resize(m_data->scaledPersistence.N());
  scaledPCALayout.resize(m_data->scaledPersistence.N());
  scaledPCA2Layout.resize(m_data->scaledPersistence.N());

  for (unsigned int level = 0; level < m_data->scaledPersistence.N(); level++) {    
    // Resize vectors
    scaledIsoLayout[level].resize(m_data->crystals[level].N());
    scaledPCALayout[level].resize(m_data->crystals[level].N());
    scaledPCA2Layout[level].resize(m_data->crystals[level].N());

    // Compute scaling factors
    FortranLinalg::DenseVector<Precision> isoDiff = FortranLinalg::Linalg<Precision>::Subtract(m_data->LmaxIso, m_data->LminIso);
    Precision rISO = std::max(isoDiff(0), isoDiff(1));
    FortranLinalg::Linalg<Precision>::Scale(isoDiff, 0.5f, isoDiff);
    FortranLinalg::Linalg<Precision>::Add(isoDiff, m_data->LminIso, isoDiff);

    FortranLinalg::DenseVector<Precision> pcaDiff = FortranLinalg::Linalg<Precision>::Subtract(m_data->LmaxPCA, m_data->LminPCA);
    Precision rPCA = std::max(pcaDiff(0), pcaDiff(1));
    FortranLinalg::Linalg<Precision>::Scale(pcaDiff, 0.5f, pcaDiff);
    FortranLinalg::Linalg<Precision>::Add(pcaDiff, m_data->LminPCA, pcaDiff);

    FortranLinalg::DenseVector<Precision> pca2Diff = FortranLinalg::Linalg<Precision>::Subtract(m_data->LmaxPCA2, m_data->LminPCA2);
    Precision rPCA2 = std::max(pca2Diff(0), pca2Diff(1));
    FortranLinalg::Linalg<Precision>::Scale(pca2Diff, 0.5f, pca2Diff);
    FortranLinalg::Linalg<Precision>::Add(pca2Diff, m_data->LminPCA2, pca2Diff);


    for (unsigned int crystal = 0; crystal < m_data->crystals[level].N(); crystal++) { 
      // copy layout matrices
      scaledIsoLayout[level][crystal] = 
          FortranLinalg::Linalg<Precision>::Copy(m_data->IsoLayout[level][crystal]);
      scaledPCALayout[level][crystal] = 
          FortranLinalg::Linalg<Precision>::Copy(m_data->PCALayout[level][crystal]);
      scaledPCA2Layout[level][crystal] = 
          FortranLinalg::Linalg<Precision>::Copy(m_data->PCA2Layout[level][crystal]);

      // perform scaling
      FortranLinalg::Linalg<Precision>::AddColumnwise(scaledIsoLayout[level][crystal], isoDiff, scaledIsoLayout[level][crystal]);
      FortranLinalg::Linalg<Precision>::Scale(scaledIsoLayout[level][crystal], 2.f/rISO, scaledIsoLayout[level][crystal]);

      FortranLinalg::Linalg<Precision>::AddColumnwise(scaledPCALayout[level][crystal], pcaDiff, scaledPCALayout[level][crystal]);
      FortranLinalg::Linalg<Precision>::Scale(scaledPCALayout[level][crystal], 2.f/rPCA, scaledPCALayout[level][crystal]);

      FortranLinalg::Linalg<Precision>::AddColumnwise(scaledPCA2Layout[level][crystal], pca2Diff, scaledPCA2Layout[level][crystal]);
      FortranLinalg::Linalg<Precision>::Scale(scaledPCA2Layout[level][crystal], 2.f/rPCA2, scaledPCA2Layout[level][crystal]);
    }     
  }
}

/**
 *
 */
int SimpleHDVizDataImpl::getNumberOfSamples() {
  return k_defaultSamplesCount;
}

/**
 *
 */
FortranLinalg::DenseMatrix<int>& SimpleHDVizDataImpl::getEdges(int persistenceLevel) {  
  return m_data->crystals[persistenceLevel];
}    

/**
 *
 */
std::vector<FortranLinalg::DenseMatrix<Precision>>& SimpleHDVizDataImpl::getLayout(
    HDVizLayout layout, int persistenceLevel) {  
  switch (layout) {
    case HDVizLayout::ISOMAP : 
      // return m_data->IsoLayout[persistenceLevel];
      return scaledIsoLayout[persistenceLevel];
      break;
    case HDVizLayout::PCA : 
      // return m_data->PCALayout[persistenceLevel];
      return scaledPCALayout[persistenceLevel];
      break;
    case HDVizLayout::PCA2 :
      // return  m_data->PCA2Layout[persistenceLevel];
      return scaledPCA2Layout[persistenceLevel];
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
FortranLinalg::DenseVector<Precision>& SimpleHDVizDataImpl::getExtremaWidthsScaled(
     int persistenceLevel) {
  return extremaWidthScaled[persistenceLevel];
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
Precision SimpleHDVizDataImpl::getWidthMin(int persistenceLevel) {
  return widthMin[persistenceLevel];
}

/**
 *
 */
Precision SimpleHDVizDataImpl::getWidthMax(int persistenceLevel) {
  return widthMax[persistenceLevel];
}

/**
 *
 */
std::vector<FortranLinalg::DenseVector<Precision>>& SimpleHDVizDataImpl::getMean(
    int persistenceLevel) {
  return m_data->fmean[persistenceLevel];
}

/**
 *
 */
std::vector<FortranLinalg::DenseVector<Precision>>& SimpleHDVizDataImpl::getMeanNormalized(
    int persistenceLevel) {
  return meanNormalized[persistenceLevel];
}

/**
 *
 */
std::vector<FortranLinalg::DenseVector<Precision>>& SimpleHDVizDataImpl::getWidth(
    int persistenceLevel) {
  return m_data->mdists[persistenceLevel];
}

/**
 * 
 */
std::vector<FortranLinalg::DenseVector<Precision>>& SimpleHDVizDataImpl::getWidthScaled(
    int persistenceLevel) {
  return widthScaled[persistenceLevel];
}

/**
 *
 */
std::vector<FortranLinalg::DenseVector<Precision>>& SimpleHDVizDataImpl::getDensity(
    int persistenceLevel) {
  return m_data->spdf[persistenceLevel];
}

/**
 *
 */
ColorMapper<Precision>& SimpleHDVizDataImpl::getColorMap(int persistenceLevel) {
  // TODO: Color maps have no business in this data structure. Move out.
  return colormap[persistenceLevel];
}
 
 /**
 *
 */ 
ColorMapper<Precision>& SimpleHDVizDataImpl::getDColorMap(int persistenceLevel) {
  // TODO: Color maps have no business in this data structure. Move out.
  return dcolormap[persistenceLevel];
}

/**
 *
 */
int SimpleHDVizDataImpl::getMinPersistenceLevel() { 
  return m_data->minLevel(0);
}

/**
 *
 */
int SimpleHDVizDataImpl::getMaxPersistenceLevel() { 
  // TODO: Move into HDProcessResult as 'maxLevel'.
  return m_data->scaledPersistence.N() - 1;
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
