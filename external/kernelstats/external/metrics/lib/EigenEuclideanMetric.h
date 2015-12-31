#ifndef EIGENEUCLIDEANMETRIC_H
#define EIGENEUCLIDEANMETRIC_H

#include "EigenMetric.h"
#include <math.h>


template<typename TPrecision>
class EuclideanMetric : public Metric<TPrecision>{
  public:
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, Eigen::Dynamic> MatrixXp;
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;
  
    virtual ~EuclideanMetric(){};

    virtual TPrecision distance(const VectorXp &x1, const VectorXp &x2){
      return (x1-x2).norm();
    };

};
  

#endif
