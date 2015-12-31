#ifndef DISTANCES_H
#define DISTANCES_H

#include "Metric.h"
#include "MinHeap.h"
#include "DenseMatrix.h"
#include "DenseVector.h"


template <typename TPrecision>
class Distance{
  
  public:
    static FortranLinalg::DenseMatrix<TPrecision> computeDistances(FortranLinalg::Matrix<TPrecision> &data, 
                                                    Metric<TPrecision> &metric){
        
        FortranLinalg::DenseMatrix<TPrecision> distances(data.N(), data.N());
  
        computeDistances(data, metric, distances);

        return distances;
    };



    static void computeDistances(FortranLinalg::Matrix<TPrecision> &data, 
                                 Metric<TPrecision> &metric, 
                                 FortranLinalg::Matrix<TPrecision> &distances){
      
        for(unsigned int i=0; i < data.N(); i++){
          distances(i,i) = 0;
          for(unsigned int j=i+1; j<data.N(); j++){
            distances(i,j) = metric.distance(data, i, data, j);
            distances(j,i) = distances(i,j);
          }
        }
    };




    static void computeDistances(FortranLinalg::Matrix<TPrecision> &data,
                                 int index,  
                                 Metric<TPrecision> &metric, 
                                 FortranLinalg::Vector<TPrecision> &distances){
      
        for(unsigned int i=0; i < data.N(); i++){
          distances(i) = 0;
          distances(i) = metric.distance(data, index, data, i);
        }
    };




    static void computeKNN(FortranLinalg::Matrix<TPrecision> &data, FortranLinalg::Matrix<int> &knn,
        FortranLinalg::Matrix<TPrecision> &dists, Metric<TPrecision> &metric){
        
        TPrecision *distances = new TPrecision[data.N()];

        for(unsigned int i = 0; i < data.N(); i++){

          for(unsigned int j=0; j<data.N(); j++){
            distances[j] = metric.distance(data, j, data, i); 
          }
        
          MinHeap<TPrecision> minHeap(distances, data.N());
          for(unsigned int j=0; j < knn.M(); j++){
            knn(j, i) = minHeap.getRootIndex();
            dists(j, i) = minHeap.extractRoot();
          }
        }
        delete[] distances;
    };


    static void computeKNN(FortranLinalg::Matrix<TPrecision> &data, int index,
        FortranLinalg::Vector<int> &knn, FortranLinalg::Vector<TPrecision> &dists, Metric<TPrecision> &metric){
        
        TPrecision *distances = new TPrecision[data.N()];
        for(unsigned int i=0; i<data.N(); i++){
          distances[i] = metric.distance(data, i, data, index); 
        }
        
        MinHeap<TPrecision> minHeap(distances, data.N());
        for(unsigned int i=0; i <knn.N(); i++){
          knn(i) = minHeap.getRootIndex();
          dists(i) = minHeap.extractRoot();
        }
	delete[] distances;
    };
   



    static void computeKNN(FortranLinalg::Matrix<TPrecision> &data,
        FortranLinalg::Vector<TPrecision> &point, FortranLinalg::Vector<int> &knn,
        FortranLinalg::Vector<TPrecision> &dists, Metric<TPrecision> &metric){
        
        TPrecision *distances = new TPrecision[data.N()];
        for(unsigned int i=0; i<data.N(); i++){
          distances[i] = metric.distance(data, i, point); 
        }
        
        MinHeap<TPrecision> minHeap(distances, data.N());
        for(unsigned int i=0; i <knn.N(); i++){
          knn(i) = minHeap.getRootIndex();
          dists(i) = minHeap.extractRoot();
        }
	      delete[] distances;
    };



};

#endif
