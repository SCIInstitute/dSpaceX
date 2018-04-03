\name{msr-package}
\alias{msr}
\alias{msr-package}
\title{Data Analysis with the Morse-Smale Complex }
\docType{package}
\description{
Discrete Morse-Smale complex approximation based on k-NN graph. The Morse-Smale
complex provides a decomposition of the domain. This package provides methods to
compute a hierarchical sequence of Morse-Smale complicies and tools that exploit
this domain decomposition for regression and visualization of scalar functions.
}

\details{
The core functionality rests on the discrete approximation of the Morse-Smale
complex from a sample of a function (see \code{msc.nn}, \code{msc.nn.svm},
\code{msc.nn.kd}.

Based on this functionality the regression approach in [2] (see \code{msc.lm}
and \code{msc.slm}) and the explorartory data anlsysis approach based on the
visualization in [3] (see \code{plot.msc}) is implemented.  
}



\author{
  Samuel Gerber, Kristi Potter, Oliver Ruebel
}
\seealso{
  \code{\link{msc.nn}}
  \code{\link{msc.nn.svm}}
  \code{\link{msc.nn.kd}}
  \code{\link{predict.msc}}
  \code{\link{plot.msc}}
  \code{\link{msc.lm}}
  \code{\link{msc.elnet}}
  \code{\link{msc.slm}},
  \code{\link{msc.slm.elnet}},
}
\references{
  [1] Samuel Gerber and Kristin Potter
  The Morse-Smale Complex for Data Analysis,
  Journal of Statistical Software, 2012, vol. 50, no. 2, pp 1-22 
  
  [2] Samuel Gerber, Oliver Ruebel Peer-Timo Bremer, Valerio Pascucci, Ross Whitaker,
  Morse-Smale Regression, Journal of Computational and Graphical Statistics, 2012

  [3] Samuel Gerber, Peer-Timo Bremer, Valerio Pascucci, Ross Whitaker,
  Visual Exploration of High Dimensional Scalar Functions,
  IEEE Transactions on Visualization and Computer Graphics, vol. 16,
  no. 6, pp 1271-1280, Nov.-Dec. 2010.

  
  David M. Mount and Sunil Arya
  ANN library
  \url{http://www.cs.umd.edu/~mount/ANN/}
}
\keyword{cluster,multivariate,toplogy,nonparametric,models,nonlinear}
