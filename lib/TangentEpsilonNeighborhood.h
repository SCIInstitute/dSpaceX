#ifndef TANGENTEPSILONNEIGHBORHOOD_H
#define TANGENTEPSILONNEIGHBORHOOD_H

#include "Neighborhood.h"
#include "EpsilonNeighborhood.h"
#include "Geometry.h"
#include "PCA.h"
#include "MahalanobisMetric.h"
#include "EuclideanMetric.h"

#include "SparseMatrix.h"

template <typename TPrecision>
class TangentEpsilonNeighborhood : public Neighborhood<TPrecision>{
 
                                          
  public:
    
    TangentEpsilonNeighborhood(TPrecision epsilon, TPrecision value) : 
                               eps(epsilon), val(value), 
                               epsn(metric, epsilon, value){};

   

    SparseMatrix<TPrecision> generateNeighborhood(Matrix<TPrecision> &data){
      
      
      DenseVector<TPrecision> dists(data.N());
      SparseMatrix<TPrecision> adjTmp = epsn.generateNeighborhood(data);

      SparseMatrix<TPrecision> adj(data.N(), data.N());

      for(unsigned int i=0; i< adj.N(); i++){
        std::cout << "Tangent neighborhood at: " << i << std::endl;
        typename SparseMatrix<TPrecision>::SparseEntry *neighbors = adjTmp.getEntries(i);
        typename SparseMatrix<TPrecision>::SparseEntryIterator it = neighbors->begin();
       
        //Fill neighborhood matrix 
        DenseMatrix<TPrecision> ndata(data.M(), neighbors->size());
        int index = 0;
        for(; it!=neighbors->end(); ++it){
          for(unsigned int j=0; j < data.M(); j++){
            ndata(j, index) = data(j, it->first);
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
        Geometry<TPrecision>::computeDistances(data, i, mahal, dists); 
        typename SparseMatrix<TPrecision>::SparseEntry *entry = adj.getEntries(i); 
        for(unsigned int k=0; k< dists.N(); k++){
          if(k !=i && dists(k) < eps){
            entry->operator[](k) = dists(k);
          }
        }

        ndata.deallocate();
        pca.cleanup();
      }

      adjTmp.deallocate();
      dists.deallocate(); 

      return adj; 

    };

  
  private:
    EuclideanMetric<Precision> metric;
    TPrecision eps;
    TPrecision val;
    EpsilonNeighborhood<TPrecision> epsn;


                                         
};

#endif
