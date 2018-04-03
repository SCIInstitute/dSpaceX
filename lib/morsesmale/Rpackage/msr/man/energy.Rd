\name{camera_estimation}
\alias{energy}
\title{Energy Function of a Camera Estimation Problem}
\description{
Given two images with point correspondences, the goal is to estimate the
translation and rotation of two calibrated cameras.  This problem can be
formulated as a minimization of the total squared algebraic
error:

\eqn{h(R,t)=f(E)=\sum_{i} (x_{i}^{T}Ex'_{i})^2}{h(R,t)=f(E)=\sum_{i} (x_{i}^{T}Ex'_{i})^2}

with \eqn{x_i=[x_{i1}x_{i2}1]^T}{x_i=[x_{i1}x_{i2}1]^T} and
\eqn{x'_i=[x'_{i1}x'_{i2}1]^T}{x'_i=[x'_{i1}x'_{i2}1]^T} being corresponding
points on the image plane defined in the respective camera coordinates.  The
essential matrix \eqn{E=[t]_{\times} R}{E=[t]_{x} R} is a 3 x 3 rank-2 matrix.
In this formulation, the translation between the two cameras is described by
the unit vector t, and the relative camera orientation is defined by the
orthogonal rotation matrix R. Both t and R are expressed in the coordinate
frame of x. Due to the formulation of the problem, E is guaranteed to have only
5 degrees of freedom: 3 to describe the rotation and 2 to determine the
translation up to scale. Hence, h is defined on a 5D manifold embedded in 9D
space. For more detailed information on the definition of this problem, see the
manuscript by.
}

\usage{
energy
}

\author{
  Samuel Gerber
}
\examples{
data(camera_estimation)
summary(energy)
}
\references{
Peter Lindstrom and Mark Duchaineau,
Factoring Algebraic Error for Relative Pose Estimation,
Lawrence Livermore National Laboratory,
LLNL-TR-411194, Mar. 2009
}
\keyword{datasets}
