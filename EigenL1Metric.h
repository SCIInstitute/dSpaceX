#ifndef EIGENL1METRIC_H
#define EIGENL1METRIC_H

#include "EigenMetric.h"
#include <math.h>


template<typename TPrecision>
class L1Metric : public Metric<TPrecision>{
  public:
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, Eigen::Dynamic> MatrixXp;
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;
  
    virtual ~L1Metric(){};

    virtual TPrecision distance(const VectorXp &x1, const VectorXp &x2){
      return (x1-x2).array().abs().sum();
    }
};
  

#endif
