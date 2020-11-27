//templated C-Style IO functions
#ifndef IO_H
#define IO_H

#include <Eigen/Core>
#include <csv/rapidcsv.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>
#include <vector>
#include <sys/stat.h>

class IO {
public:

  static bool fileExists(const std::string& filename)
  {
    struct stat buf;
    return stat(filename.c_str(), &buf) != -1;
  }
  
  //read string list text file
  static void writeStringList(const std::string &filename, std::vector< std::string > list){
    std::ofstream file;
    file.open(filename.c_str());
  
    int index =0;
    for(int i=0; i<list.size(); i++){
      file << list[i] << std::endl;
    }
    file.close();
  };

  //read string list text file
  static std::vector<std::string> readStringList(const std::string &filename, int lo = -1){
    std::ifstream file;
    file.open(filename.c_str());
  
    int index =0;
    std::vector<std::string> stringlist;
    while(!file.eof()){

      std::string line;
      getline(file, line);
      if(file.eof()) break;
        if(index != lo){
          stringlist.push_back(line);
        }
      ++index;
    }
    file.close();
    return stringlist;
  };


  template <typename Precision>
  static void readAsciiArrayList(const std::string &filename, int N, std::list<Precision*> &indicies){
    std::ifstream indexFile;
    indexFile.open(filename.c_str());

    while(!indexFile.eof()){
      std::string line;
      getline(indexFile, line);
      if(line.length() > 3){
        Precision *dim = new Precision[N];
        std::stringstream ss(line);
        std::string tmp;  
        int n = 0;
        while(ss >> tmp && n < N){
          dim[n] = (Precision)atof(tmp.c_str());
          n++;
        } 
        indicies.push_back(dim);
      }
    }
    indexFile.close();
  };

  
  template<typename T>
  static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> readCSVMatrix(const std::string &filename)
  {
    rapidcsv::Document csv(filename, rapidcsv::LabelParams(-1, -1), rapidcsv::SeparatorParams(), rapidcsv::ConverterParams(true));
    
    size_t rows = csv.GetRowCount();
    size_t cols = csv.GetColumnCount();
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> M(rows, cols);
    for (unsigned i = 0; i < rows; i++) {
      std::vector<T> row = csv.GetRow<T>(i);
      for (unsigned j = 0; j < cols; j++) {
        M(i,j) = row[j];
      }
    }

    return M;
  }

  /*
   * Reads a binary matrix written in the specified order (can be Eigen::RowMajor or Eigen::ColMajor).
   */
  template<typename T, Eigen::StorageOptions Order = Eigen::RowMajor>
  static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> readBinMatrix(const std::string &filename)
  {
    std::ifstream dims(filename + ".dims");
    unsigned rows{0}, cols{0};
    std::string dtype;
    dims >> rows >> cols >> dtype;
    if (rows == 0 || cols == 0) { throw(std::runtime_error("num rows or cols is zero for binary file containing matrix")); }

    // ensure .bin is of correct type (TODO: enable conversion to requested type)
    if (dtype == "float32" && typeid(T) != typeid(float) || dtype == "float64" && typeid(T) != typeid(double))
      throw std::runtime_error("Binary matrix of type " + dtype + " incompatible with requested type " + typeid(T).name() + "\n\tNOTE: This input matrix can be converted here fairly easily, but better to use data/convert/binarize() Python function to convert the matrix to the desired precision.");

    // read file into a vector
    std::ifstream vals(filename, std::ios::binary);
    if (!vals.is_open()) { throw(std::runtime_error("could not open binary file for reading matrix")); }
    std::vector<char> data_vec = std::vector<char>(std::istreambuf_iterator<char>(vals), {});

    // wrap vector data with a matrix and return it as an rvalue
    Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Order>> M(reinterpret_cast<T*>(data_vec.data()), rows, cols);
    return std::move<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>(M);
  }

};

#endif
