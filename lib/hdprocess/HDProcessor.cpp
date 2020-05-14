#include "HDProcessor.h"
#include "utils/DataExport.h"

Precision MAX = std::numeric_limits<Precision>::max();

//coordinate center
int globalMin = -1;

using namespace FortranLinalg;

HDProcessor::HDProcessor() = default;


/**
 * Process the input data and generate all data files necessary for visualization.
 * @param[in] d Distances Matrix containing pairwise distances between samples.
 * @param[in] qoi Vector containing quantity of interest values for each sample.
 * @param[in] knn Number of nearest neighbor for Morse-Samle complex computation.
 * @param[in] nSamples Number of samples for regression curve. 
 * @param[in] persistence Number of persistence levels to compute.
 * @param[in] randdom Whether to apply random noise to input function.
 * @param[in] sigma Bandwidth for inverse regression.
 * @param[in] sigmaSmooth Bandwidth for inverse regression. (diff?)
 */
HDProcessResult* HDProcessor::processOnMetric(
    DenseMatrix<Precision> d, DenseVector<Precision> qoi,
    int knn, int nSamples, int persistenceArg, bool random,
    Precision sigmaArg, Precision sigmaSmooth) {
  // TODO: Assert(qoi.N() == d.M() && d.M() == d.N())

  // Initialize processing result output object.
  m_result = new HDProcessResult();

  // Embed Distance Metric into 3D space
  EuclideanMetric<Precision> metric;
  MetricMDS<Precision> mds;
  // Store input data as member variables.
  std::cout << "knn = " << knn << std::endl;
  // make copy of distances so embedder won't trash data.
  auto dd = Linalg<Precision>::Copy(d);
  Xall = mds.embed(dd, 3); // TODO why 3?
  yall = qoi;
  
  // Add noise to yall in case of equivalent values 
  if (random) {
    addNoise(yall);
  }
     
  // Compute Morse-Smale complex    
  NNMSComplex<Precision> msComplex(d, qoi, knn, sigmaSmooth > 0, sigmaSmooth*sigmaSmooth, true);
  
  // Store persistence levels
  persistence = msComplex.getPersistence();

  // Store Nearest Neighbors
  m_result->knn = msComplex.getNearestNeighbors();
  
  
  // Save QoI function values  
  m_result->X = Linalg<Precision>::Copy(Xall);
  m_result->Y = Linalg<Precision>::Copy(yall);  

  // Scale persistence to be in [0,1]
  DenseVector<Precision> pScaled(persistence.N());
  Precision fmax = Linalg<Precision>::Max(yall);
  Precision fmin = Linalg<Precision>::Min(yall);
  Precision frange = fmax - fmin;
  for (unsigned int i=0; i < persistence.N(); i++) {
    pScaled(i) = persistence(i) / frange;    // don't subtract fmin here since it comes from field values, not persistence
  }
  pScaled(pScaled.N()-1) = 1;                // set to 1 because the max value returned by the mscomplex is huge
  
  // Store Scled Persistence Data
  m_result->scaledPersistence = Linalg<Precision>::Copy(pScaled);

  // Read number of persistence levels to compute visualization for
  int nlevels = persistenceArg;
  int start = 0;
  if (nlevels > 0) {
    start = persistence.N() - nlevels;
  }
  if (start < 0) {
    start = 0;
  }

  // Save start persistence
  DenseVector<Precision> pStart(1);
  pStart(0) = start;

  // Save number of requested regression samples
  DenseVector<int> regressionSampleCount(1);
  regressionSampleCount(0) = nSamples;
  m_result->regressionSampleCount = Linalg<int>::Copy(regressionSampleCount);

  // Store Min Level (Starting Level)
  m_result->minLevel = Linalg<Precision>::Copy(pStart);

  // Resize Stores for Saving Persistence Level information
  m_result->crystals.resize(persistence.N());
  m_result->crystalPartitions.resize(persistence.N());

  // Regression Stores
  m_result->extremaValues.resize(persistence.N());  
  m_result->extremaWidths.resize(persistence.N());
  m_result->R.resize(persistence.N());
  m_result->gradR.resize(persistence.N());
  m_result->Rvar.resize(persistence.N());
  m_result->mdists.resize(persistence.N());
  m_result->fmean.resize(persistence.N());
  m_result->spdf.resize(persistence.N());

  // Layout Stores
  m_result->PCAExtremaLayout.resize(persistence.N());  
  m_result->PCALayout.resize(persistence.N());
  m_result->PCA2ExtremaLayout.resize(persistence.N());
  m_result->PCA2Layout.resize(persistence.N());
  m_result->IsoExtremaLayout.resize(persistence.N());
  m_result->IsoLayout.resize(persistence.N());

  
  // Compute inverse regression curves and additional information for each crystal
  for (unsigned int persistenceLevel = start; persistenceLevel < persistence.N(); persistenceLevel++){
    computeAnalysisForLevel(msComplex, persistenceLevel, nSamples, sigmaArg, true /*computeRegression*/);
  }

  // Export crystal partitions for shapeodds
  {
    bool exportCrystalPartitions = false;  // TODO: add these as a parameters to the function
    std::string partitionsName("crystalpartitions.csv");
    if (exportCrystalPartitions)
      DataExport::exportCrystalPartitions(m_result->crystalPartitions, start, partitionsName);
  }
  
  // detach and return processed result
  HDProcessResult *result = m_result;
  m_result = nullptr;
  return result;
}

#if 0 //<ctc> this function seems identical to above ::processOnMetric, and both have bugs, so just commenting it out for now, purposely not fixing anything herein.  // NOTE: we think the function above is for distance matrices, and this one is for QoIs and Design Params
/**
 * Process the input data and generate all data files necessary for visualization.
 * @param[in] x Matrix containing input sample domain.
 * @param[in] y Vector containing input function values.
 * @param[in] knn Number of nearest neighbor for Morse-Samle complex computation.
 * @param[in] nSamples Number of samples for regression curve. 
 * @param[in] persistenceArg Number of persistence levels to compute.
 * @param[in] randArg Whether to apply random noise to input function.
 * @param[in] sigma Bandwidth for inverse regression.
 * @param[in] sigmaSmooth Bandwidth for inverse regression. (diff?)
 */
HDProcessResult* HDProcessor::process(  
  DenseMatrix<Precision> x, DenseVector<Precision> y,  
  int knn, int nSamples, int persistenceArg, 
  bool randArg, Precision sigma, Precision sigmaSmooth) {

  // Initialize processing result output object.
  m_result = new HDProcessResult();

  // Store input data as member variables.
  Xall = x;
  yall = y;
  
  // Add noise to yall in case of equivalent values 
  if (randArg) {
    addNoise(yall);
  }
     
  // Compute Morse-Smale complex
  NNMSComplex<Precision> msComplex(Xall, yall, knn, sigmaSmooth > 0, 0.01, sigmaSmooth*sigmaSmooth);
  
  // Store persistence levels
  persistence = msComplex.getPersistence();

  // Store Nearest Neighbors
  m_result->knn = msComplex.getNearestNeighbors();

  
  // Save geometry and function
  m_result->X = Linalg<Precision>::Copy(Xall);
  m_result->Y = Linalg<Precision>::Copy(yall);  

  // Scale persistence to be in [0,1]
  // TODO: Is this just doing a normalization?
  DenseVector<Precision> pScaled(persistence.N());
  Precision fmax = Linalg<Precision>::Max(yall);
  Precision fmin = Linalg<Precision>::Min(yall);
  Precision frange = fmax - fmin;
  for (unsigned int i=0; i < persistence.N(); i++) {
    pScaled(i) = persistence(i) / frange;    // TODO: Shouldn't this also subtract fmin?
  }
  pScaled(pScaled.N()-1) = 1;                // TODO: Determine why is this set to 1?
  
  // Store Scled Persistence Data
  m_result->scaledPersistence = Linalg<Precision>::Copy(pScaled);
  

  // Read number of persistence levels to compute visualization for
  int nlevels = persistenceArg;
  int start = 0;
  if (nlevels > 0) {
    start = persistence.N() - nlevels;
  }
  if (start < 0) {
    start = 0;
  }

  // Save start persistence
  DenseVector<Precision> pStart(1);
  pStart(0) = start;

  // Save number of requested regression samples
  DenseVector<int> regressionSampleCount(1);
  regressionSampleCount(1) = nSamples;
  m_result->regressionSampleCount = Linalg<int>::Copy(regressionSampleCount);

  // Store Min Level (Starting Level)
  m_result->minLevel = Linalg<Precision>::Copy(pStart);

  // Resize Stores for Saving Persistence Level information
  m_result->crystals.resize(persistence.N());
  m_result->crystalPartitions.resize(persistence.N());

  // Regression Stores
  m_result->extremaValues.resize(persistence.N());  
  m_result->extremaWidths.resize(persistence.N());
  m_result->R.resize(persistence.N());
  m_result->gradR.resize(persistence.N());
  m_result->Rvar.resize(persistence.N());
  m_result->mdists.resize(persistence.N());
  m_result->fmean.resize(persistence.N());
  m_result->spdf.resize(persistence.N());

  // Layout Stores
  m_result->PCAExtremaLayout.resize(persistence.N());  
  m_result->PCALayout.resize(persistence.N());
  m_result->PCA2ExtremaLayout.resize(persistence.N());
  m_result->PCA2Layout.resize(persistence.N());
  m_result->IsoExtremaLayout.resize(persistence.N());
  m_result->IsoLayout.resize(persistence.N());

  
  // Compute inverse regression curves and additional information for each crystal
  for (unsigned int persistenceLevel = start; persistenceLevel < persistence.N(); persistenceLevel++){
    computeAnalysisForLevel(msComplex, persistenceLevel, nSamples, sigma, true /*computeRegression*/);
  }

  // detach and return processed result
  HDProcessResult *result = m_result;
  m_result = nullptr;
  return result;
}
#endif

/**
 * Compute analysis for a single persistence level.
 * @param[in] msComplex A computed Morse-Smale complex.
 * @param[in] persistenceLevel The persistence level to regress.
 * @param[in] nSamples Number of samples for regression curve.  
 * @param[in] sigma Bandwidth for inverse regression.
 */
void HDProcessor::computeAnalysisForLevel(NNMSComplex<Precision> &msComplex,
    unsigned int persistenceLevel, int nSamples, Precision sigma, bool computeRegression) {
  // Number of extrema in current crystal
  // int nExt = persistence.N() - persistenceLevel + 1;      // jonbronson commented out 8/16/17
  msComplex.mergePersistence(persistence(persistenceLevel));
  crystalIDs.deallocate();
  crystalIDs = msComplex.getPartitions();
  crystals.deallocate();
  crystals = msComplex.getCrystals();

  // Find global minimum as refernce point for aligning subsequent persistence levels
  if (globalMin == -1) {
    double tmp = std::numeric_limits<Precision>::max();
    for (unsigned int i=0; i < crystals.N(); i++) {
      if (tmp > yall(crystals(1, i))) {
        globalMin = crystals(1, i);
        tmp = yall(globalMin);
      }
    }
  }      

  // Compute map of extrema to extremaID
  exts.clear();
  int eID = 0;
  for (int e=0; e<2; e++){
    for (unsigned int i=0; i<crystals.N(); i++){
      int extrema = crystals(e, i);
      // Check if this extrema is already in the list
      map_i_i_it it = exts.find(extrema);
      if (it == exts.end()){         
        exts[extrema] = eID;
        ++eID;
      }  
    }
  }

  int nExt = exts.size(); // jonbronson added 8/16/17

  // Save crystals
  DenseMatrix<int> crystalTmp(crystals.M(), crystals.N());
  for (unsigned int i=0; i < crystalTmp.N(); i++) {
    for (unsigned int j=0; j < crystalTmp.M(); j++) {
      crystalTmp(j, i) = exts[crystals(j, i)];         // TODO: What is this transformation? 11/9/17
    }
  } 

  // Store Crystals in Result
  m_result->crystals[persistenceLevel] = Linalg<int>::Copy(crystalTmp);
  // Store Crystal Partitions in Result (which samples belong to which crystal)
  m_result->crystalPartitions[persistenceLevel] = Linalg<int>::Copy(crystalIDs);


  crystalTmp.deallocate();   

  // Grab and Store Extrema Function Values 
  DenseVector<Precision> Ef(nExt);
  for (map_i_i_it it = exts.begin(); it != exts.end(); ++it) {
    int eID = it->second;
    int eIndex = it->first;    
    Ef(eID) = yall(eIndex);
  }
  m_result->extremaValues[persistenceLevel] = Linalg<Precision>::Copy(Ef);  
  Ef.deallocate();


  std::cout << std::endl << "PersistenceLevel: " << persistenceLevel << std::endl;
  std::cout << "# of Crystals: " << crystals.N() << std::endl;
  std::cout << "=================================" << std::endl << std::endl;
  
  if (!computeRegression) {
    // Create and return fake data for now.
    // Resize Stores for Regression Information
    m_result->R[persistenceLevel].resize(crystals.N());
    m_result->gradR[persistenceLevel].resize(crystals.N());
    m_result->Rvar[persistenceLevel].resize(crystals.N());
    m_result->mdists[persistenceLevel].resize(crystals.N());  
    m_result->fmean[persistenceLevel].resize(crystals.N());  
    m_result->spdf[persistenceLevel].resize(crystals.N());  

    // Resize Stores with Layout Information
    m_result->IsoLayout[persistenceLevel].resize(crystals.N());
    m_result->PCALayout[persistenceLevel].resize(crystals.N());
    m_result->PCA2Layout[persistenceLevel].resize(crystals.N());
        
    // m_result->extremaWidths[persistenceLevel]
    DenseVector<Precision> fakeVector(nExt);
    DenseMatrix<Precision> fakeLayoutMatrix(2, nSamples);    
    m_result->extremaWidths[persistenceLevel] = Linalg<Precision>::Copy(fakeVector);  

    for (unsigned int crystalIndex = 0; crystalIndex < crystals.N(); crystalIndex++) {
      m_result->IsoLayout[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(fakeLayoutMatrix);
      m_result->PCALayout[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(fakeLayoutMatrix);
      m_result->PCA2Layout[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(fakeLayoutMatrix);
    }

    DenseVector<Precision> fakeSpdf(nSamples);
    DenseMatrix<Precision> fakeExtremaMatrix(2, nExt);
    m_result->IsoExtremaLayout[persistenceLevel] = Linalg<Precision>::Copy(fakeExtremaMatrix);

    // Create fake regression info for each crystal of current persistence level.
    for (unsigned int crystalIndex = 0; crystalIndex < crystals.N(); crystalIndex++) {
    //   // Store Regression Info in Results
    //   m_result->R[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(ScrystalIDs[crystalIndex]);
    //   m_result->gradR[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(gradS);
    //   m_result->Rvar[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(Svar);
    //   m_result->mdists[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(pdist);
      m_result->fmean[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(fakeVector);
      m_result->spdf[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(fakeSpdf);
    }

    return;
  }

  // ------------------------------------------------------------
  // Only Proceed Below if Regression can be ran over input.
  // ------------------------------------------------------------
  std::cout << "Before Regression: crystals.N() = " << crystals.N() << std::endl;

  DenseMatrix<Precision> S(Xall.M(), crystals.N()*nSamples + nExt);
  std::vector<DenseMatrix<Precision>> ScrystalIDs(crystals.N());  
  std::vector<DenseMatrix<Precision>> XpcrystalIDs(crystals.N());  
  DenseVector<Precision> eWidths(exts.size());
  Linalg<Precision>::Zero(eWidths);

  std::vector<std::vector<unsigned int>> Xi(crystals.N());
  std::vector<std::vector<unsigned int>> Xiorig(crystals.N());
  std::vector<std::vector<Precision>> yci(crystals.N());

  // Compute regression for each Morse-Smale crystal
  for (unsigned int crystalIndex = 0; crystalIndex < crystals.N(); ++crystalIndex) {
    for (unsigned int i=0; i< crystalIDs.N(); i++) {
      if (crystalIDs(i) == (int) crystalIndex) {
        Xiorig[crystalIndex].push_back(i);
        Xi[crystalIndex].push_back(i);
        yci[crystalIndex].push_back(yall(i));
      }
    }
  }


  for (unsigned int a = 0; a < crystals.N(); a++) {
    for (unsigned int b = 0; b < crystals.N(); b++) {
      if (a == b) continue;
      int ea1 = crystals(0, a);
      int ea2 = crystals(1, a);
      int eb1 = crystals(0, b);
      int eb2 = crystals(1, b);
      bool touch = false;
      Precision val = 0;
      if (ea1 == eb1 ) {
        val = yall(ea1);
        touch = true;
      }
      if (ea2 == eb2) {
        val = yall(ea2);
        touch = true;
      }
      // Add points within sigma of extrema to this points.
      if (touch) {
        for (unsigned int i = 0; i < Xiorig[b].size(); i++) {
          unsigned int index = Xiorig[b][i];
          if (fabs(val - yall(index)) < 2*sigma) {
            Xi[a].push_back(index);
            yci[a].push_back(val + val - yall(index));
          }
        } 
      }
    }
  }

  // Resize Stores for Regression Information
  m_result->R[persistenceLevel].resize(crystals.N());
  m_result->gradR[persistenceLevel].resize(crystals.N());
  m_result->Rvar[persistenceLevel].resize(crystals.N());
  m_result->mdists[persistenceLevel].resize(crystals.N());  
  m_result->fmean[persistenceLevel].resize(crystals.N());  
  m_result->spdf[persistenceLevel].resize(crystals.N());  

  // Regression for each crystal of current persistence level.
  for (unsigned int crystalIndex = 0; crystalIndex < crystals.N(); crystalIndex++) {
    computeRegressionForCrystal(crystalIndex, persistenceLevel, sigma, nSamples, Xi, yci, ScrystalIDs, S, eWidths);
  }

  // Store Maximal ExtremaWidths in Result
  m_result->extremaWidths[persistenceLevel] = Linalg<Precision>::Copy(eWidths);

  eWidths.deallocate();

  // Add extremal points to S for computing layout
  int count = 0;
  for (map_i_i_it it = exts.begin(); it != exts.end(); ++it) { 
    count++;
    // std::cout << "Adding extremal point #" << count << std::endl;
    // Average the end points of all curves with that extremea
    DenseVector<Precision> out(Xall.M());
    Linalg<Precision>::Zero(out);
    int n = 0;
    for (int e = 0; e<2; e++) {
      for (unsigned int k=0; k < crystals.N(); k++) {
        if (crystals(e, k) == it->first){
          if (e == 0 ) {
            Linalg<Precision>::Add(out, ScrystalIDs[k], nSamples-1, out); 
          }
          else {
            Linalg<Precision>::Add(out, ScrystalIDs[k], 0, out);
          }
          n++;
        }
      }
    }
    // Add averaged extrema to S
    Linalg<Precision>::Scale(out, 1.f/n, out);
    Linalg<Precision>::SetColumn(S, nSamples*crystals.N()+it->second, out);
    out.deallocate();
  }  

  //----- Complete PCA layout 
  computePCALayout(S, nExt, nSamples, persistenceLevel);      

  //----- PCA extrema / PCA curves layout
  computePCAExtremaLayout(S, ScrystalIDs, nExt, nSamples, persistenceLevel);

  //----- Isomap extrema / PCA curves layout 
  computeIsomapLayout(S, ScrystalIDs, nExt, nSamples, persistenceLevel);     


  S.deallocate();
  for (unsigned int i=0; i < crystals.N(); i++) { 
    ScrystalIDs[i].deallocate();
    XpcrystalIDs[i].deallocate();    
  }
}

/**
 * Computes regression curves for each crystal of specified persistence level.
 * TODO(jonbronson):  We will need an abstraction for computing regression that
 *                    doesn't rely on using the X matrix of samples.
 */
void HDProcessor::computeRegressionForCrystal(
    unsigned int crystalIndex, unsigned int persistenceLevel, Precision sigma, int nSamples, 
    std::vector<std::vector<unsigned int>> &Xi, std::vector<std::vector<Precision>> &yci,
    std::vector<DenseMatrix<Precision>> &ScrystalIDs, DenseMatrix<Precision> &S,
    DenseVector<Precision> &eWidths) {
  // Extract samples and function values from crystalIDs
  DenseMatrix<Precision> X(Xall.M(), Xi[crystalIndex].size());
  DenseMatrix<Precision> y(1, X.N());
  std::vector<unsigned int> indexes;
  for (unsigned int i=0; i< X.N(); i++){
    unsigned int index = Xi[crystalIndex][i];
    indexes.push_back(index);
    Linalg<Precision>::SetColumn(X, i, Xall, index);  
    y(0, i) = yci[crystalIndex][i];
  }

  
  // Compute Rgeression curve
  std::cout << "Computing regression curve for crystalID " << crystalIndex << std::endl;
  std::cout << X.N() << " points" << std::endl;

  GaussianKernel<Precision> kernel(sigma, 1);
  FirstOrderKernelRegression<Precision> kr(X, y, kernel, 1000);
     
  /*      
    //Get locations
    DenseMatrix<Precision> Zend = y;
    DenseVector<Precision> yv(1);
    DenseVector<Precision> tmp(Xall.M());
    DenseMatrix<Precision> Xp(X.M(), X.N());
    for (unsigned int k=0; k<X.N(); k++) {
       yv(0) = y(0, k);
       kr.evaluate(yv, tmp);
       Linalg<Precision>::SetColumn(Xp, k, tmp);
    }
    XpcrystalIDs[crystalIndex] = Xp;
  */

  // Compute min and max function value
  int e1 = crystals(0, crystalIndex);
  int e2 = crystals(1, crystalIndex);
  int e1ID = exts[e1];
  int e2ID = exts[e2];
  Precision zmax = yall(e1);
  Precision zmin = yall(e2);

  // Create samples (regressed in input space) between min and max function values
  DenseVector<Precision> z(1);
  DenseVector<Precision> pdist(nSamples);
  DenseVector<Precision> tmp(Xall.M());
  DenseMatrix<Precision> Zp(1, nSamples);
  ScrystalIDs[crystalIndex] = DenseMatrix<Precision>(Xall.M(), nSamples);
  DenseMatrix<Precision> gStmp(Xall.M(), 1);
  DenseMatrix<Precision> gradS(Xall.M(), nSamples);
  DenseVector<Precision> sdev( Xall.M() );
  DenseMatrix<Precision> Svar(Xall.M(), nSamples);
  for (int k=0; k < nSamples; k++) {
    z(0) = zmin + (zmax-zmin) * ( k/ (nSamples-1.f) );
    Zp(0, k) = z(0);
    kr.evaluate(z, tmp, gStmp, sdev.data());
    pdist(k) = 0;
    for (unsigned int q = 0; q < sdev.N(); q++) {
      pdist(k) += sdev(q);
      sdev(q) = sqrt(sdev(q));
    }
    pdist(k) = sqrt(pdist(k));
    
    Linalg<Precision>::SetColumn(S, crystalIndex*nSamples + k, tmp);
    Linalg<Precision>::SetColumn(ScrystalIDs[crystalIndex], k, tmp);
    Linalg<Precision>::SetColumn(Svar, k, sdev);
    Linalg<Precision>::SetColumn(gradS, k, gStmp, 0);
  }
  sdev.deallocate();
  z.deallocate();
  tmp.deallocate();
  
  // Store Regression Info in Results
  m_result->R[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(ScrystalIDs[crystalIndex]);
  m_result->gradR[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(gradS);
  m_result->Rvar[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(Svar);
  m_result->mdists[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(pdist);

  gradS.deallocate(); 
  Svar.deallocate();

  // Compute maximal extrema widths
  if (eWidths(e2ID) < pdist(0)) {
    eWidths(e2ID) = pdist(0); 
  }
  if (eWidths(e1ID) < pdist(nSamples-1)) {
    eWidths(e1ID) = pdist(nSamples-1); 
  }

  pdist.deallocate();
  
  // Compute function value mean at sampled locations
  DenseVector<Precision> fmean(Zp.N());
  for (unsigned int i=0; i < Zp.N(); i++) {
    fmean(i) = Zp(0, i);
  }

  // Store means in result object.
  m_result->fmean[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(fmean);
  fmean.deallocate();

  // Compute sample density.
  DenseVector<Precision> spdf(Zp.N());
  for (unsigned int i=0; i < Zp.N(); i++) {
    Precision sum = 0;
    for (unsigned int j=0; j < y.N(); j++) {
      Precision k = kernel.f(Zp, i, y, j);
      sum += k;
    } 
    spdf(i) = sum/Xall.N();
  }

  // Store sample density in result object.
  m_result->spdf[persistenceLevel][crystalIndex] = Linalg<Precision>::Copy(spdf);  
  spdf.deallocate(); 
 

  X.deallocate();
  Zp.deallocate();
  y.deallocate();
}

/**
 * Add small pertubations to data achieve general position / avoid pathological cases.
 */
void HDProcessor::addNoise(DenseVector<Precision> &v) {
  std::cerr << "Adding noise to M-S field...\n";
  Random<Precision> rand;
  double a = 0.00000001 *( Linalg<Precision>::Max(v) - Linalg<Precision>::Min(v));
  for (unsigned int i=0; i < v.N(); i++) {
    v(i) += rand.Uniform() * a;
  }
}

/**
 * Linearly transform E to fit align with Efit
 * Used to align extrema of subsequent persistence levels
 */
void HDProcessor::fit(DenseMatrix<Precision> &E, DenseMatrix<Precision> &Efit) {
  DenseMatrix<Precision> Eorig(exts.size(), 2);
  DenseMatrix<Precision> Enew(exts.size(), 2);

  int e1 = exts[globalMin];
  int e2 = extsOrig[globalMin];

  for( map_i_i_it it = exts.begin(); it != exts.end(); ++it){
    int i1 = it->second;
    int i2 = extsOrig[it->first];
    for(unsigned int j=0; j<2; j++){
      Eorig(it->second, j) = Efit(j, i2) - Efit(j, e2);
      Enew(it->second, j) = E(j, i1) - E(j, e1);
    }  
  }
  DenseMatrix<Precision> Etmp = Linalg<Precision>::Copy(Enew);
  DenseMatrix<Precision> T = Linalg<Precision>::LeastSquares(Etmp, Eorig);
  DenseMatrix<Precision> ET = Linalg<Precision>::Multiply(Enew, T);
  for(unsigned int i=0; i< E.N(); i++){
    for(unsigned int j=0; j<2; j++){
      E(j, i) = ET(i, j) + Efit(j, e2);
    }   
  }
  T.deallocate();
  Eorig.deallocate();
  Enew.deallocate();
  Etmp.deallocate();
  ET.deallocate();
}

/**
 * Compute low-d layout of geomtry using PCA.
 * TODO:  Return layout result that include:
 *        - Crystal's Layout Matrix
 *        - Extrema Layout Matrix
 *        - Extrema Values Matrix
 */
void HDProcessor::computePCALayout(DenseMatrix<Precision> &S, 
  int nExt, int nSamples, unsigned int persistenceLevel) {
  unsigned int dim = 2; 
  PCA<Precision> pca(S, dim);        
  DenseMatrix<Precision> fL = pca.project(S);
  if (fL.M() < dim) {
    DenseMatrix<Precision> fLtmp(dim, fL.N());
    Linalg<Precision>::Zero(fLtmp);
    for (unsigned int i=0; i < fL.M(); i++) {
      Linalg<Precision>::SetRow(fLtmp, i, fL, i);
    }
    fL.deallocate();
    fL = fLtmp;
  }

  // Save extremal points location and function value.
  DenseMatrix<Precision> E(fL.M(), nExt); 
  for (map_i_i_it it = exts.begin(); it != exts.end(); ++it) {
    int eID = it->second;
    Linalg<Precision>::SetColumn(E, eID, fL, crystals.N()*nSamples+eID);
  } 

  // Align extrema to previous etxrema.
  if (extsOrig.size() != 0) {
    fit(E, extremaPosPCA);
  } else {
    extremaPosPCA = Linalg<Precision>::Copy(E);
    DenseVector<Precision> Lmin = Linalg<Precision>::RowMin(E);
    DenseVector<Precision> Lmax = Linalg<Precision>::RowMax(E);
    
    // Copy to results object.
    m_result->LminPCA = Linalg<Precision>::Copy(Lmin);
    m_result->LmaxPCA = Linalg<Precision>::Copy(Lmax);
    Lmin.deallocate();
    Lmax.deallocate();
  }

  // Resize Layout in Results Object
  m_result->PCALayout[persistenceLevel].resize(crystals.N());

  // Save layout for each crystalIDs - stretch to extremal points.
  for (unsigned int i =0; i < crystals.N(); i++) {
    DenseMatrix<Precision> tmp(fL.M(), nSamples);
    DenseVector<Precision> a(fL.M());
    DenseVector<Precision> b(fL.M());
    Linalg<Precision>::ExtractColumn(E, exts[crystals(1, i)], a );
    Linalg<Precision>::ExtractColumn(E, exts[crystals(0, i)], b );

    for (unsigned int j=0; j < tmp.N(); j++){
      Linalg<Precision>::SetColumn(tmp, j, fL, nSamples*i+j);
    }

    Linalg<Precision>::Subtract(a, tmp, 0, a);
    Linalg<Precision>::AddColumnwise(tmp, a, tmp);
    Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

    DenseVector<Precision> stretch(fL.M());
    for (unsigned int j=0; j < tmp.N(); j++){
      Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
      Linalg<Precision>::Add(tmp, j, stretch);
    }

    // Store layout information in result object.
    m_result->PCALayout[persistenceLevel][i] = Linalg<Precision>::Copy(tmp);    

    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
  }

  // Store ExtremaLayout in Result Object
  m_result->PCAExtremaLayout[persistenceLevel] = Linalg<Precision>::Copy(E);
  E.deallocate();

  pca.cleanup();
  fL.deallocate();      
}

/**
 * Compute low-d layout of geomtry using PCA Extrema
 * TODO:  Return layout result that include:
 *        - Lmin, Lmax
 *        - Crystal's Layout Matrix
 *        - Extrema Layout Matrix
 *        - Extrema Values Matrix
 * Then Move disk-writing methods outside of compute method.
 */
void HDProcessor::computePCAExtremaLayout(DenseMatrix<Precision> &S, 
    std::vector<DenseMatrix<Precision>> &ScrystalIDs, 
    int nExt, int nSamples, unsigned int persistenceLevel) {
  unsigned int dim = 2; 
  DenseMatrix<Precision> Xext(Xall.M(), nExt);
  for (int i=0; i < nExt; i++) {
    Linalg<Precision>::SetColumn(Xext, i, S, nSamples*crystals.N()+i);
  }
  int ndim = 2;
  if (Xext.N() == 2) {
    ndim = 1;
  }
  PCA<Precision> pca2(Xext, ndim);
  DenseMatrix<Precision> pca2L = pca2.project(Xext);
  if (ndim == 1) {
    DenseMatrix<Precision> tmp(2, 2);
    tmp(0, 0) = pca2L(0, 0);
    tmp(0, 1) = pca2L(0, 1);
    tmp(1, 0) = 0;
    tmp(1, 1) = 0;
    pca2L.deallocate();
    pca2L = tmp;
  }

  // Align extrema to previous etxrema.
  if (extsOrig.size() != 0) {
    fit(pca2L, extremaPosPCA2);
  } else {
    extremaPosPCA2 = Linalg<Precision>::Copy(pca2L);       
    DenseVector<Precision> Lmin = Linalg<Precision>::RowMin(pca2L);
    DenseVector<Precision> Lmax = Linalg<Precision>::RowMax(pca2L);

    // Copy to results object.
    m_result->LminPCA2 = Linalg<Precision>::Copy(Lmin);
    m_result->LmaxPCA2 = Linalg<Precision>::Copy(Lmax);    
    Lmin.deallocate();
    Lmax.deallocate();
  }

  // Resize Layout in Results Object
  m_result->PCA2Layout[persistenceLevel].resize(crystals.N());

  // Save layout for each crystal - stretch to extremal points.
  for (unsigned int i = 0; i < crystals.N(); i++) { 
    // Do pca for each crystal to preserve strcuture of curve in crystal.
    PCA<Precision> pca(ScrystalIDs[i], dim);        
    DenseMatrix<Precision> tmp = pca.project(ScrystalIDs[i]);
    DenseVector<Precision> a(pca2L.M());
    DenseVector<Precision> b(pca2L.M());
    Linalg<Precision>::ExtractColumn(pca2L, exts[crystals(1, i)], a );
    Linalg<Precision>::ExtractColumn(pca2L, exts[crystals(0, i)], b );

    Linalg<Precision>::Subtract(a, tmp, 0, a);
    Linalg<Precision>::AddColumnwise(tmp, a, tmp);
    Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

    DenseVector<Precision> stretch(pca2L.M());
    for (unsigned int j=0; j<tmp.N(); j++) {
      Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
      Linalg<Precision>::Add(tmp, j, stretch);
    }

    // Store layout information in result object.
    m_result->PCA2Layout[persistenceLevel][i] = Linalg<Precision>::Copy(tmp);    
    
    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
    pca.cleanup();
  }

  // Store ExtremaLayout in Result Object
  m_result->PCA2ExtremaLayout[persistenceLevel] = Linalg<Precision>::Copy(pca2L);  
  pca2L.deallocate();
  pca2.cleanup();         
}

/**
 * Compute low-d layout of geomtry using Isomap algorithm
 * TODO:  Return layout result that include:
 *        - Lmin, Lmax
 *        - Crystal's Layout Matrix
 *        - Extrema Layout Matrix
 *        - Extrema Values Matrix
 * Then Move disk-writing methods outside of compute method.
 */
void HDProcessor::computeIsomapLayout(DenseMatrix<Precision> &S, 
    std::vector<DenseMatrix<Precision>> &ScrystalIDs, 
    int nExt, int nSamples, unsigned int persistenceLevel) {
  // Do an isomap layout.
  EuclideanMetric<Precision> metric;
  unsigned int dim = 2; 
  SparseMatrix<Precision> adj(nExt, nExt, std::numeric_limits<Precision>::max());
  for (unsigned int i=0; i < crystals.N(); i++) {
    Precision dist = 0; 
    for (int j=1; j < nSamples; j++) {
      int index1 = nSamples*i+j;
      int index2 = index1 - 1;
      dist += metric.distance(S, index1, S, index2);
    }       

    int index1 = exts[crystals(0, i)];
    int index2 = exts[crystals(1, i)];
    adj.set(index1, index2, dist);
    adj.set(index2, index1, dist);
  }


  KNNNeighborhood<Precision> nh(10);
  Isomap<Precision> isomap(&nh, dim);
  DenseMatrix<Precision> isoL = isomap.embedAdj(adj);


  // Align extrema to previous etxrema
  if (extsOrig.size() != 0) {
    fit(isoL, extremaPosIso);
  } else {
    extremaPosIso = Linalg<Precision>::Copy(isoL);                
    DenseVector<Precision> Lmin = Linalg<Precision>::RowMin(isoL);
    DenseVector<Precision> Lmax = Linalg<Precision>::RowMax(isoL);

    // Copy to results object.
    m_result->LminIso = Linalg<Precision>::Copy(Lmin);
    m_result->LmaxIso = Linalg<Precision>::Copy(Lmax);
    Lmin.deallocate();
    Lmax.deallocate();

    // Store original extream indicies.
    extsOrig = exts;
  }

  // Resize Layout in Results Object
  m_result->IsoLayout[persistenceLevel].resize(crystals.N());

  // Save layout for each crystal - stretch to extremal points.
  for (unsigned int i =0; i < crystals.N(); i++) { 
    // Do pca for each crystal to preserve strcuture of curve in crystal.
    PCA<Precision> pca(ScrystalIDs[i], dim);        
    DenseMatrix<Precision> tmp = pca.project(ScrystalIDs[i]);
    DenseVector<Precision> a(isoL.M());
    DenseVector<Precision> b(isoL.M());
    Linalg<Precision>::ExtractColumn(isoL, exts[crystals(1, i)], a );
    Linalg<Precision>::ExtractColumn(isoL, exts[crystals(0, i)], b );
    Linalg<Precision>::Subtract(a, tmp, 0, a);
    Linalg<Precision>::AddColumnwise(tmp, a, tmp);
    Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

    DenseVector<Precision> stretch(isoL.M());
    for (unsigned int j=0; j < tmp.N(); j++){
      Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
      Linalg<Precision>::Add(tmp, j, stretch);
    }

    // Store layout information in result object.
    m_result->IsoLayout[persistenceLevel][i] = Linalg<Precision>::Copy(tmp);
    
    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
    pca.cleanup();
  }

  // Store ExtremaLayout in Result Object
  m_result->IsoExtremaLayout[persistenceLevel] = Linalg<Precision>::Copy(isoL);
  isoL.deallocate();
}
