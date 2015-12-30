#ifndef TANGENTKNNNEIGHBORHOOD_H
#define TANGENTKNNNEIGHBORHOOD_H

#include "Neighborhood.h"
#include "Distance.h"
#include "PCA.h"
#include "MahalanobisMetric.h"
#include "EuclideanMetric.h"

#include "SparseMatrix.h"

template <typename TPrecision>
class TangentKNNNeighborhood : public Neighborhood<TPrecision>{
 
                                          
  public:
    
    TangentKNNNeighborhood(int k, TPrecision value) : 
                               knn(k), val(value){};

   

    FortranLinalg::SparseMatrix<TPrecision> generateNeighborhood(FortranLinalg::Matrix<TPrecision> &data){
      using namespace FortranLinalg;
      //knn 
      DenseMatrix<int> knns(knn, data.N());
      DenseMatrix<TPrecision> knnDists(knn, data.N());
      Distance<TPrecision>::computeKNN(data, knns, knnDists, euclideanMetric);  

      //complete adjancy matrix
      SparseMatrix<TPrecision> adj(data.N(), data.N(), val);

      DenseMatrix<TPrecision> ndata(data.M(), knn);
    
      DenseVector<int> nn(knn);
      DenseVector<TPrecision> nnDists(knn);
      
      
      for(unsigned int i=0; i< adj.N(); i++){
       //Fill neighborhood matrix 
        int index = 0;
        for(int k=0; k<knn; ++k){
          for(unsigned int j=0; j < data.M(); j++){
            ndata(j, index) = data(j, knns(k, i));
          }          
          index++;
        }

        //Do local pca
        PCA<TPrecision> pca(ndata, 0, true);
        for(unsigned int j=0; j<pca.ev.N(); j++){
          for(unsigned int k=0; k<pca.ev.M(); k++){
            pca.ev(k, j) /= sqrt(pca.ew(j));
          }
        }


        MahalanobisMetric<TPrecision> mahal(pca.ev);
        Distance<TPrecision>::computeKNN(data, i, nn, nnDists, mahal);
        typename SparseMatrix<TPrecision>::SparseEntry *entry = adj.getEntries(i); 
        for(int k=1; k < knn; k++){
              entry->operator[](nn(k)) = nnDists(k);
        }

        pca.cleanup();
      }

      //cleanup memmory
      nn.deallocate();
      nnDists.deallocate();
      knns.deallocate();
      knnDists.deallocate();
      ndata.deallocate();
      
      return adj; 

    };

  
  private:
    EuclideanMetric<Precision> euclideanMetric;
    int knn;
    TPrecision val;


                                         
};

#endif
