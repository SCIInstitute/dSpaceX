#include "ShapeOdds.h"

namespace Shapeodds {

ShapeOdds::ShapeOdds()
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

  std::cout << "my_V_matrix: \n" << my_V_matrix << std::endl;
  std::cout << "my_F_matrix: \n" << my_F_matrix << std::endl;

  int idata[] = {1,2,4,8};
  const float fdata[] = {1.234,2.468,4.816,8.1624};
  const std::string sidata("1,2,4,8");
  const std::string sfdata("1.234,2.468,4.816,8.1624");
  
  Eigen::Map<Eigen::Matrix2i> imat(idata);
  std::cout << "imat (map of idata):\n " << imat << std::endl;
  std::cout << "imat.row(0):\n " << imat.row(0) << std::endl;
  std::cout << "imat.col(0):\n " << imat.col(0) << std::endl;
  std::cout << "imat.minCoeff():\n " << imat.minCoeff() << std::endl;
  std::cout << "imat.colwise().minCoeff():\n " << imat.colwise().minCoeff() << std::endl;
  std::cout << "imat.maxCoeff():\n " << imat.maxCoeff() << std::endl;
  std::cout << "imat.colwise().maxCoeff():\n " << imat.colwise().maxCoeff() << std::endl;
  

  //my_F_matrix = (Eigen::MatrixXi(12,3)<<
}

ShapeOdds::~ShapeOdds()
{
}

int ShapeOdds::doSomething(int x)
{
  return do_something_quietly(x);
}
  
int ShapeOdds::do_something_quietly(int y)
{
  return y*y;
}


// creates a new sample (an image) from the given model at the specified latent space coordinate
bool ShapeOdds::evaluateModel(Model &model, FortranLinalg::DenseMatrix<Precision> &z_coord)
{
  // I = f(z):
  //  phi = W * Z + w0
  //  I = 1 / ( 1 + e^(-phi) )

  //TODO
}



} // end namespace Shapeodds
