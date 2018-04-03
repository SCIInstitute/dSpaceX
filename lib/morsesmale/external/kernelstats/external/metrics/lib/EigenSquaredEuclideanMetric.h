#ifndef EIGENSQUAREDEUCLIDEANMETRIC_H
#define EIGENSQUAREDEUCLIDEANMETRIC_H

#include "EigenMetric.h"
#include <math.h>


template<typename TPrecision>
class SquaredEuclideanMetric : public Metric<TPrecision>{
  public:
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, Eigen::Dynamic> MatrixXp;
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;
    
    
    virtual ~SquaredEuclideanMetric(){};

    virtual TPrecision distance(const VectorXp &x1, const VectorXp &x2){
      return (x1-x2).squaredNorm();
    };


};
  

#endif
