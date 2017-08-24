#include "Precision.h"

#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "PCA.h"
//#include "EnableFloatingPointExceptions.h"
#include <tclap/CmdLine.h>
#include "KNNNeighborhood.h"
#include "Isomap.h"
#include "FirstOrderKernelRegression.h"
#include "NNMSComplex.h"
#include "Random.h"

#include <list>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <map>


Precision MAX = std::numeric_limits<Precision>::max();

// Position of extrema for different projection methods
// PCA of all points inluding regresion curves
FortranLinalg::DenseMatrix<Precision> extremaPosPCA;

// PCA of extrema only
FortranLinalg::DenseMatrix<Precision> extremaPosPCA2;

// Isomap of regression cruve graphs
FortranLinalg::DenseMatrix<Precision> extremaPosIso;

// List of extrema and extrema of previous persistence level used to align low
// dimensional mappings to previous level
typedef std::map<int, int> map_i_i;
typedef map_i_i::iterator map_i_i_it; 
map_i_i exts;
map_i_i extsOrig;

//coordinate center
int globalMin = -1;


FortranLinalg::DenseVector<int> crystalIDs;
FortranLinalg::DenseMatrix<int> crystals;
FortranLinalg::DenseVector<Precision> persistence;
FortranLinalg::DenseMatrix<Precision> Xall;
FortranLinalg::DenseVector<Precision> yall;


void computePCALayout(FortranLinalg::DenseMatrix<Precision> &S, 
  int nExt, int nSamples, unsigned int nP);
void computePCAExtremaLayout(FortranLinalg::DenseMatrix<Precision> &S, 
  std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
  int nExt, int nSamples, unsigned int nP);
void computeIsomapLayout(FortranLinalg::DenseMatrix<Precision> &S, 
  std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
  EuclideanMetric<Precision> &l2, int nExt, int nSamples, unsigned int nP);


/**
 * Linearly transform E to fit aligin with Efit
 * Used to align extrema of subsequent persistence levels
 */
void fit(FortranLinalg::DenseMatrix<Precision> &E, FortranLinalg::DenseMatrix<Precision> &Efit){
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
 * HDVisProcess application entry point.
 */
int main(int argc, char **argv){
  
  using namespace FortranLinalg;
  //Command line parsing
  TCLAP::CmdLine cmd("Compute MS-Complex and summary representation", ' ', "1");

  TCLAP::ValueArg<Precision> sigmaArg("s","sigma",
      "Kernel regression bandwith (sigma for Gaussian)", 
      true, 1,  "float");
  cmd.add(sigmaArg);  
    
  TCLAP::ValueArg<std::string> xArg("x","domain","Data points in domain", 
      true,  "", "");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> fArg("f","function","f(x), function value for each data point in X", 
      true,  "", "");
  cmd.add(fArg);

  TCLAP::ValueArg<int> pArg("p","persistence",
      "Number of persistence levels to compute; all = -1 , default = 20", 
      true, 20,  "integer");
  cmd.add(pArg);  
  
  TCLAP::ValueArg<int> samplesArg("n","samples",
    "Number of samples for each regression curve, default = 50", 
    true, 50,  "integer");
  cmd.add(samplesArg);  

  TCLAP::ValueArg<int> knnArg("k","knn",
      "Number of nearest neighbors for Morse-Smale approximation, default = 50", 
      true, 50,  "integer");
  cmd.add(knnArg); 

  TCLAP::SwitchArg randArg("r", "random", 
    "Adds 0.0001 * range(f) uniform random noise to f, in case of 0 gradients due to equivivalent values", false); 
  cmd.add(randArg);

  TCLAP::ValueArg<double> smoothArg("", "smooth", 
    "Smooth function values to nearest nieghbor averages", false, 0, "double"); 
  cmd.add(smoothArg);
    
  try {
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }

  try {
    // Read geometry and function
    Xall = LinalgIO<Precision>::readMatrix(xArg.getValue());
    yall = LinalgIO<Precision>::readVector(fArg.getValue());

    // Add noise to yall in case of equivivalent values 
    if (randArg.getValue()) {
       Random<Precision> rand;
       double a = 0.00000001 *( Linalg<Precision>::Max(yall) - Linalg<Precision>::Min(yall));
       for (unsigned int i=0; i < yall.N(); i++) {
         yall(i) += rand.Uniform() * a;
       }
    }

    // Number of samples for regression curve
    int nSamples = samplesArg.getValue();
    
    // Number of nearest nieghbor for Morse-Samle complex computation
    int knn = knnArg.getValue();
   
    // Bandwidth for inverse regression
    Precision sigma = sigmaArg.getValue();
    Precision sigmaSmooth = smoothArg.getValue();
   
    // Compute Morse-Smale complex
    NNMSComplex<Precision> msComplex(Xall, yall, knn, sigmaSmooth > 0, 0.01, sigmaSmooth*sigmaSmooth );
    
    // Store persistence levels
    persistence = msComplex.getPersistence();
    
    // Save geometry and function
    std::string geomFile = "Geom.data";
    LinalgIO<Precision>::writeMatrix(geomFile, Xall);   

    std::string fFile = "Function.data";
    LinalgIO<Precision>::writeVector(fFile, yall);   


    // Scale persistence to be in [0,1]
    DenseVector<Precision> pScaled(persistence.N());
    Precision fmax = Linalg<Precision>::Max(yall);
    Precision fmin = Linalg<Precision>::Min(yall);
    Precision frange = fmax - fmin;
    for (unsigned int i=0; i < persistence.N(); i++) {
      pScaled(i) = persistence(i) / frange;
    }
    pScaled(pScaled.N()-1) = 1;
    std::string psFile = "Persistence.data";
    LinalgIO<Precision>::writeVector(psFile, pScaled);  


    // Read number of persistence levels to compute visualization for
    int nlevels = pArg.getValue();
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
    LinalgIO<Precision>::writeVector("PersistenceStart.data", pStart);  


    // Compute inverse regression curves and additional information for each
    // crystal
    for (unsigned int nP = start; nP < persistence.N(); nP++){
      // Number of extrema in current crystal
      // int nExt = persistence.N() - nP + 1;         // jonbronson commented out 8/16/17
      msComplex.mergePersistence(persistence(nP));
      crystalIDs.deallocate();
      crystalIDs = msComplex.getPartitions();
      crystals.deallocate();
      crystals = msComplex.getCrystals();
      
      // Find global minimum as refernce point for aligning subsequent persistence
      // levels
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
      std::stringstream crystalsFile;
      crystalsFile << "Crystals_" << nP << ".data";
      LinalgIO<int>::writeMatrix(crystalsFile.str(), crystalTmp); 
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
      EuclideanMetric<Precision> l2;

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


        std::stringstream ss2;
        ss2 << "ps_" << nP << "_crystal_" << crystalIndex << "_Rs.data";
        LinalgIO<Precision>::writeMatrix(ss2.str(), ScrystalIDs[crystalIndex]);


        std::stringstream ss2a;
        ss2a << "ps_" << nP << "_crystal_" << crystalIndex << "_gradRs.data";
        LinalgIO<Precision>::writeMatrix(ss2a.str(), gradS);
        gradS.deallocate(); 

        std::stringstream ss812;
        ss812 <<"ps_" << nP << "_crystal_" << crystalIndex << "_Svar.data";
        LinalgIO<Precision>::writeMatrix(ss812.str(), Svar);
        Svar.deallocate();
       

        std::stringstream ss6;
        ss6 <<"ps_" << nP << "_crystal_" << crystalIndex << "_mdists.data";
        LinalgIO<Precision>::writeVector(ss6.str(), pdist);

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
        std::stringstream ss7;
        ss7 <<"ps_" << nP << "_crystal_" << crystalIndex << "_fmean.data";
        LinalgIO<Precision>::writeVector(ss7.str(), fmean);
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
        std::stringstream ss8;
        ss8 << "ps_" << nP << "_crystal_" << crystalIndex << "_spdf.data";
        LinalgIO<Precision>::writeVector(ss8.str(), spdf);
        spdf.deallocate(); 


        

        X.deallocate();
        Zp.deallocate();
        y.deallocate();
      }
      // end of regression  loop




      // Save maximal extrema width
      std::stringstream extw;
      extw << "ExtremaWidths_" << nP << ".data";
      LinalgIO<Precision>::writeVector(extw.str(), eWidths);
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
      computeIsomapLayout(S, ScrystalIDs, l2, nExt, nSamples, nP);     
      

      S.deallocate();
      for (unsigned int i=0; i < crystals.N(); i++) { 
        ScrystalIDs[i].deallocate();
        XcrystalIDs[i].deallocate();
        XpcrystalIDs[i].deallocate();
        ycrystalIDs[i].deallocate();
      }
    }
  } catch (const char *err) {
    std::cerr << err << std::endl;
  }
  
  return 0;
}

/**
 * Compute low-d layout of geomtry using PCA.
 */
void computePCALayout(FortranLinalg::DenseMatrix<Precision> &S, int nExt, int nSamples, unsigned int nP) {
  using namespace FortranLinalg;
  unsigned int dim = 2; 
  PCA<Precision> pca(S, dim);        
  DenseMatrix<Precision> fL = pca.project(S);
  if (fL.M() < dim) {
    DenseMatrix<Precision> fLtmp(dim, fL.N());
    Linalg<Precision>::Zero(fLtmp);
    for (unsigned int i=0; i<fL.M(); i++) {
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
    LinalgIO<Precision>::writeVector("PCAMin.data", Lmin);
    LinalgIO<Precision>::writeVector("PCAMax.data", Lmax);
    Lmin.deallocate();
    Lmax.deallocate();
  }

  // Save layout for each crystalIDs - stretch to extremal points.
  for (unsigned int i =0; i<crystals.N(); i++) {
    DenseMatrix<Precision> tmp(fL.M(), nSamples);
    DenseVector<Precision> a(fL.M());
    DenseVector<Precision> b(fL.M());
    Linalg<Precision>::ExtractColumn(E, exts[crystals(1, i)], a );
    Linalg<Precision>::ExtractColumn(E, exts[crystals(0, i)], b );

    for(unsigned int j=0; j<tmp.N(); j++){
      Linalg<Precision>::SetColumn(tmp, j, fL, nSamples*i+j);
    }

    Linalg<Precision>::Subtract(a, tmp, 0, a);
    Linalg<Precision>::AddColumnwise(tmp, a, tmp);
    Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

    DenseVector<Precision> stretch(fL.M());
    for(unsigned int j=0; j<tmp.N(); j++){
      Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
      Linalg<Precision>::Add(tmp, j, stretch);
    }

    std::stringstream ss;
    ss <<"ps_" << nP << "_crystal_" << i << "_layout.data";
    LinalgIO<Precision>::writeMatrix(ss.str(), tmp);

    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
  }

  std::stringstream extL;
  extL << "ExtremaLayout_" << nP << ".data";
  LinalgIO<Precision>::writeMatrix(extL.str(), E);
  E.deallocate();

  std::stringstream extf;
  extf << "ExtremaValues_" << nP <<".data";
  LinalgIO<Precision>::writeVector(extf.str(), Ef);
  Ef.deallocate();

  pca.cleanup();
  fL.deallocate();      
}

/**
 * Compute low-d layout of geomtry using PCA Extrema
 */
void computePCAExtremaLayout(FortranLinalg::DenseMatrix<Precision> &S, 
  std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, int nExt, int nSamples, unsigned int nP) {
  using namespace FortranLinalg;
  unsigned int dim = 2; 
  DenseMatrix<Precision> Xext(Xall.M(), nExt);
  for (int i=0;i<nExt; i++) {
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
  }
  else {
    extremaPosPCA2 = Linalg<Precision>::Copy(pca2L);       
    DenseVector<Precision> Lmin = Linalg<Precision>::RowMin(pca2L);
    DenseVector<Precision> Lmax = Linalg<Precision>::RowMax(pca2L);
    LinalgIO<Precision>::writeVector("PCA2Min.data", Lmin);
    LinalgIO<Precision>::writeVector("PCA2Max.data", Lmax);
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


    std::stringstream ss;
    ss <<"ps_" << nP << "_crystal_" << i << "_pca2layout.data";
    LinalgIO<Precision>::writeMatrix(ss.str(), tmp);
    
    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
    pca.cleanup();
  }

  // Save extremal points location and function value.
  std::stringstream pca2extL;
  pca2extL << "PCA2ExtremaLayout_" << nP <<".data";
  LinalgIO<Precision>::writeMatrix(pca2extL.str(), pca2L);
  pca2L.deallocate();
  pca2.cleanup();         
}

/**
 * Compute low-d layout of geomtry using Isomap algorithm
 */
void computeIsomapLayout(FortranLinalg::DenseMatrix<Precision> &S, 
  std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
  EuclideanMetric<Precision> &l2, int nExt, int nSamples, unsigned int nP) {
  // Do an isomap layout.
  using namespace FortranLinalg;
  unsigned int dim = 2; 
  SparseMatrix<Precision> adj(nExt, nExt, std::numeric_limits<Precision>::max());
  for (unsigned int i=0; i<crystals.N(); i++) {
    Precision dist = 0; 
    for (int j=1; j< nSamples; j++) {
      int index1 = nSamples*i+j;
      int index2 = index1 - 1;
      dist += l2.distance(S, index1, S, index2);
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
    LinalgIO<Precision>::writeVector("IsoMin.data", Lmin);
    LinalgIO<Precision>::writeVector("IsoMax.data", Lmax);
    Lmin.deallocate();
    Lmax.deallocate();

    // Store original extream indicies.
    extsOrig = exts;
  }


  // Save layout for each crystal - stretch to extremal points.
  for (unsigned int i =0; i<crystals.N(); i++) { 
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
    for(unsigned int j=0; j<tmp.N(); j++){
      Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
      Linalg<Precision>::Add(tmp, j, stretch);
    }


    std::stringstream ss;
    ss <<"ps_" << nP << "_crystal_" << i << "_isolayout.data";
    LinalgIO<Precision>::writeMatrix(ss.str(), tmp);
    
    a.deallocate();
    b.deallocate();
    tmp.deallocate();
    stretch.deallocate();
    pca.cleanup();
  }

  // Save extremal points location and function value
  std::stringstream isoextL;
  isoextL << "IsoExtremaLayout_" << nP <<".data";
  LinalgIO<Precision>::writeMatrix(isoextL.str(), isoL); 
  isoL.deallocate();
}
