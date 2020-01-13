//templated C-Style IO functions
#ifndef IO_H
#define IO_H

#include <Eigen/Core>
#include "rapidcsv.h"
#include "time.h" //<ctc> just for profiling, remove when done

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <list>
#include <vector>

class IO{


  public:

  static bool fileExists(const std::string& fileName){
    std::fstream fin;
    fin.open(fileName.c_str(),std::ios::in);
    if( fin.is_open() )
    {
      fin.close();
      return true;
    }
    fin.close();
    return false;
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
    time_t t_start,t_end;
    std::time(&t_start);

#if 0

    std::ifstream fstream(filename);
    std::string line;
    std::string token;

    std::vector<std::vector<T>> matrix;
    while (std::getline(fstream, line)) {
      std::istringstream ss(line);
      std::vector<T> row;
      while (std::getline(ss, token, ',')) {
        row.push_back(std::stod(token));
      }
      matrix.push_back(row);
    }

    std::time(&t_end);
    double time_taken = double(t_end - t_start); 
    std::cout << "Time taken by file reading: " << std::fixed << time_taken << std::setprecision(5) << std::endl;

    // <ctc> todo: is there any way to know the number of rows/cols before reading them all?
    //             maybe read the first row at least to get the number of columns?
    //             this step is rather time-consuming for bigger data...
    // NOTE: the below solution does just that: reads the entire file first
    Eigen::MatrixXd M(matrix.size(),matrix[0].size());
    for (int i = 0; i < matrix.size(); i++) {
      for (int j = 0; j < matrix[0].size(); j++) {
        M(i,j) = matrix[i][j];
      }
    }

    std::time(&t_start);
    time_taken = double(t_start - t_end); 
    std::cout << "Time taken to fill Eigen matrix: " << std::fixed << time_taken << std::setprecision(5) << std::endl;

#else

    // Uses super-handy rapidcsv, https://github.com/d99kris/rapidcsv, which also happens to be faster than the stringstream above.
    rapidcsv::Document csv(filename, rapidcsv::LabelParams(-1, -1), rapidcsv::SeparatorParams(), rapidcsv::ConverterParams(true));

    std::time(&t_end);
    double time_taken = double(t_end - t_start); 
    //std::cout << "Time taken by file reading: " << std::fixed << time_taken << std::setprecision(5) << std::endl; //<ctc>

    size_t rows = csv.GetRowCount();
    size_t cols = csv.GetColumnCount();
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> M(rows, cols);
    for (unsigned i = 0; i < rows; i++) {
      std::vector<double> row = csv.GetRow<double>(i);
      for (unsigned j = 0; j < cols; j++) {
        M(i,j) = row[j];
      }
    }

    std::time(&t_start);
    time_taken = double(t_start - t_end); 
    //std::cout << "Time taken to fill Eigen matrix: " << std::fixed << time_taken << std::setprecision(5) << std::endl; //<ctc>
    
#endif

    return M;
  }


};

#endif
