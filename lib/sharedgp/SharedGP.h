#pragma once

#include <Eigen/Core>

class SharedGP
{
public:
  SharedGP();
  ~SharedGP();

  int doSomething(int x=42);
  
private:
  int do_something_quietly(int y);

  Eigen::MatrixXd my_V_matrix;
  Eigen::MatrixXd my_F_matrix;

};
