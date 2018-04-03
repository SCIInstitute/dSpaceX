\name{msc.sublevels}
\alias{msc.sublevels}
\title{Extract levels from Morse Smale Complex}
\description{
Extract a subset of the levels of the current hierarchical levels of the Morse-Smale complex.
This is usefull to save computational time for example for building regression models for only
a single or smaller range sof persistence level of the Morse-Samle hierarchy. 

}

\usage{

msc.sublevels(ms, startLevel = ms$predictLevel, endLevel = startLevel)
}

\arguments{
  \item{ms}{Morse-Smale complex object}
  \item{startLevel}{First Level to include in the new hierarchy}
  \item{endLevel}{Last level to include in the new hierarchy}
}

\value{
  An object of class \code{\link{msc}} with hierarchy levels from  startLevel to 
  endLevel of the input Morse-Smale object.
}

\author{
  Samuel Gerber
}
\examples{
data(fourpeaks)
d <- fourpeaks()

#build Morse-Smale complex of m
ms <- msc.nn(y=d[, 1], x=d[, 2:3], nLevels = 15, knn = 15)

#extract levels 9 through 14
ms <- msc.sublevels(ms, 9, 14)

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
}
\keyword{cluster,multivariate,toplogy,nonparametric,models,nonlinear}
