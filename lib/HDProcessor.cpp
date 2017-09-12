#include "HDProcessor.h"

//#include "EnableFloatingPointExceptions.h"
#include <list>
#include <set>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>

Precision MAX = std::numeric_limits<Precision>::max();

//coordinate center
int globalMin = -1;
bool bShouldWriteFiles = true;

using namespace FortranLinalg;

HDProcessor::HDProcessor() {}

/**
 * Process the input data and generate all data files necessary for visualization.
 * @param[in] x Matrix containing input sample domain.
 * @param[in] y Vector containing input function values.
 * @param[in] knn Number of nearest nieghbor for Morse-Samle complex computation.
 * @param[in] nSamples Number of samples for regression curve. 
 * @param[in] persistenceArg Number of persistence levels to compute.
 * @param[in] randArg Whether to apply random noise to input function.
 * @param[in] sigma Bandwidth for inverse regression.
 * @param[in] sigmaSmooth Bandwidth for inverse regression. (diff?)
 * @param[in] output_dir Output directory for analysis files.
 */
void HDProcessor::process(  
  FortranLinalg::DenseMatrix<Precision> x,
  FortranLinalg::DenseVector<Precision> y,  
  int knn, int nSamples, int persistenceArg, 
  bool randArg, Precision sigma, Precision sigmaSmooth, std::string output_dir) {

  if (!output_dir.empty() && *output_dir.rbegin() != '/') {
    output_dir += '/';
  }
  m_path = output_dir;
  std::cout << "Saving all output files to: " << m_path << std::endl;

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
  
  // Save geometry and function
  if (bShouldWriteFiles) {
    LinalgIO<Precision>::writeMatrix(m_path + "Geom.data", Xall);   
    LinalgIO<Precision>::writeVector(m_path + "Function.data", yall);   
  }

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
  
  if (bShouldWriteFiles) {
    LinalgIO<Precision>::writeVector(m_path + "Persistence.data", pScaled);  
  }


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
  if (bShouldWriteFiles) {
    LinalgIO<Precision>::writeVector(m_path + "PersistenceStart.data", pStart);  
  }


  // Compute inverse regression curves and additional information for each crystal
  for (unsigned int nP = start; nP < persistence.N(); nP++){
    // Number of extrema in current crystal
    // int nExt = persistence.N() - nP + 1;         // jonbronson commented out 8/16/17
    msComplex.mergePersistence(persistence(nP));
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
        crystalTmp(j, i) = exts[crystals(j, i)];
      }
    } 
    if (bShouldWriteFiles) {
      std::string crystalsFile = "Crystals_" + std::to_string(nP) + ".data";
      LinalgIO<int>::writeMatrix(m_path + crystalsFile, crystalTmp); 
    }
    crystalTmp.deallocate();   

    std::cout << std::endl << "PersistenceLevel: " << nP << std::endl;
    std::cout << "# of Crystals: " << crystals.N() << std::endl;
    std::cout << "=================================" << std::endl << std::endl;

    DenseMatrix<Precision> S(Xall.M(), crystals.N()*nSamples + nExt);
    std::vector< DenseMatrix<Precision> > ScrystalIDs(crystals.N());
    std::vector< DenseMatrix<Precision> > XcrystalIDs(crystals.N());
    std::vector< DenseMatrix<Precision> > XpcrystalIDs(crystals.N());
    std::vector< DenseMatrix<Precision> > ycrystalIDs(crystals.N());
    DenseVector<Precision> eWidths(exts.size());
    Linalg<Precision>::Zero(eWidths);

    std::vector< std::vector<unsigned int> > Xi(crystals.N());
    std::vector< std::vector<unsigned int> > Xiorig(crystals.N());
    std::vector< std::vector<Precision> > yci(crystals.N());

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


    for (unsigned int a=0; a<crystals.N(); a++) {
      for (unsigned int b=0; b<crystals.N(); b++) {
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
          for (unsigned int i=0; i < Xiorig[b].size(); i++) {
            unsigned int index = Xiorig[b][i];
            if (fabs(val - yall(index)) < 2*sigma) {
              Xi[a].push_back(index);
              yci[a].push_back(val + val - yall(index));
            }
          } 
        }
      }
    }



    // Regression for each crystal of current perssistence level.
    for (unsigned int crystalIndex = 0; crystalIndex < crystals.N(); crystalIndex++) {
      // Extract samples and function values from crystalIDs
      DenseMatrix<Precision> X(Xall.M(), Xi[crystalIndex].size());
      DenseMatrix<Precision> y(1, X.N());
      for (unsigned int i=0; i< X.N(); i++){
        unsigned int index = Xi[crystalIndex][i];
        Linalg<Precision>::SetColumn(X, i, Xall, index);
        y(0, i) = yci[crystalIndex][i];
      }
      XcrystalIDs[crystalIndex] = Linalg<Precision>::Copy(X);
      ycrystalIDs[crystalIndex] = Linalg<Precision>::Copy(y);


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
        for (int q=0; q<sdev.N(); q++) {
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

      std::string crystalPrefix = 
          "ps_" + std::to_string(nP) + "_crystal_" + std::to_string(crystalIndex);

      if (bShouldWriteFiles) {
        std::string crystalIdFilename = crystalPrefix + "_Rs.data";
        LinalgIO<Precision>::writeMatrix(m_path + crystalIdFilename, ScrystalIDs[crystalIndex]);
      }

      if (bShouldWriteFiles) {
        std::string gradsFilename = crystalPrefix + "_gradRs.data";
        LinalgIO<Precision>::writeMatrix(m_path + gradsFilename, gradS);
      }
      gradS.deallocate(); 

      if (bShouldWriteFiles) {
        std::string svarFilename = crystalPrefix + "_Svar.data";
        LinalgIO<Precision>::writeMatrix(m_path + svarFilename, Svar);
      }
      Svar.deallocate();
     
      if (bShouldWriteFiles) {
        std::string mdistFilename = crystalPrefix + "_mdists.data";
        LinalgIO<Precision>::writeVector(m_path + mdistFilename, pdist);
      }

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

      if (bShouldWriteFiles) {
        std::string fmeanFilename = crystalPrefix + "_fmean.data";
        LinalgIO<Precision>::writeVector(m_path + fmeanFilename, fmean);
      }
      fmean.deallocate();

      // Compute sample density
      DenseVector<Precision> spdf(Zp.N());
      for (unsigned int i=0; i < Zp.N(); i++) {
        Precision sum = 0;
        for (unsigned int j=0; j < y.N(); j++) {
          Precision k = kernel.f(Zp, i, y, j);
          sum += k;
        } 
        spdf(i) = sum/Xall.N();
      }

      if (bShouldWriteFiles) {
        std::string spdfFilename = crystalPrefix + "_spdf.data";
        LinalgIO<Precision>::writeVector(m_path + spdfFilename, spdf);
      }
      spdf.deallocate(); 
     

      X.deallocate();
      Zp.deallocate();
      y.deallocate();
    }
    // end of regression  loop




    // Save maximal extrema width
    if (bShouldWriteFiles) {
      std::string extremaWidthsFilename = "ExtremaWidths_" + std::to_string(nP) + ".data";
      LinalgIO<Precision>::writeVector(m_path + extremaWidthsFilename, eWidths);
    }
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
    computePCALayout(S, nExt, nSamples, nP);      

    //----- PCA extrema / PCA curves layout
    computePCAExtremaLayout(S, ScrystalIDs, nExt, nSamples, nP);

    //----- Isomap extrema / PCA curves layout 
    computeIsomapLayout(S, ScrystalIDs, nExt, nSamples, nP);     
    

    S.deallocate();
    for (unsigned int i=0; i < crystals.N(); i++) { 
      ScrystalIDs[i].deallocate();
      XcrystalIDs[i].deallocate();
      XpcrystalIDs[i].deallocate();
      ycrystalIDs[i].deallocate();
    }
  }
}

void HDProcessor::addNoise(FortranLinalg::DenseVector<Precision> &v) {
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
void HDProcessor::fit(FortranLinalg::DenseMatrix<Precision> &E, FortranLinalg::DenseMatrix<Precision> &Efit){
  using namespace FortranLinalg;
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
 *        - Lmin, Lmax
 *        - Crystal NP's Layout Matrix
 *        - Extrema Layout Matrix
 *        - Extrema Values Matrix
 * Then Move disk-writing methods outside of compute method.
 */
void HDProcessor::computePCALayout(FortranLinalg::DenseMatrix<Precision> &S, 
  int nExt, int nSamples, unsigned int nP) {
  using namespace FortranLinalg;
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
  DenseVector<Precision> Ef(nExt);
  for (map_i_i_it it = exts.begin(); it != exts.end(); ++it) {
    int eID = it->second;
    int eIndex = it->first;
    Linalg<Precision>::SetColumn(E, eID, fL, crystals.N()*nSamples+eID);
    Ef(eID) = yall(eIndex);
  } 

  // Align extrema to previous etxrema.
  if (extsOrig.size() != 0) {
    fit(E, extremaPosPCA);
  } else {
    extremaPosPCA = Linalg<Precision>::Copy(E);
    DenseVector<Precision> Lmin = Linalg<Precision>::RowMin(E);
    DenseVector<Precision> Lmax = Linalg<Precision>::RowMax(E);
    if (bShouldWriteFiles) {
      LinalgIO<Precision>::writeVector(m_path + "PCAMin.data", Lmin);
      LinalgIO<Precision>::writeVector(m_path + "PCAMax.data", Lmax);
    }
    Lmin.deallocate();
    Lmax.deallocate();
  }

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

    if (bShouldWriteFiles) {
      std::string layoutFilename = 
          "ps_" + std::to_string(nP) + "_crystal_" + std::to_string(i) + "_layout.data";
      LinalgIO<Precision>::writeMatrix(m_path + layoutFilename, tmp);
    }

    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
  }

  if (bShouldWriteFiles) {
    std::string extremaLayoutFilename = "ExtremaLayout_" + std::to_string(nP) + ".data";
    LinalgIO<Precision>::writeMatrix(m_path + extremaLayoutFilename, E);
  }
  E.deallocate();

  if (bShouldWriteFiles) {
    std::string extremaValuesFilename = "ExtremaValues_" + std::to_string(nP) + ".data";
    LinalgIO<Precision>::writeVector(m_path + extremaValuesFilename, Ef);
  }
  Ef.deallocate();

  pca.cleanup();
  fL.deallocate();      
}

/**
 * Compute low-d layout of geomtry using PCA Extrema
 * TODO:  Return layout result that include:
 *        - Lmin, Lmax
 *        - Crystal NP's Layout Matrix
 *        - Extrema Layout Matrix
 *        - Extrema Values Matrix
 * Then Move disk-writing methods outside of compute method.
 */
void HDProcessor::computePCAExtremaLayout(FortranLinalg::DenseMatrix<Precision> &S, 
  std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
  int nExt, int nSamples, unsigned int nP) {
  using namespace FortranLinalg;
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
    if (bShouldWriteFiles) {
      LinalgIO<Precision>::writeVector(m_path + "PCA2Min.data", Lmin);
      LinalgIO<Precision>::writeVector(m_path + "PCA2Max.data", Lmax);
    }
    Lmin.deallocate();
    Lmax.deallocate();
  }


  // Save layout for each crystal - stretch to extremal points.
  for (unsigned int i =0; i<crystals.N(); i++) { 
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

    if (bShouldWriteFiles) {
      std::string pca2LayoutFilename = 
          "ps_" + std::to_string(nP) + "_crystal_" + std::to_string(i) + "_pca2layout.data";
      LinalgIO<Precision>::writeMatrix(m_path + pca2LayoutFilename, tmp);
    }
    
    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
    pca.cleanup();
  }

  // Save extremal points location and function value.
  if (bShouldWriteFiles) {
    std::string pca2Filename = "PCA2ExtremaLayout_" + std::to_string(nP) + ".data";
    LinalgIO<Precision>::writeMatrix(m_path + pca2Filename, pca2L);
  }
  pca2L.deallocate();
  pca2.cleanup();         
}

/**
 * Compute low-d layout of geomtry using Isomap algorithm
 * TODO:  Return layout result that include:
 *        - Lmin, Lmax
 *        - Crystal NP's Layout Matrix
 *        - Extrema Layout Matrix
 *        - Extrema Values Matrix
 * Then Move disk-writing methods outside of compute method.
 */
void HDProcessor::computeIsomapLayout(FortranLinalg::DenseMatrix<Precision> &S, 
  std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
  int nExt, int nSamples, unsigned int nP) {
  // Do an isomap layout.
  using namespace FortranLinalg;

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
    if (bShouldWriteFiles) {
      LinalgIO<Precision>::writeVector(m_path + "IsoMin.data", Lmin);
      LinalgIO<Precision>::writeVector(m_path + "IsoMax.data", Lmax);
    }
    Lmin.deallocate();
    Lmax.deallocate();

    // Store original extream indicies.
    extsOrig = exts;
  }


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

    if (bShouldWriteFiles) {
      std::string isoLayoutFilename = 
          "ps_" + std::to_string(nP) + "_crystal_" + std::to_string(i) + "_isolayout.data";
      LinalgIO<Precision>::writeMatrix(m_path + isoLayoutFilename, tmp);
    }
    
    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
    pca.cleanup();
  }

  // Save extremal points location and function value
  if (bShouldWriteFiles) {
    std::string isoExtremaFilename = "IsoExtremaLayout_" + std::to_string(nP) + ".data";
    LinalgIO<Precision>::writeMatrix(m_path + isoExtremaFilename, isoL); 
  }
  isoL.deallocate();
}
