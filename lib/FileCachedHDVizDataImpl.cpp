#include "FileCachedHDVizDataImpl.h"
#include <stdexcept>

const std::string k_defaultPath = "./";
const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";
const int k_defaultSamplesCount = 50;

FileCachedHDVizDataImpl::FileCachedHDVizDataImpl(std::string path) {
  layout = HDVizLayout::ISOMAP;
  L = std::vector<FortranLinalg::DenseMatrix<Precision>>(0);
  nSamples = k_defaultSamplesCount;

  std::string persistenceDataHeaderFilename = k_defaultPersistenceDataHeaderFilename;
  std::string persistenceStartHeaderFilename = k_defaultPersistenceStartHeaderFilename;
  std::string geomDataHeaderFilename = k_defaultGeomDataHeaderFilename;
  std::string parameterNamesFilename = k_defaultParameterNamesFilename;
  m_path = path.empty() ? k_defaultPath : path;
  std::cout << "Loading Data from: " << m_path << std::endl;

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
  loadData(maxLevel);
};


Precision FileCachedHDVizDataImpl::getSelectedCoordinate(
    int persistenceLevel, int selectedCell, int selectedPoint, int index) {
  // TODO: Add call check to enforce that persistenceLevel == cachedPersistenceLevel.
  return R[selectedCell](index, selectedPoint);
}

Precision FileCachedHDVizDataImpl::getSelectedVariance(
    int persistenceLevel, int selectedCell, int selectedPoint, int index) {
  // TODO: Add call check to enforce that persistenceLevel == cachedPersistenceLevel.
  return Rvar[selectedCell](index, selectedPoint);
}

FortranLinalg::DenseMatrix<int>& FileCachedHDVizDataImpl::getEdges(int persistenceLevel) {
  // TODO: Add call check to enforce that persistenceLevel == cachedPersistenceLevel.
  return edges;
}    

std::vector<FortranLinalg::DenseMatrix<Precision>>& FileCachedHDVizDataImpl::getLayout(
    HDVizLayout layout, int persistenceLevel) {
  // TODO: Add call check to enforce that:
  //       layout == cachedLayout  &&
  //       persistenceLevel == cachedPersistenceLevel
  return L;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getPersistence() {
  return pSorted;
}

FortranLinalg::DenseVector<std::string>& FileCachedHDVizDataImpl::getNames() {
  return m_names;
}

int FileCachedHDVizDataImpl::getNumberOfSamples() {
  return nSamples;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaValues(
    int persistenceLevel) {
  // TODO: Add call check to enforce that persistenceLevel == cachedPersistenceLevel.
  return ef;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaNormalized(
    int persistenceLevel) {
  // TODO: Add call check to enforce that persistenceLevel == cachedPersistenceLevel.
  return ez;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getExtremaWidths() {
  return ew;
}

FortranLinalg::DenseMatrix<Precision>& FileCachedHDVizDataImpl::getExtremaLayout() {
  return eL;
}

FortranLinalg::DenseMatrix<Precision>* FileCachedHDVizDataImpl::getReconstruction() {
  return R;
}

FortranLinalg::DenseMatrix<Precision>* FileCachedHDVizDataImpl::getVariance() {
  return Rvar;
}

FortranLinalg::DenseMatrix<Precision>* FileCachedHDVizDataImpl::getGradient() {
  return gradR;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRMin() {
  return Rmin;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRMax() {
  return Rmax;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRsMin() {
  return Rsmin;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getRsMax() {
  return Rsmax;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getGradientMin() {
  return gRmin;
}

FortranLinalg::DenseVector<Precision>& FileCachedHDVizDataImpl::getGradientMax() {
  return gRmax;
}

Precision FileCachedHDVizDataImpl::getExtremaMinValue() {
  return efmin;
}
  
Precision FileCachedHDVizDataImpl::getExtremaMaxValue() {
  return efmax;
}

Precision FileCachedHDVizDataImpl::getZMin() {
  return zmin;
}

Precision FileCachedHDVizDataImpl::getZMax() {
  return zmax;
}

FortranLinalg::DenseVector<Precision>* FileCachedHDVizDataImpl::getValueColor() {
  return yc;
}

FortranLinalg::DenseVector<Precision>* FileCachedHDVizDataImpl::getZ() {
  return z;
}

FortranLinalg::DenseVector<Precision>* FileCachedHDVizDataImpl::getWidth() {
  return yw;
}

FortranLinalg::DenseVector<Precision>* FileCachedHDVizDataImpl::getDensity() {
  return yd;
}

ColorMapper<Precision>& FileCachedHDVizDataImpl::getColorMap() {
  return colormap;
}
  
ColorMapper<Precision>& FileCachedHDVizDataImpl::getDColorMap() {
  return dcolormap;
}

int FileCachedHDVizDataImpl::getMinPersistenceLevel() { 
  return minLevel; 
}

int FileCachedHDVizDataImpl::getMaxPersistenceLevel() { 
  return maxLevel; 
}

void FileCachedHDVizDataImpl::setLayout(HDVizLayout layout, int level) {
  this->layout = layout;
  Lmin.deallocate();
  Lmax.deallocate();
  if (layout == HDVizLayout::ISOMAP) {
    Lmin = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "IsoMin.data.hdr");
    Lmax = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "IsoMax.data.hdr");
    loadLayout("_isolayout.data.hdr", "IsoExtremaLayout", level);
  } else if (layout == HDVizLayout::PCA) {
    Lmin = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCAMin.data.hdr");
    Lmax = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCAMax.data.hdr");
    loadLayout("_layout.data.hdr", "ExtremaLayout", level);
  } else if (layout == HDVizLayout::PCA2) {
    Lmin = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCA2Min.data.hdr");
    Lmax = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCA2Max.data.hdr");
    loadLayout("_pca2layout.data.hdr", "PCA2ExtremaLayout", level);
  } else {
    throw std::invalid_argument("Unrecognized HDVizlayout specified.");
  }
};


void FileCachedHDVizDataImpl::loadLayout(std::string type, std::string extFile, int level) {
  for (unsigned int i = 0; i < edges.N(); i++) {
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    L[i].deallocate();
    L[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + filename);
  }
  // Extrema layout 
  eL.deallocate();
  std::string eL_Filename = extFile + "_" + std::to_string(level) + ".data.hdr";

  eL = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + eL_Filename);  

  /*Lmin = FortranLinalg::Linalg<Precision>::ExtractColumn(L[0], 0);
  Lmax = FortranLinalg::Linalg<Precision>::ExtractColumn(L[0], 0);
  for(unsigned int e=0; e<edges.N(); e++){
    for(unsigned int i=0; i<L[e].N(); i++){
      for(unsigned int j=0; j< L[e].M(); j++){
        if(Lmin(j) > L[e](j, i)){
          Lmin(j) = L[e](j, i);
        }
        if(Lmax(j) < L[e](j, i)){
          Lmax(j) = L[e](j, i);
        }
      }
    }
  }*/

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

  nSamples = L[0].N();
};

void FileCachedHDVizDataImpl::loadData(int level) {
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
     
    delete[] R;
    delete[] Rvar;
    delete[] gradR;
    delete[] yc;
    delete[] z;
    delete[] yw;
    delete[] yd;
  }

  // Edges
  edges.deallocate(); 
  std::string crystalsFilename = "Crystals_" + std::to_string(level) + ".data.hdr";
  edges = FortranLinalg::LinalgIO<int>::readMatrix(m_path + crystalsFilename);


  // Read layout information matrices.
  L = std::vector<FortranLinalg::DenseMatrix<Precision>>(edges.N());
  setLayout(layout, level);

  R = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
  Rvar = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
  gradR = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
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
  yc = new FortranLinalg::DenseVector<Precision>[edges.N()];
  z = new FortranLinalg::DenseVector<Precision>[edges.N()];
  yw = new FortranLinalg::DenseVector<Precision>[edges.N()];
  yd = new FortranLinalg::DenseVector<Precision>[edges.N()];

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
  zmax = std::numeric_limits<Precision>::min();
  zmin = std::numeric_limits<Precision>::max();

  for(unsigned int i = 0; i < edges.N(); i++){
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    yw[i].deallocate();
    yw[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + filename);    

    for(unsigned int k=0; k< yw[i].N(); k++){
      if(yw[i](k) < zmin){
        zmin = yw[i]( k);
      }
      if(yw[i](k) > zmax){
        zmax = yw[i](k);
      }
    }
  }

  for(unsigned int i = 0; i < edges.N(); i++){
    FortranLinalg::Linalg<Precision>::Scale(yw[i], 0.3/zmax, yw[i]);
    FortranLinalg::Linalg<Precision>::Add(yw[i], 0.03, yw[i]);
  }

  FortranLinalg::Linalg<Precision>::Scale(ew, 0.3/zmax, ew);
  FortranLinalg::Linalg<Precision>::Add(ew, 0.03, ew);
};


void FileCachedHDVizDataImpl::loadDensityValues(std::string type, int level){
  Precision zmax = std::numeric_limits<Precision>::min();
  Precision zmin = std::numeric_limits<Precision>::max();

  for(unsigned int i=0; i<edges.N(); i++){
    std::string filename = "ps_" + std::to_string(level) + "_crystal_" + std::to_string(i) + type;
    yd[i].deallocate();
    yd[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + filename);    

    for(unsigned int k=0; k< yd[i].N(); k++){
      if(yd[i](k) < zmin){
        zmin = yd[i]( k);
      }
      if(yd[i](k) > zmax){
        zmax = yd[i](k);
      }
    }
  }
  dcolormap = ColorMapper<Precision>(0, zmax); 
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

