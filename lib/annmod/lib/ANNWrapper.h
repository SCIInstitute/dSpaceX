#ifndef ANNWRAPPER_H
#define ANNWRAPPER_H

#include "DenseMatrix.h"
#include "DenseVector.h"

#include "ANN/ANN.h"


template <typename TPrecision>
class ANNWrapper{
  
  public:
 
    static void computeANN(FortranLinalg::DenseMatrix<double> &data,
        FortranLinalg::DenseMatrix<int> &knn, FortranLinalg::DenseMatrix<double>
        &dists, double eps){

      ANNpointArray pts= data.getColumnAccessor();

      ANNkd_tree *annTree = new ANNkd_tree( pts, data.N(), data.M()); 
     
      int **knnData = knn.getColumnAccessor();
      double **distData = dists.getColumnAccessor();

      for(unsigned int i = 0; i < data.N(); i++){
        annTree->annkSearch( pts[i], knn.M(), knnData[i], distData[i], eps);
      }
    
      delete annTree;
      annClose(); // done with ANN

    };  

 
    
};

#endif
