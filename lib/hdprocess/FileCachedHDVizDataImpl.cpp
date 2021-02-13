#include "FileCachedHDVizDataImpl.h"
#include <stdexcept>

const std::string k_defaultPath = "./";
const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";
const int k_defaultSamplesCount = 50;

FileCachedHDVizDataImpl::FileCachedHDVizDataImpl(std::string path) {
  m_currentLayout = HDVizLayout::ISOMAP;
  L = std::vector<FortranLinalg::DenseMatrix<Precision>>(0);
  nSamples = k_defaultSamplesCount;

  std::string persistenceDataHeaderFilename = k_defaultPersistenceDataHeaderFilename;
  std::string persistenceStartHeaderFilename = k_defaultPersistenceStartHeaderFilename;
  std::string geomDataHeaderFilename = k_defaultGeomDataHeaderFilename;
  std::string parameterNamesFilename = k_defaultParameterNamesFilename;
  m_path = path.empty() ? k_defaultPath : path;


  m_knn = FortranLinalg::LinalgIO<int>::readMatrix(m_path + "KNN.data.hdr");
  pSorted = FortranLinalg::LinalgIO<Precision>::readVector(m_path + persistenceDataHeaderFilename);
  maxLevel = pSorted.N() - 1;      
  FortranLinalg::DenseVector<Precision> tmp = 
      FortranLinalg::LinalgIO<Precision>::readVector(m_path + persistenceStartHeaderFilename);
  minLevel = tmp(0);

  FortranLinalg::DenseMatrix<Precision> G = 
      FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + geomDataHeaderFilename);
  Rmin = FortranLinalg::Linalg<Precision>::RowMin(G);
  Rmax = FortranLinalg::Linalg<Precision>::RowMax(G);
  
  m_names = FortranLinalg::DenseVector<std::string>(G.M());
  G.deallocate();

  std::ifstream nfile;
  nfile.open(m_path + parameterNamesFilename);
  if (!nfile.fail()) {
    for (unsigned int i=0; i < m_names.N(); i++) {
      getline(nfile, m_names(i)); 
    }
  }
  nfile.close();
  m_currentLevel = maxLevel;
  loadData(m_currentLevel);
};


FortranLinalg::DenseMatrix<Precision>& FileCachedHDVizDataImpl::getX() {
  // TODO:  Implement this function. 
  FortranLinalg::DenseMatrix<Precision> x;
  return x;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getY() {
  // TODO:  Implement this function. 
  FortranLinalg::DenseVector<Precision> y;
  return y; 
}

FortranLinalg::DenseMatrix<int>& FileCachedHDVizDataImpl::getNearestNeighbors() {
  return m_knn;
}

FortranLinalg::DenseMatrix<int>& FileCachedHDVizDataImpl::getCrystals(int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return edges;
}    

std::vector<FortranLinalg::DenseMatrix<Precision>>& FileCachedHDVizDataImpl::getLayout(
    HDVizLayout layout, int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  maybeSwapLayoutCache(layout);
  return L;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getPersistence() {
  return pSorted;
}

FortranLinalg::DenseVector<std::string>& FileCachedHDVizDataImpl::getNames() {
  return m_names;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaValues(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return ef;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaNormalized(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return ez;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaWidths(
    int persistenceLevel) {
  // TODO: This function is currently returning extrema widths scaled. Factor out
  //       that logic and return the original extrema widths from this function.
  maybeSwapLevelCache(persistenceLevel);
  return ew;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaWidthsScaled(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return ew;
}

FortranLinalg::DenseMatrix<Precision>& FileCachedHDVizDataImpl::getExtremaLayout(
    HDVizLayout layout, int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  maybeSwapLayoutCache(layout);
  return eL;
}

std::vector<FortranLinalg::DenseMatrix<Precision>>& FileCachedHDVizDataImpl::getReconstruction(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return R;
}

std::vector<FortranLinalg::DenseMatrix<Precision>>& FileCachedHDVizDataImpl::getVariance(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return Rvar;
}

std::vector<FortranLinalg::DenseMatrix<Precision>>& FileCachedHDVizDataImpl::getGradient(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return gradR;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRMin() {
  return Rmin;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRMax() {
  return Rmax;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRsMin(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return Rsmin;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRsMax(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return Rsmax;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getGradientMin(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return gRmin;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getGradientMax(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return gRmax;
}

Precision FileCachedHDVizDataImpl::getExtremaMinValue(int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return efmin;
}
  
Precision FileCachedHDVizDataImpl::getExtremaMaxValue(int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return efmax;
}

Precision FileCachedHDVizDataImpl::getWidthMin(int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return widthMin;
}

Precision FileCachedHDVizDataImpl::getWidthMax(int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return widthMax;
}

std::vector<FortranLinalg::DenseVector<Precision>>& FileCachedHDVizDataImpl::getMean(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return yc;
}

std::vector<FortranLinalg::DenseVector<Precision>>& FileCachedHDVizDataImpl::getMeanNormalized(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return z;
}

std::vector<FortranLinalg::DenseVector<Precision>>& FileCachedHDVizDataImpl::getWidth(
    int persistenceLevel) {
  // TODO: This function is currently returning widths scaled. Factor out
  //       that logic and return the original widths from this function.
  maybeSwapLevelCache(persistenceLevel);
  return yw;
}

std::vector<FortranLinalg::DenseVector<Precision>>& FileCachedHDVizDataImpl::getWidthScaled(
    int persistenceLevel) {  
  maybeSwapLevelCache(persistenceLevel);
  return yw;
}

std::vector<FortranLinalg::DenseVector<Precision>>& FileCachedHDVizDataImpl::getDensity(
    int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return yd;
}

ColorMapper<Precision>& FileCachedHDVizDataImpl::getColorMap(
  int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return colormap;
}
  
ColorMapper<Precision>& FileCachedHDVizDataImpl::getDColorMap(
  int persistenceLevel) {
  maybeSwapLevelCache(persistenceLevel);
  return dcolormap;
}

int FileCachedHDVizDataImpl::getMinPersistenceLevel() const {
  return minLevel; 
}

int FileCachedHDVizDataImpl::getMaxPersistenceLevel() const { 
  return maxLevel; 
}

void FileCachedHDVizDataImpl::setLayout(HDVizLayout layout, int level) {
  m_currentLayout = layout;
  Lmin.deallocate();
  Lmax.deallocate();
  switch (layout) {
    case HDVizLayout::ISOMAP:
      loadLayout("_isolayout.data.hdr", "IsoExtremaLayout", 
          m_path + "IsoMin.data.hdr", m_path + "IsoMax.data.hdr", level);
      break;

    case HDVizLayout::PCA:
      loadLayout("_layout.data.hdr", "ExtremaLayout", 
          m_path + "PCAMin.data.hdr", m_path + "PCAMax.data.hdr", level);  
      break;    

    case HDVizLayout::PCA2:
      loadLayout("_pca2layout.data.hdr", "PCA2ExtremaLayout",
          m_path + "PCA2Min.data.hdr", m_path + "PCA2Max.data.hdr", level);
      break;

    default:  
      throw std::invalid_argument("Unrecognized HDVizlayout specified.");
      break;
  }
};


void FileCachedHDVizDataImpl::loadLayout(std::string type, std::string extFile, 
    std::string minFile, std::string maxFile, int level) {
  Lmin = FortranLinalg::LinalgIO<Precision>::readVector(minFile);
  Lmax = FortranLinalg::LinalgIO<Precision>::readVector(maxFile);

  for (unsigned int i = 0; i < edges.N(); i++) {
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    L[i].deallocate();
    L[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + filename);
  }
  // Extrema layout 
  eL.deallocate();
  std::string eL_Filename = extFile + "_" + std::to_string(level) + ".data.hdr";

  eL = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + eL_Filename);  

 
  FortranLinalg::DenseVector<Precision> diff = FortranLinalg::Linalg<Precision>::Subtract(Lmax, Lmin);
  Precision r = std::max(diff(0), diff(1));
  FortranLinalg::Linalg<Precision>::Scale(diff, 0.5f, diff);
  FortranLinalg::Linalg<Precision>::Add(diff, Lmin, diff);

  for (unsigned int i=0; i < edges.N(); i++) {
    FortranLinalg::Linalg<Precision>::AddColumnwise(L[i], diff, L[i]);
    FortranLinalg::Linalg<Precision>::Scale(L[i], 2.f/r, L[i]);
  }

  FortranLinalg::Linalg<Precision>::AddColumnwise(eL, diff, eL);
  FortranLinalg::Linalg<Precision>::Scale(eL, 2.f/r, eL);

  nLayoutSamples = L[0].N();
};

void FileCachedHDVizDataImpl::loadData(int level) {
  m_currentLevel = level;
  if (!L.empty()) {
    for (unsigned int i=0; i<edges.N(); i++) {
      L[i].deallocate();
      R[i].deallocate();
      Rvar[i].deallocate();
      gradR[i].deallocate();
      yc[i].deallocate();
      z[i].deallocate();
      yw[i].deallocate();
      yd[i].deallocate();
    }
  }

  // Edges
  edges.deallocate(); 
  std::string crystalsFilename = "Crystals_" + std::to_string(level) + ".data.hdr";
  edges = FortranLinalg::LinalgIO<int>::readMatrix(m_path + crystalsFilename);


  // Read layout information matrices.
  L = std::vector<FortranLinalg::DenseMatrix<Precision>>(edges.N());
  setLayout(m_currentLayout, level);

  R = std::vector<FortranLinalg::DenseMatrix<Precision>>(edges.N());
  Rvar = std::vector<FortranLinalg::DenseMatrix<Precision>>(edges.N());
  gradR = std::vector<FortranLinalg::DenseMatrix<Precision>>(edges.N());
  loadReconstructions(level);


  // Extrema function values
  ef.deallocate();      
  std::string extremaValuesFilename = "ExtremaValues_" + std::to_string(level) + ".data.hdr";
  ef = FortranLinalg::LinalgIO<Precision>::readVector(m_path + extremaValuesFilename);
  
  // Create Normalized Extrema Values
  ez.deallocate();
  ez = FortranLinalg::DenseVector<Precision>(ef.N());
  efmin = FortranLinalg::Linalg<Precision>::Min(ef);
  efmax = FortranLinalg::Linalg<Precision>::Max(ef);
  FortranLinalg::Linalg<Precision>::Subtract(ef, efmin, ez);
  FortranLinalg::Linalg<Precision>::Scale(ez, 1.f/(efmax-efmin), ez);
  
  std::string extremaWidthsFilename = "ExtremaWidths_" + std::to_string(level) + ".data.hdr";
  ew.deallocate();
  ew = FortranLinalg::LinalgIO<Precision>::readVector(m_path + extremaWidthsFilename);


  // Load color and width informations.
  yc = std::vector<FortranLinalg::DenseVector<Precision>>(edges.N());
  z = std::vector<FortranLinalg::DenseVector<Precision>>(edges.N());
  yw = std::vector<FortranLinalg::DenseVector<Precision>>(edges.N());
  yd = std::vector<FortranLinalg::DenseVector<Precision>>(edges.N());

  loadColorValues("_fmean.data.hdr", level);
  loadWidthValues("_mdists.data.hdr", level);
  loadDensityValues("_spdf.data.hdr", level);
};


void FileCachedHDVizDataImpl::loadColorValues(std::string type, int level){ 
  for(unsigned int i=0; i<edges.N(); i++){
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    yc[i].deallocate();
    yc[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + filename);
    z[i] = FortranLinalg::DenseVector<Precision>(yc[i].N());
    FortranLinalg::Linalg<Precision>::Subtract(yc[i], efmin, z[i]);
    FortranLinalg::Linalg<Precision>::Scale(z[i], 1.f/(efmax-efmin), z[i]);
  }

  colormap = ColorMapper<Precision>(efmin, efmax);
  colormap.set(0, 204.f/255.f, 210.f/255.f, 102.f/255.f, 204.f/255.f,
      41.f/255.f, 204.f/255.f, 0, 5.f/255.f);  
};



void FileCachedHDVizDataImpl::loadWidthValues(std::string type, int level){
  widthMax = std::numeric_limits<Precision>::min();   // TODO: Why is zmax being reused here?
  widthMin = std::numeric_limits<Precision>::max();   // TODO: Why is zmin being reused here?

  for(unsigned int i = 0; i < edges.N(); i++){
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    yw[i].deallocate();
    yw[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + filename);    

    for(unsigned int k=0; k< yw[i].N(); k++){
      if(yw[i](k) < widthMin){
        widthMin = yw[i]( k);
      }
      if(yw[i](k) > widthMax){
        widthMax = yw[i](k);
      }
    }
  }

  for(unsigned int i = 0; i < edges.N(); i++){
    FortranLinalg::Linalg<Precision>::Scale(yw[i], 0.3/widthMax, yw[i]);
    FortranLinalg::Linalg<Precision>::Add(yw[i], 0.03, yw[i]);
  }

  FortranLinalg::Linalg<Precision>::Scale(ew, 0.3/widthMax, ew);
  FortranLinalg::Linalg<Precision>::Add(ew, 0.03, ew);
};


void FileCachedHDVizDataImpl::loadDensityValues(std::string type, int level) {  
  Precision densityMax = std::numeric_limits<Precision>::min();
  Precision densityMin = std::numeric_limits<Precision>::max();

  for(unsigned int i=0; i<edges.N(); i++){
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    yd[i].deallocate();
    yd[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + filename);    

    for(unsigned int k=0; k< yd[i].N(); k++){
      if(yd[i](k) < densityMin){
        densityMin = yd[i]( k);
      }
      if(yd[i](k) > densityMax){
        densityMax = yd[i](k);
      }
    }
  }
  dcolormap = ColorMapper<Precision>(0, densityMax); 
  dcolormap.set(1, 0.5, 0, 1, 0.5, 0 , 1, 0.5, 0);  
};

void FileCachedHDVizDataImpl::loadReconstructions(int level){
  for(unsigned int i=0; i< edges.N(); i++){
    std::string baseFilename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i);
    std::string rFilename = baseFilename + "_Rs.data.hdr";
    R[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + rFilename);

    std::string gradrFilename = baseFilename + "_gradRs.data.hdr";
    gradR[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + gradrFilename);

    std::string svarFilename = baseFilename + "_Svar.data.hdr";
    Rvar[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + svarFilename);
  }

  // Rmin = FortranLinalg::Linalg<Precision>::ExtractColumn(R[0], 0);
  // Rmax = FortranLinalg::Linalg<Precision>::ExtractColumn(R[0], 0);
  Rvmin = FortranLinalg::Linalg<Precision>::ExtractColumn(Rvar[0], 0);
  Rvmax = FortranLinalg::Linalg<Precision>::ExtractColumn(Rvar[0], 0);
  Rsmin = FortranLinalg::Linalg<Precision>::ExtractColumn(R[0], 0);
  Rsmax = FortranLinalg::Linalg<Precision>::ExtractColumn(R[0], 0);
  gRmin = FortranLinalg::Linalg<Precision>::ExtractColumn(gradR[0], 0);
  gRmax = FortranLinalg::Linalg<Precision>::ExtractColumn(gradR[0], 0);
  for(unsigned int e=0; e<edges.N(); e++){
    for(unsigned int i=0; i<R[e].N(); i++){
      for(unsigned int j=0; j< R[e].M(); j++){
        if(Rsmin(j) > R[e](j, i) - Rvar[e](j, i)){
          Rsmin(j) = R[e](j, i) - Rvar[e](j, i);
        }
        if(Rsmax(j) < R[e](j, i) + Rvar[e](j, i)){
          Rsmax(j) = R[e](j, i) + Rvar[e](j, i);
        }

        if(Rvmin(j) > Rvar[e](j, i)){
          Rvmin(j) = Rvar[e](j, i);
        }
        if(Rvmax(j) < Rvar[e](j, i)){
          Rvmax(j) = Rvar[e](j, i);
        }

        if(gRmin(j) > gradR[e](j, i)){
          gRmin(j) = gradR[e](j, i);
        }
        if(gRmax(j) < gradR[e](j, i)){
          gRmax(j) = gradR[e](j, i);
        }
      }
    }
  }
  vmax = 0;
  for(unsigned int i=0; i<Rvmax.N(); i++){
    if(vmax < Rvmax(i)){
      vmax = Rvmax(i);
    }
  }
};

/**
 *
 */
void FileCachedHDVizDataImpl::maybeSwapLevelCache(int level) {
  if (level != m_currentLevel) {
    loadData(level);    
  }
}

/**
 *
 */
void FileCachedHDVizDataImpl::maybeSwapLayoutCache(HDVizLayout layout) {
  if (layout != m_currentLayout) {
    setLayout(layout, m_currentLevel);
  }
}


