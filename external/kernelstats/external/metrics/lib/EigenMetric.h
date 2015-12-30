#ifndef EIGENMETRIC_H
#define EIGENMETRIC_H

#include <Eigen/Dense>

template<typename TPrecision>
class Metric{
    
  public:
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, Eigen::Dynamic> MatrixXp;
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;
  
    virtual ~Metric(){};
    virtual TPrecision distance(const VectorXp &x1, const VectorXp &x2) = 0;
};


#endif

