#include "SimpleHDVizDataImpl.h"
#include "dataset/ValueIndexPair.h"
#include <stdexcept>

using namespace dspacex;

// TODO: Move these constants into a shared location.
const std::string k_defaultPath = "./";
const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";

/**
 * SimpleHDVizDataImpl constuctor
 */
SimpleHDVizDataImpl::SimpleHDVizDataImpl(std::shared_ptr<HDProcessResult> result) : m_data(result) {
  assert(result != nullptr);
  auto num_samples = result->crystalPartitions[result->crystalPartitions.size()-1].size();  // same for all (computed) persistences (the top N levels)

  // the vector of values for the field
  auto& fieldvals = result->Y;

  // create vector of sample ids and their fieldvalues
  m_samples.resize(num_samples);
  for (int i = 0; i < num_samples; i++) {
    ValueIndexPair sample;
    sample.idx = i;
    sample.val = fieldvals[i];
    m_samples[i] = sample;
  }


  // TEMP DEBUG CODE

  // std::cout << "VECTOR / MATRIX SIZES" << std::endl;
  // std::cout << "scaledPersistence.N() = " << m_data->scaledPersistence.N() << std::endl;
  // std::cout << "minLevel.N() = " << m_data->minLevel.N() << std::endl;
  // std::cout << "getMinPersistenceLevel() = " << getMinPersistenceLevel() << std::endl;
  // std::cout << "getMaxPersistenceLevel() = " << getMaxPersistenceLevel() << std::endl;
  // for (unsigned int level = 0; level <= getMaxPersistenceLevel(); level++) {
  //   std::cout << " m_data->extremaValues[" << level << "].N() = " << m_data->extremaValues[level].N() << std::endl;
  // }

  /*
  std::cout << "X.N() = " << m_data->X.N() << std::endl;
  std::cout << "Y.N() = " << m_data->Y.N() << std::endl;
  for (unsigned int level = 0; level < m_data->scaledPersistence.N(); level++) {
    std::cout << "Persistence Level " << level << std::endl;
    std::cout << "    crystals.MxN() = " << m_data->crystals[level].M() << " x " << m_data->crystals[level].N() << std::endl;
    // std::cout << "    crystalPartitions.N() = " << m_data->crystalPartitions[level].N() << std::endl;
    std::cout << "    extremaValues.N() = " << m_data->extremaValues[level].N() << std::endl;
    std::cout << "    extremaWidths.N() = " << m_data->extremaWidths[level].N() << std::endl; 
    std::cout << "    IsoExtremaLayout.MxN() = " << m_data->IsoExtremaLayout[level].M() << " x " << m_data->IsoExtremaLayout[level].N() << std::endl;
    std::cout << "    Min(ef) = " <<  FortranLinalg::Linalg<Precision>::Min(m_data->extremaValues[level]) << std::endl;
    std::cout << "    Max(ef) = " <<  FortranLinalg::Linalg<Precision>::Max(m_data->extremaValues[level]) << std::endl;
    for (unsigned int crystal = 0; crystal < m_data->spdf[level].size(); crystal++) {
      std::cout << "  Crystal " << crystal << std::endl;
      std::cout << "     spdf[" << crystal << "].N() = " << m_data->spdf[level][crystal].N() << std::endl;
    }    
  }
  */
  // TEMP DEBUG CODE

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

  for (unsigned int level = getMinPersistenceLevel(); level < m_data->scaledPersistence.N(); level++) {
    auto ef = m_data->extremaValues[level];
    efmin[level] = FortranLinalg::Linalg<Precision>::Min(ef);
    efmax[level] = FortranLinalg::Linalg<Precision>::Max(ef);

    // Rescaling/min-max normalization of extrema
    auto ez = FortranLinalg::DenseVector<Precision>(ef.N());
    FortranLinalg::Linalg<Precision>::Subtract(ef, efmin[level], ez);
    FortranLinalg::Linalg<Precision>::Scale(ez, 1.f/(efmax[level] - efmin[level]), ez);
    extremaNormalized[level] = ez;

    auto yc = m_data->fmean[level];    
    auto z = std::vector<FortranLinalg::DenseVector<Precision>>(getCrystals(level).cols());
    auto yw = m_data->mdists[level];
    widthMin[level] = std::numeric_limits<Precision>::max();
    widthMax[level] = std::numeric_limits<Precision>::min();
    for (unsigned int i=0; i < getCrystals(level).cols(); i++) {
      z[i] = FortranLinalg::DenseVector<Precision>(yc[i].N());
      FortranLinalg::Linalg<Precision>::Subtract(yc[i], efmin[level], z[i]);
      FortranLinalg::Linalg<Precision>::Scale(z[i], 1.f/(efmax[level]- efmin[level]), z[i]);            

      for(unsigned int k=0; k< yw[i].N(); k++){  
        if(yw[i](k) < widthMin[level]){
          widthMin[level] = yw[i](k);
        }      
        if(yw[i](k) > widthMax[level]){
          widthMax[level] = yw[i](k);
        }
      }
    }
    meanNormalized[level] = z;

    widthScaled[level].resize(getCrystals(level).cols());
    for (unsigned int i=0; i < getCrystals(level).cols(); i++) {
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
    for (unsigned int i=0; i < getCrystals(level).cols(); i++) {
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
  for (unsigned int level = getMinPersistenceLevel(); level < m_data->scaledPersistence.N(); level++) {
    Rsmin[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->R[level][0], 0);
    Rsmax[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->R[level][0], 0);
    gRmin[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->gradR[level][0], 0);
    gRmax[level] = FortranLinalg::Linalg<Precision>::ExtractColumn(m_data->gradR[level][0], 0);

    for(unsigned int e = 0; e < getCrystals(level).cols(); e++){
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


  // Construct set of points belonging to each crystal, appending any that are missing (extrema and "ridgeline")
  // NOTE: the reason extrema may be missing is crystalPartitions is a 1:1 mapping between sample and crystal,
  //       yet mins, maxes, and ridgelines can be shared between crystals
  auto num_persistences = result->crystals.size();
  m_crystals.resize(num_persistences);
  m_extrema.resize(num_persistences);

  // print crystal helper
  auto printCrystal = [](auto crystal, int persistence=-1, int crystal_id=-1) {
                        std::cout << "p" << persistence << "c" << crystal_id << ":\n\t";
                        for (auto i: crystal) printf("%2d (%0.2f), ", i.idx, i.val);
                        std::cout << std::endl; };

  // construct arrays of samples and their values for each crystal of each persistence
  for (auto p = 0; p < num_persistences; p++) {
    auto num_crystals = result->extrema[p].size();  // m_extrema contains pairs of extrema for each crystal
    if (num_crystals == 0) continue; // only the top N persistence levels were calculated
    
    // add the extrema (vector of each crystal's max/min pairs of sample ids)
    m_extrema[p].resize(num_crystals);
    m_extrema[p] = result->extrema[p];

    // add each sample to its crystal
    m_crystals[p].resize(num_crystals);
    for (auto id = 0; id < num_samples; id++) {
      // use crystalsPartitions (to which crystal each sample belongs) to insert samples
      m_crystals[p][result->crystalPartitions[p][id]].push_back(m_samples[id]);
    }

    // sort crystal samples and add any that are missing
    // 0. sort crystal samples by value
    // 1. identify extrema of crystal and ensure they're included
    // 2/3. if nodes[1 /*minima-1*/] isn't one of minima's neighbors...
    //  - walk down from that node until we reach minima
    // 2/3. if nodes[n-2 /*maxima-1*/] isn't one of maxima's neighbors...
    //  - walk up from that node until we reach maxima
    for (auto c = 0; c < num_crystals; c++) {
      auto& crystal = m_crystals[p][c];

      // 0. sort each crystal by increasing fieldvalue
      std::sort(crystal.begin(), crystal.end(), ValueIndexPair::compare);

      // 1. add the extrema to their corresponding crystals if missing
      auto maxidx = m_extrema[p][c].first;
      auto minidx = m_extrema[p][c].second;
      if (crystal[crystal.size()-1].idx != maxidx)
        crystal.push_back(m_samples[maxidx]);
      if (crystal[0].idx != minidx)
        crystal.insert(crystal.begin(), m_samples[minidx]);

      // before augmentation with missing samples
      //printCrystal(crystal, p, c);
      
      // 2/3. Add shared "ridgeline" members of each crystal so paths between extrema is unbroken
      assert(crystal.size() >= 2);
      auto min_sample = crystal[0];
      auto max_sample = crystal[crystal.size() - 1];

      // smallest to min
      auto current = crystal[1];
      while (result->knng(1, current.idx) != -1 && result->knng(1, current.idx) != min_sample.idx) {
        // std::cout << current.idx << "'s sheerest neighbors (asc:dec): " << result->knng(1, current.idx)
        //           << ":" << result->knng(0, current.idx) << std::endl;

        // add current's neighbor of steepest descent and continue
        auto next = m_samples[result->knng(1, current.idx)];
        crystal.insert(crystal.begin()+1, next);
        current = next;
      }

      // largest to max
      current = crystal[crystal.size() - 2];
      while (result->knng(0, current.idx) != -1 && result->knng(0, current.idx) != max_sample.idx) {
        // add current's neighbor of steepest ascent and continue
        auto next = m_samples[result->knng(0, current.idx)];
        crystal.insert(crystal.end()-1, next);
        current = next;
      }

      // after augmentation to verify
      //printCrystal(crystal, p, c);
    }
  }

  computeScaledLayouts();
  //
} // END CONSTRUCTOR

void SimpleHDVizDataImpl::computeScaledLayouts() {
  // Resize vectors
  scaledIsoLayout.resize(m_data->scaledPersistence.N());
  scaledPCALayout.resize(m_data->scaledPersistence.N());
  scaledPCA2Layout.resize(m_data->scaledPersistence.N());
  scaledIsoExtremaLayout.resize(m_data->scaledPersistence.N());
  scaledPCAExtremaLayout.resize(m_data->scaledPersistence.N());
  scaledPCA2ExtremaLayout.resize(m_data->scaledPersistence.N());

  for (unsigned int level = getMinPersistenceLevel(); level < m_data->scaledPersistence.N(); level++) {    
    // Resize vectors
    scaledIsoLayout[level].resize(m_data->crystals[level].cols());
    scaledPCALayout[level].resize(m_data->crystals[level].cols());
    scaledPCA2Layout[level].resize(m_data->crystals[level].cols());

    // Copy extrema layout matrices
    scaledIsoExtremaLayout[level] = 
        FortranLinalg::Linalg<Precision>::Copy(m_data->IsoExtremaLayout[level]);
    scaledPCAExtremaLayout[level] =
        FortranLinalg::Linalg<Precision>::Copy(m_data->PCAExtremaLayout[level]);
    scaledPCA2ExtremaLayout[level] =
        FortranLinalg::Linalg<Precision>::Copy(m_data->PCA2ExtremaLayout[level]);

    // Compute scaling factors
    FortranLinalg::DenseVector<Precision> isoDiff = FortranLinalg::Linalg<Precision>::Subtract(m_data->LmaxIso, m_data->LminIso);
    Precision rISO = std::max(isoDiff(0), isoDiff(1));
    if (rISO <= 0.0) { rISO = 0.1; } // give it something to scale by if layouts are degenerate
    FortranLinalg::Linalg<Precision>::Scale(isoDiff, 0.5f, isoDiff);
    FortranLinalg::Linalg<Precision>::Add(isoDiff, m_data->LminIso, isoDiff);

    FortranLinalg::DenseVector<Precision> pcaDiff = FortranLinalg::Linalg<Precision>::Subtract(m_data->LmaxPCA, m_data->LminPCA);
    Precision rPCA = std::max(pcaDiff(0), pcaDiff(1));
    if (rPCA <= 0.0) { rPCA = 0.1; }
    FortranLinalg::Linalg<Precision>::Scale(pcaDiff, 0.5f, pcaDiff);
    FortranLinalg::Linalg<Precision>::Add(pcaDiff, m_data->LminPCA, pcaDiff);

    FortranLinalg::DenseVector<Precision> pca2Diff = FortranLinalg::Linalg<Precision>::Subtract(m_data->LmaxPCA2, m_data->LminPCA2);
    Precision rPCA2 = std::max(pca2Diff(0), pca2Diff(1));
    if (rPCA2 <= 0.0) { rPCA2 = 0.1; }
    FortranLinalg::Linalg<Precision>::Scale(pca2Diff, 0.5f, pca2Diff);
    FortranLinalg::Linalg<Precision>::Add(pca2Diff, m_data->LminPCA2, pca2Diff);

    // Peform scaling on extrema layout
    FortranLinalg::Linalg<Precision>::AddColumnwise(scaledIsoExtremaLayout[level], isoDiff, scaledIsoExtremaLayout[level]);
    FortranLinalg::Linalg<Precision>::Scale(scaledIsoExtremaLayout[level], 2.f/rISO, scaledIsoExtremaLayout[level]);

    FortranLinalg::Linalg<Precision>::AddColumnwise(scaledPCAExtremaLayout[level], pcaDiff, scaledPCAExtremaLayout[level]);
    FortranLinalg::Linalg<Precision>::Scale(scaledPCAExtremaLayout[level], 2.f/rPCA, scaledPCAExtremaLayout[level]);

    FortranLinalg::Linalg<Precision>::AddColumnwise(scaledPCA2ExtremaLayout[level], pca2Diff, scaledPCA2ExtremaLayout[level]);
    FortranLinalg::Linalg<Precision>::Scale(scaledPCA2ExtremaLayout[level], 2.f/rPCA2, scaledPCA2ExtremaLayout[level]);

    for (unsigned int crystal = 0; crystal < m_data->crystals[level].cols(); crystal++) {
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

FortranLinalg::DenseMatrix<Precision>& SimpleHDVizDataImpl::getX() {
  return m_data->X;
}

const std::vector<Precision>& SimpleHDVizDataImpl::getY() const {
  return m_data->Y;
}

/**
 *
 */
FortranLinalg::DenseMatrix<int>& SimpleHDVizDataImpl::getNearestNeighbors() {
  return m_data->knn;
}

/**
 *
 */
const Eigen::MatrixXi& SimpleHDVizDataImpl::getCrystals(int persistenceLevel) const {
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
      // return m_data->IsoExtremaLayout[persistenceLevel];
      return scaledIsoExtremaLayout[persistenceLevel];
      break;
    case HDVizLayout::PCA : 
      // return m_data->PCAExtremaLayout[persistenceLevel];
      return scaledPCAExtremaLayout[persistenceLevel];
      break;
    case HDVizLayout::PCA2 :
      // return  m_data->PCA2ExtremaLayout[persistenceLevel];
      return scaledPCA2ExtremaLayout[persistenceLevel];
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
int SimpleHDVizDataImpl::getMinPersistenceLevel() const { 
  return const_cast<SimpleHDVizDataImpl*>(this)->m_data->minLevel(0);
}

/**
 *
 */
int SimpleHDVizDataImpl::getMaxPersistenceLevel() const { 
  // TODO: Move into HDProcessResult as 'maxLevel'.
  return m_data->scaledPersistence.N() - 1;
}
