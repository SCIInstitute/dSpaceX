#pragma once

#include <Eigen/Core>

// Specification of the computational precision used throughout the library
using Precision = float;
using EigenVectorX      = Eigen::Matrix<Precision, Eigen::Dynamic, 1>;
using EigenRowVectorX   = Eigen::Matrix<Precision, 1, Eigen::Dynamic>;
using EigenMatrixX      = Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>;
