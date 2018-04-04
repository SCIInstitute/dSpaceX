#ifndef ISOMAP_H
#define ISOMAP_H

#include "MetricMDS.h"
#include "graph/GraphAlgorithms.h"
#include "graph/Neighborhood.h"

#include <math.h>

template <typename TPrecision>
class Isomap{
  public:
    
    
    Isomap(Neighborhood<TPrecision> *n, int nd) : nb(n), ndims(nd){
    };




    FortranLinalg::DenseMatrix<TPrecision> embed(FortranLinalg::Matrix<TPrecision> &data ){
      FortranLinalg::SparseMatrix<TPrecision> adj = nb->generateNeighborhood(data);
      FortranLinalg::DenseMatrix<TPrecision> result =  embedAdj(adj);
      adj.deallocate();
      return result;
    };




    FortranLinalg::DenseMatrix<TPrecision> embedAdj(FortranLinalg::SparseMatrix<TPrecision> &adj ){
      FortranLinalg::DenseMatrix<TPrecision> dists(adj.N(), adj.N());
      GraphAlgorithms<TPrecision>::all_pairs_dijkstra(adj, dists);

      for(unsigned int i=0; i<dists.N(); i++){
        dists(i, i) = 0;
        for(unsigned int j=i+1; j<dists.N(); j++){
            TPrecision m = std::min(dists(i, j), dists(j, i));
            dists(i, j) = m;
            dists(j, i) = m;
        }
      }

      FortranLinalg::DenseMatrix<TPrecision> em = mds.embed(dists, ndims);
      dists.deallocate();
      return em;
    };


    
  
  private:
    Neighborhood<TPrecision> *nb;
    MetricMDS<TPrecision> mds; 
    int ndims;

};



#endif
