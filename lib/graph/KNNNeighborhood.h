#ifndef KNNNEIGHBORHOOD_H
#define KNNNEIGHBORHOOD_H

#include "Distance.h"
#include "EuclideanMetric.h"
#include "Neighborhood.h"
#include "SparseMatrix.h"

template <typename TPrecision>
class KNNNeighborhood : public Neighborhood<TPrecision>{
 
                                          
  public:
    
    KNNNeighborhood(unsigned int k, TPrecision value = std::numeric_limits<TPrecision>::max()) : 
                               knn(k), val(value){};

   

    FortranLinalg::SparseMatrix<TPrecision> generateNeighborhood(FortranLinalg::Matrix<TPrecision> &data){
    
      using namespace FortranLinalg;


      //knn 
      DenseMatrix<int> knns(knn, data.N());
      DenseMatrix<TPrecision> knnDists(knn, data.N());
      Distance<TPrecision>::computeKNN(data, knns, knnDists, euclideanMetric);  

      //complete adjancy matrix
      SparseMatrix<TPrecision> adj(data.N(), data.N(), val);
      
      for(unsigned int i=0; i< adj.N(); i++){
        typename SparseMatrix<TPrecision>::SparseEntry *entry = adj.getEntries(i); 
        for(unsigned int k=1; k < knn; k++){
          entry->operator[](knns(k, i)) = knnDists(k, i);
          typename SparseMatrix<TPrecision>::SparseEntry *entry2 = adj.getEntries(knns(k, i));
          entry2->operator[](i) = knnDists(k, i);
        }
      }

      //cleanup memmory
      knns.deallocate();
      knnDists.deallocate();      
      return adj; 

    };

  
  private:
    unsigned int knn;
    TPrecision val;
    EuclideanMetric<TPrecision> euclideanMetric;


                                         
};

#endif
