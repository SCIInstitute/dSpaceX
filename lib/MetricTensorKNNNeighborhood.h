#ifndef METRICTENSORKNNNEIGHBORHOOD_H
#define METRICTENSORKNNNEIGHBORHOOD_H

#include "Neighborhood.h"
#include "EpsilonNeighborhood.h"
#include "Geometry.h"
#include "PCA.h"
#include "MahalanobisMetric2.h"
#include "EuclideanMetric.h"

#include "SparseMatrix.h"

template <typename TPrecision>
class MetricTensorKNNNeighborhood : public Neighborhood<TPrecision>{
 
                                          
  public:
    
    MetricTensorKNNNeighborhood(int k, TPrecision a, TPrecision value) : 
                               knn(k), alpha(a), val(value){};

   

    SparseMatrix<TPrecision> generateNeighborhood(Matrix<TPrecision> &data){
      
      
      //knn 
      DenseMatrix<int> knns(knn, data.N());
      DenseMatrix<TPrecision> knnDists(knn, data.N());
      Geometry<TPrecision>::computeKNN(data, knns, knnDists, euclideanMetric);  

      //complete adjancy matrix
      SparseMatrix<TPrecision> adj(data.N(), data.N(), val);

      //Metric tensors J = G^T*G
      //save G fro creating mahalanobis distance metric afterwards
      DenseMatrix<TPrecision> J[data.N()];

      //Compute Metric Tensor at each point
      DenseMatrix<TPrecision> ndata(data.M(), knn);

      for(unsigned int i=0; i< adj.N(); i++){
        std::cout << "Computing G_" << i << std::endl;
        //Fill neighborhood matrix 
        int index = 0;
        for(int k=0; k<knn; ++k){
          for(unsigned int j=0; j < data.M(); j++){
            ndata(j, index) = data(j, knns(k, i));
          }          
          index++;
        }


        //Do local pca and compute G
        PCA<TPrecision> pca(ndata, 0, true);
        for(unsigned int j=0; j<pca.ev.N(); j++){
          for(unsigned int k=0; k<pca.ev.M(); k++){
            pca.ev(k, j) /= sqrt(pca.ew(j));
          }
        }
        J[i] = Linalg<Precision>::Multiply(pca.ev, pca.ev, false, true); 

        

        //Write tensor for visualization in matlab
        //std::stringstream ss;
        //ss << "tensor_" << i <<".data";
        //LinalgIO<TPrecision>::writeMatrix(ss.str().c_str(), J[i]);

        pca.cleanup();
      }


      std::cout << alpha << std::endl;
      //Ensure smootheness of metric tensor with weighting term alpha
      DenseVector<int> nn(knn);
      DenseVector<TPrecision> nnDists(knn);

      DenseMatrix<TPrecision> JTmp(J[0].M(), J[0].N());
        for(unsigned int i=0; i< adj.N(); i++){
          std::cout << "Smoothing neighborhood and computing knn for " << i << std::endl;


          //GTmp_i = 1 / (1+alpha) ( G_i + alpha / knn * sum_nn G_nn)
          Linalg<TPrecision>::Copy(J[i], JTmp);
          //Compute smoothed metric tensor
          for(int k=1; k<knn; ++k){
            Linalg<TPrecision>::Add(JTmp, J[knns(k, i)], JTmp);
          }
          Linalg<TPrecision>::Scale(JTmp, alpha/knn, JTmp);
          Linalg<TPrecision>::Add(JTmp, J[i], JTmp);
          Linalg<TPrecision>::Scale(JTmp, 1/(1+alpha), JTmp);
          
          //Write updated tensor for visualization in matlab
          //std::stringstream ss;
          //ss << "tensorsmooth_" << i <<".data";
          //LinalgIO<TPrecision>::writeMatrix(ss.str().c_str(), JTmp);
          
          //Compute new weighted adjacencies
          MahalanobisMetric2<TPrecision> mahal(JTmp);
          Geometry<TPrecision>::computeKNN(data, i, nn, nnDists, mahal);
          typename SparseMatrix<TPrecision>::SparseEntry *entry = adj.getEntries(i); 
          for(int k=1; k < knn; k++){
            entry->operator[](nn(k)) = nnDists(k);
          }
        }




      //cleanup memmory
      nn.deallocate();
      nnDists.deallocate();
      knns.deallocate();
      knnDists.deallocate();
      JTmp.deallocate();
      ndata.deallocate();
      for(unsigned int i=0; i < data.N(); i++){
        J[i].deallocate();
      }



      return adj; 
    
    };

  
  private:
    EuclideanMetric<Precision> euclideanMetric;
    int knn;
    TPrecision alpha;
    TPrecision val;


                                         
};

#endif
