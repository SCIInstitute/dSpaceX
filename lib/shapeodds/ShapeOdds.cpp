#include "ShapeOdds.h"
#include "imageutils/ImageLoader.h"
#include "lodepng.h"

namespace Shapeodds {

ShapeOdds::ShapeOdds()
{
  using namespace Eigen;
  
#if 0  // was just learning how to use the Eigen library
  // Inline mesh of a cube
  my_V_matrix = (MatrixXd(8,3)<<
                 0.0,0.0,0.0,
                 0.0,0.0,1.0,
                 0.0,1.0,0.0,
                 0.0,1.0,1.0,
                 1.0,0.0,0.0,
                 1.0,0.0,1.0,
                 1.0,1.0,0.0,
                 1.0,1.0,1.0).finished();
  my_F_matrix = (MatrixXi(12,3)<<
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
  
  // same as: Map<Matrix2i> imat(idata);
  Map<MatrixXi> imat(idata,2,2);
  std::cout << "imat (map of idata):\n" << imat << std::endl;
  std::cout << "imat.row(0):\n" << imat.row(0) << std::endl;
  std::cout << "imat.col(0):\n" << imat.col(0) << std::endl;
  std::cout << "imat.minCoeff():\n" << imat.minCoeff() << std::endl;
  std::cout << "imat.colwise().minCoeff():\n" << imat.colwise().minCoeff() << std::endl;
  std::cout << "imat.maxCoeff():\n" << imat.maxCoeff() << std::endl;
  std::cout << "imat.colwise().maxCoeff():\n" << imat.colwise().maxCoeff() << std::endl;
  
  Map<MatrixXf> fmat(fdata,2,2);
  std::cout << "fmat (map of fdata):\n" << fmat << std::endl;
  fmat.array() *= -1.0;
  std::cout << "fmat * -1.0:\n" << fmat << std::endl;
  fmat = fmat.array().exp();
  std::cout << "e^fmat:\n" << fmat << std::endl;
  fmat.array() += 1.0;
  std::cout << "fmat + 1.0:\n" << fmat << std::endl;
  fmat = fmat.array().inverse();
  std::cout << "1.0/fmat:\n" << fmat << std::endl;

  Map<RowVectorXf> fvec_of_fmat(fmat.data(), fmat.size());
  std::cout << "vec of fmat:\n" << fvec_of_fmat << std::endl;
  Matrix<float, Dynamic, Dynamic, RowMajor> fmat_rm(fmat);
  Map<RowVectorXf> fvec_of_fmat_rm(fmat_rm.data(), fmat_rm.size());
  std::cout << "row major vec of fmat:\n" << fvec_of_fmat_rm << std::endl;
  
  //colmajor column to rowmajor image (just like I below)
  int imgdata[] = {1,2,4,8,16,32};
  Map<MatrixXi> imat_61(imgdata, 6, 1);
  std::cout << "col major 6 x 1 imat_61:\n" << imat_61 << std::endl;
  typedef Matrix<int,Dynamic,Dynamic,RowMajor> rowmaj_image;
  Map<rowmaj_image> imat_23_rowmaj(imat_61.data(), 2, 3);
  std::cout << "row major 2 x 3 image imat_61:\n" << imat_23_rowmaj << std::endl;
  typedef Matrix<int,Dynamic,Dynamic,ColMajor> colmaj_image;
  Map<colmaj_image> imat_32_colmaj(imat_61.data(), 3, 2);
  std::cout << "col major 3 x 2 image imat_61:\n" << imat_32_colmaj << std::endl;

  MatrixXf M1(2,6);    // Column-major storage
  M1 << 1, 2, 3,  4,  5,  6,
        7, 8, 9, 10, 11, 12;
  std::cout << "M1:" << std::endl << M1 << std::endl;
  Map<MatrixXf> M2(M1.data(), 6,2);
  std::cout << "M2:" << std::endl << M2 << std::endl;
#endif
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

// TODO if helpful; e.g., if we want to write [8|16|32|64]-bit greyscale this fcn could take care of resampling appropriately)
// void writeImage(const Eigen::MatrixXd &column_order_image, std::string path, unsigned w, unsigned h)
// {
// }

// creates a new sample (an image) from the given model at the specified latent space coordinate
// write evaluated model to disk if requested (if w * h != I.size(), then write I.rows() x I.cols() image)
Eigen::MatrixXd ShapeOdds::evaluateModel(Model &model, const Eigen::VectorXd &z_coord,
                                         const bool writeToDisk, const std::string outpath, unsigned w, unsigned h)
{
  // I = f(z):
  //  phi = W * z + w0
  //  I = 1 / ( 1 + e^(-phi) )

  // the z_coord should just be one row
  //std::cout << "latent space coord (z):\n" << z_coord << std::endl;
  Eigen::MatrixXd phi = model.W * z_coord + model.w0;
  //std::cout << "phi = W * z + w0:\n" << phi << std::endl;

  //I = 1.0 / (1 + exp(-phi));
  phi.array() *= -1.0;
  phi = phi.array().exp();
  phi.array() += 1.0;
  Eigen::MatrixXd I(phi.array().inverse());
  //std::cout << "I = 1 / (1 + e^(-phi)):\n" << I << std::endl;
  
  if (writeToDisk)
  {
    I.array() *= 255.0;
    //std::cout << "I * 255:\n" << I << std::endl;

    if (I.size() != w * h)
    { 
      std::cout << "Warning: w * h (" << w << " * " << h << ") != computed image size (" << I.size() << ")\n";
      w = I.rows();
      h = I.cols();
    }

    Eigen::Map<Eigen::MatrixXd> I_image(I.data(), h, w);  // column major ordering
    unsigned char buffer[I.size()];
    std::vector<unsigned char> image;
    unsigned error;

    // copy image into row-order array
    for (unsigned c = 0; c < w; c++)    // for every column...
      for (unsigned r = 0; r < h; r++)  // read all the rows
        buffer[r * w + c] = (unsigned char)std::round(I_image(r, c));

    error = lodepng::encode(image, buffer, w, h, LCT_GREY, 8);
    if (error) {
      throw std::runtime_error("encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
    }

    lodepng::save_file(image, outpath.c_str());
  }
      
  return I;
}

// verify evaluated model and how closely it corresponds to sample at that idx
// return measured difference between generated sample and original
float ShapeOdds::testEvaluateModel(Model &model, const Eigen::Matrix<double, 1, Eigen::Dynamic> &z_coord, unsigned p, unsigned c, unsigned z_idx,
                                   const bool writeToDisk, const std::string basePath, const unsigned w, const unsigned h)
{
  std::string outpath(basePath + "/p" + std::to_string(p) + "-c" + std::to_string(c) +"-z" + std::to_string(z_idx) + ".png");

  Eigen::MatrixXd I(evaluateModel(model, z_coord, writeToDisk, outpath, w, h));

#if 0  //<ctc> moved this here since evaluateModel doesn't need to know model's persistence, crystalid, or index of z_coord

  // how to figure out which image goes with which row of Z... (this is actually done now in the code that calls this fcn)
  // -> the sample indices are the key: each sample corresponds to one of the 1000 z's for this model
  // -> also, while technically the z's don't have to be saved, it's very useful to store them
  //test by loading the images for each element (are they correlated? there aren't the same number...)
  //and compare pixels to start with (list of samples is stored in the model's Crystal)
  //
  // samples of p0-c0: <manually get 'em>
  //  - not sure if order matters...
  //   [] just compute them all to compare images for each Z
  //     - still leaves some samples out though since there are fewer Zs than samples...)
  //  - there are fewer Z rows than samples, so...
  //  - how do I pick the z coordinate corresponding to a sample?
  //
  const std::set<unsigned> &sample_indices = model.getCrystal().getSamples();
  for (unsigned sample_idx : sample_indices)
  {
    std::string path("/Users/cam/data/dSpaceX/DATA/CantileverBeam_wclust_wraw/images/1.png");
    std::string outpath("/Users/cam/data/dSpaceX/DATA/CantileverBeam_wclust_wraw/outimages/1.png");
    unsigned w,h;
    Image image = imageLoader.loadImage(path, ImageLoader::Format::PNG);

    // just use lodepng since it's super simple
    {
      std::vector<unsigned char> image;
      std::vector<unsigned char> buffer;
      lodepng::State state;
      unsigned error;

      state.decoder.color_convert = 0;
      state.decoder.remember_unknown_chunks = 1; //make it reproduce even unknown chunks in the saved image

      lodepng::load_file(buffer, path.c_str());
      error = lodepng::decode(image, w, h, state, buffer);
      if(error) {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return 0;
      }

      buffer.clear();

      state.encoder.text_compression = 1;

      error = lodepng::encode(buffer, image, w, h, state);
      if(error) {
        std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return 0;
      }

      lodepng::save_file(buffer, outpath);
    }
  }
#endif

  return true;
}

ModelPair MSModelContainer::getModel(unsigned p, unsigned c)
{
  if (p >= persistence_levels.size() || c >= persistence_levels[p].numCrystals())
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  return ModelPair(modelName(p, c), persistence_levels[p].getCrystal(c).getModel());
}

std::vector<ModelPair> MSModelContainer::getAllModels()
{
  unsigned persistence_padding = paddedStringWidth(persistence_levels.size());

  std::vector<ModelPair> models;  
  for (unsigned p = 0; p < persistence_levels.size(); p++)
  {
    unsigned crystals_padding = persistence_levels[p].numCrystals();
    for (unsigned c = 0; c < persistence_levels[p].numCrystals(); c++)
    {
      models.push_back(ModelPair(modelName(p,c,persistence_padding,crystals_padding),
                                 persistence_levels[p].getCrystal(c).getModel()));
    }
  }
  return models;
}


} // end namespace Shapeodds
