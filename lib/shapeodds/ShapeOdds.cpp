#include "ShapeOdds.h"
#include "imageutils/ImageLoader.h"

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
  float fdata[] = {1.234,2.468,4.816,8.1624};
  const std::string sidata("1,2,4,8");
  const std::string sfdata("1.234,2.468,4.816,8.1624");
  
  // same as: Eigen::Map<Eigen::Matrix2i> imat(idata);
  Eigen::Map<Eigen::MatrixXi> imat(idata,2,2);
  std::cout << "imat (map of idata):\n" << imat << std::endl;
  std::cout << "imat.row(0):\n" << imat.row(0) << std::endl;
  std::cout << "imat.col(0):\n" << imat.col(0) << std::endl;
  std::cout << "imat.minCoeff():\n" << imat.minCoeff() << std::endl;
  std::cout << "imat.colwise().minCoeff():\n" << imat.colwise().minCoeff() << std::endl;
  std::cout << "imat.maxCoeff():\n" << imat.maxCoeff() << std::endl;
  std::cout << "imat.colwise().maxCoeff():\n" << imat.colwise().maxCoeff() << std::endl;
  
  Eigen::Map<Eigen::MatrixXf> fmat(fdata,2,2);
  std::cout << "fmat (map of fdata):\n" << fmat << std::endl;
  fmat.array() *= -1.0;
  std::cout << "fmat * -1.0:\n" << fmat << std::endl;
  fmat = fmat.array().exp();
  std::cout << "e^fmat:\n" << fmat << std::endl;
  fmat.array() += 1.0;
  std::cout << "fmat + 1.0:\n" << fmat << std::endl;
  fmat = fmat.array().inverse();
  std::cout << "1.0/fmat:\n" << fmat << std::endl;
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
  //  phi = W * z + w0
  //  I = 1 / ( 1 + e^(-phi) )
#if 1
  // create Eigen matrices of the three components of the model
  Eigen::Map<Eigen::MatrixXd> _Z(model.Z.data(),model.Z.M(),model.Z.N());
  std::cout << "Z:\n" << _Z << std::endl;
  Eigen::Map<Eigen::MatrixXd> _W(model.W.data(),model.W.M(),model.W.N());
  std::cout << "W:\n" << _W << std::endl;
  Eigen::Map<Eigen::MatrixXd> _w0(model.w0.data(),model.w0.M(),model.w0.N());
  std::cout << "w0:\n" << _w0 << std::endl;

  // the z_coord should just be one row
  //quick check to test: use the first element of the model's Z
  Eigen::VectorXd z = _Z.row(0);
  std::cout << "latent space coord (z):\n" << z << std::endl;
  Eigen::MatrixXd phi = _W * z + _w0;
  std::cout << "phi = W * z + w0:\n" << phi << std::endl;

  //I = 1.0 / (1 + exp(-phi));
  phi.array() *= -1.0;
  phi = phi.array().exp();
  phi.array() += 1.0;
  Eigen::MatrixXd I(phi.array().inverse());
  std::cout << "I = 1 / (1 + e^(-phi)):\n" << I << std::endl;
  I.array() *= 255.0;
  std::cout << "I * 255:\n" << I << std::endl;

  // write this to an image
  //todo Load the image of the first sample and simply compare pixels to start with
  ImageLoader imageLoader;
  std::string path("/Users/cam/data/dSpaceX/DATA/CantileverBeam_wclust_wraw/images/1.png");
  Image image = imageLoader.loadImage(path, ImageLoader::Format::PNG);
  // samples of p0-c0: <manually get 'em>
  //  - not sure if order matters...
  //  - there are fewer Z rows than samples, so...
  //  - how do I pick the z coordinate corresponding to a sample?
  
  //ImageIO<Image>::saveImage(target, "init.mhd");
#endif
}



} // end namespace Shapeodds
