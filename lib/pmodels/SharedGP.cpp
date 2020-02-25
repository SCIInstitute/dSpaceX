#include "Models.h"

namespace dspacex {

SharedGP::SharedGP()
{
  // Inline mesh of a cube
  my_V_matrix = (Eigen::MatrixXd(8,3)<<
                 0.0,0.0,0.0,
                 0.0,0.0,1.0,
                 0.0,1.0,0.0,
                 0.0,1.0,1.0,
                 1.0,0.0,0.0,
                 1.0,0.0,1.0,
                 1.0,1.0,0.0,
                 1.0,1.0,1.0).finished();
  my_F_matrix = (Eigen::MatrixXi(12,3)<<
                 1,7,5,
                 1,3,7,
                 1,4,3,
                 1,2,4,
                 3,8,7,
                 3,4,8,
                 5,7,8,
                 5,8,6,
                 1,5,6,
                 1,6,2,
                 2,6,8,
                 2,8,4).finished().array()-1;
}

SharedGP::~SharedGP()
{
}

int SharedGP::doSomething(int x)
{
  return do_something_quietly(x);
}
  
int SharedGP::do_something_quietly(int y)
{
  return y*y;
}

} // dspacex
