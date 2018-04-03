#ifndef EPSILONNEIGHBORHOOD_H
#define EPSILONNEIGHBORHOOD_H

#include "Neighborhood.h"
#include "Geometry.h"

#include "SparseMatrix.h"

template <typename TPrecision>
class EpsilonNeighborhood : public Neighborhood<TPrecision>{
 
                                          
  public:
    
    EpsilonNeighborhood(Metric<TPrecision> &m, TPrecision epsilon, TPrecision value) 
                      : metric(m), eps(epsilon), val(value){};

    SparseMatrix<TPrecision> generateNeighborhood(Matrix<TPrecision> &data){
      
      
      SparseMatrix<TPrecision> adj(data.N(), data.N(), val);
      DenseMatrix<TPrecision> distances = Geometry<TPrecision>::computeDistances(data, metric);

      for(unsigned int i=0; i < adj.N(); i++){
        
        typename SparseMatrix<TPrecision>::SparseEntry *entry = adj.getEntries(i); 
        for(unsigned int j=0; j < adj.N(); j++){
          if(i !=j && distances(i, j) < eps){
            entry->operator[](j) = distances(i, j);
          }
        }
      }

      distances.deallocate();
      return adj; 

    };

  
  private:

    Metric<TPrecision> &metric;
    TPrecision eps;
    TPrecision val;

                                         
};

#endif
