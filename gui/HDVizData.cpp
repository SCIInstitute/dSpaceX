#include "HDVizData.h"
#include <stdexcept>

const std::string k_defaultPath = "./";
const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";

HDVizData::HDVizData(std::string path) {
  layout = HDVizLayout::ISOMAP;
  L = nullptr;
  selectedCell  = 0;
  selectedPoint = 0;
  nSamples = 50;

  std::string persistenceDataHeaderFilename = k_defaultPersistenceDataHeaderFilename;
  std::string persistenceStartHeaderFilename = k_defaultPersistenceStartHeaderFilename;
  std::string geomDataHeaderFilename = k_defaultGeomDataHeaderFilename;
  std::string parameterNamesFilename = k_defaultParameterNamesFilename;
  m_path = path.empty() ? k_defaultPath : path;
  std::cout << "Loading Data from: " << m_path << std::endl;

  pSorted = FortranLinalg::LinalgIO<Precision>::readVector(m_path + persistenceDataHeaderFilename);
  currentLevel = pSorted.N() - 1;      
  FortranLinalg::DenseVector<Precision> tmp = 
      FortranLinalg::LinalgIO<Precision>::readVector(m_path + persistenceStartHeaderFilename);
  minLevel = tmp(0);

  FortranLinalg::DenseMatrix<Precision> G = 
      FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + geomDataHeaderFilename);
  Rmin = FortranLinalg::Linalg<Precision>::RowMin(G);
  Rmax = FortranLinalg::Linalg<Precision>::RowMax(G);
  nAll = G.N();
  names = FortranLinalg::DenseVector<std::string>(G.M());
  G.deallocate();

  std::ifstream nfile;
  nfile.open(m_path + parameterNamesFilename);
  if (!nfile.fail()) {
    for (unsigned int i=0; i<names.N(); i++) {
      getline(nfile, names(i)); 
    }
  }
  nfile.close();
  loadData();
};


Precision HDVizData::getSelectedCoordinate(int index) {
  return R[selectedCell](index, selectedPoint);
};

Precision HDVizData::getSelectedVariance(int index) {
  return Rvar[selectedCell](index, selectedPoint);
};


void HDVizData::increasePersistanceLevel() {
  setPersistenceLevel(currentLevel+1);
};
   
void HDVizData::decreasePersistanceLevel() {
  setPersistenceLevel(currentLevel-1);
};

int HDVizData::getPersistanceLevel() {
  return currentLevel;
};

void HDVizData::setPersistenceLevel(int pl, bool update) {
  currentLevel = pl;
  if (currentLevel > (int) pSorted.N()-1) {
    currentLevel = pSorted.N()-1;
  } else if(currentLevel < minLevel ) {
    currentLevel = minLevel;
  }

  loadData();

  if (selectedCell >= (int) edges.N()) {
    selectedCell = edges.N() - 1;
  }
  // if (update) {
  //   notifyChange();
  // }
}


void HDVizData::setLayout(HDVizLayout layout) {
  this->layout = layout;
  Lmin.deallocate();
  Lmax.deallocate();
  if (layout == HDVizLayout::ISOMAP) {
    Lmin = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "IsoMin.data.hdr");
    Lmax = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "IsoMax.data.hdr");
    loadLayout("_isolayout.data.hdr", "IsoExtremaLayout");
  } else if (layout == HDVizLayout::PCA) {
    Lmin = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCAMin.data.hdr");
    Lmax = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCAMax.data.hdr");
    loadLayout("_layout.data.hdr", "ExtremaLayout");
  } else if (layout == HDVizLayout::PCA2) {
    Lmin = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCA2Min.data.hdr");
    Lmax = FortranLinalg::LinalgIO<Precision>::readVector(m_path + "PCA2Max.data.hdr");
    loadLayout("_pca2layout.data.hdr", "PCA2ExtremaLayout");
  } else {
    throw std::invalid_argument("Unrecognized HDVizlayout specified.");
  }
};


void HDVizData::loadLayout(std::string type, std::string extFile) {
  for (unsigned int i=0; i < edges.N(); i++) {
    std::stringstream ss1;
    ss1 << "ps_" << currentLevel << "_crystal_" << i << type;
    L[i].deallocate();
    L[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + ss1.str());
  }
  //extrema layout 
  eL.deallocate();
  std::stringstream ss2;
  ss2 << extFile << "_" << currentLevel << ".data.hdr";

  eL = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + ss2.str());  

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

void HDVizData::loadData() {
  if (L != nullptr) {
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
     delete[] L;
     delete[] R;
     delete[] Rvar;
     delete[] gradR;
     delete[] yc;
     delete[] z;
     delete[] yw;
     delete[] yd;
  }

  //edges
  edges.deallocate(); 
  std::stringstream ss1; 
  ss1 << "Crystals_" << currentLevel << ".data.hdr";
  edges = FortranLinalg::LinalgIO<int>::readMatrix(m_path + ss1.str());


  //read layout information matrices
  L = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
  setLayout(layout);

  R = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
  Rvar = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
  gradR = new FortranLinalg::DenseMatrix<Precision>[edges.N()];
  loadReconstructions();


  //extrema function values
  ef.deallocate();      
  std::stringstream ss2; 
  ss2 << "ExtremaValues_" << currentLevel << ".data.hdr";
  ef = FortranLinalg::LinalgIO<Precision>::readVector(m_path + ss2.str());
  ez.deallocate();
  ez = FortranLinalg::DenseVector<Precision>(ef.N());
  efmin = FortranLinalg::Linalg<Precision>::Min(ef);
  efmax = FortranLinalg::Linalg<Precision>::Max(ef);
  
  std::stringstream ss3; 
  ss3 << "ExtremaWidths_" << currentLevel << ".data.hdr";
  ew.deallocate();
  ew = FortranLinalg::LinalgIO<Precision>::readVector(m_path + ss3.str());


  //Load color and width informations
  yc = new FortranLinalg::DenseVector<Precision>[edges.N()];
  z = new FortranLinalg::DenseVector<Precision>[edges.N()];
  yw = new FortranLinalg::DenseVector<Precision>[edges.N()];
  yd = new FortranLinalg::DenseVector<Precision>[edges.N()];

  loadColorValues("_fmean.data.hdr");
  loadWidthValues("_mdists.data.hdr");
  loadDensityValues("_spdf.data.hdr");
};


void HDVizData::loadColorValues(std::string type){ 
  for(unsigned int i=0; i<edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_" << currentLevel << "_crystal_" << i << type;
    yc[i].deallocate();
    yc[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + ss1.str());
    z[i] = FortranLinalg::DenseVector<Precision>(yc[i].N());
    FortranLinalg::Linalg<Precision>::Subtract(yc[i], efmin, z[i]);
    FortranLinalg::Linalg<Precision>::Scale(z[i], 1.f/(efmax-efmin), z[i]);
  }

  FortranLinalg::Linalg<Precision>::Subtract(ef, efmin, ez);
  FortranLinalg::Linalg<Precision>::Scale(ez, 1.f/(efmax-efmin), ez);

  colormap = ColorMapper<Precision>(efmin, efmax);
  colormap.set(0, 204.f/255.f, 210.f/255.f, 102.f/255.f, 204.f/255.f,
      41.f/255.f, 204.f/255.f, 0, 5.f/255.f);  
};



void HDVizData::loadWidthValues(std::string type){
  zmax = std::numeric_limits<Precision>::min();
  zmin = std::numeric_limits<Precision>::max();

  for(unsigned int i=0; i<edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_" << currentLevel << "_crystal_" << i << type;
    yw[i].deallocate();
    yw[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + ss1.str());    

    for(unsigned int k=0; k< yw[i].N(); k++){
      if(yw[i](k) < zmin){
        zmin = yw[i]( k);
      }
      if(yw[i](k) > zmax){
        zmax = yw[i](k);
      }
    }
  }

  for(unsigned int i=0; i<edges.N(); i++){
    FortranLinalg::Linalg<Precision>::Scale(yw[i], 0.3/zmax, yw[i]);
    FortranLinalg::Linalg<Precision>::Add(yw[i], 0.03, yw[i]);
  }

  FortranLinalg::Linalg<Precision>::Scale(ew, 0.3/zmax, ew);
  FortranLinalg::Linalg<Precision>::Add(ew, 0.03, ew);
};


void HDVizData::loadDensityValues(std::string type){
  Precision zmax = std::numeric_limits<Precision>::min();
  Precision zmin = std::numeric_limits<Precision>::max();

  for(unsigned int i=0; i<edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_" << currentLevel << "_crystal_" << i << type;
    yd[i].deallocate();
    yd[i] = FortranLinalg::LinalgIO<Precision>::readVector(m_path + ss1.str());    

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

void HDVizData::loadReconstructions(){
  for(unsigned int i=0; i< edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_" << currentLevel << "_crystal_" << i << "_Rs.data.hdr";
    R[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + ss1.str());

    std::stringstream ss2;
    ss2 << "ps_" << currentLevel << "_crystal_" << i << "_gradRs.data.hdr";
    gradR[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + ss2.str());

    std::stringstream ss3;
    ss3 << "ps_" << currentLevel << "_crystal_" << i << "_Svar.data.hdr";
    Rvar[i] = FortranLinalg::LinalgIO<Precision>::readMatrix(m_path + ss3.str());
  }

  //Rmin = FortranLinalg::Linalg<Precision>::ExtractColumn(R[0], 0);
  //Rmax = FortranLinalg::Linalg<Precision>::ExtractColumn(R[0], 0);
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

