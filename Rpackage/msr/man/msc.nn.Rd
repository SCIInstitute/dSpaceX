\name{msc.nn}
\alias{msc}
\alias{msc.kd}
\alias{msc.svm}
\alias{msc.nn}
\alias{msc.nn.kd}
\alias{msc.nn.svm}
\alias{msc.graph}
\title{Nearest Neighbor Morse Smale Complex}
\description{
Compute a hierarchy of Morse-Smale complex of the scattered data x using a
neareast neighbor based approach at the requested persistence levels. The
persistence is a threshold for merging neighboring extrema. If the difference of
lower function value of the extrema and the saddle connecting them is below
\code{persistence} the extrema are merged.  The \code{msc.nn.svm} and
\code{msc.nn.kd} construct Morse-Smale complex that allow probabilistic
preditcion (using \code{\link{predict}}) pf the partion assignment of unseen
data points, see also \code{\link{predict.msc}}.  
The nearest neighbor computation uses the ANN library by David M. Mount and
Sunil Arya (\url{http://www.cs.umd.edu/~mount/ANN/}).
}

\usage{
  msc.nn(y, x, knn = ncol(x), pLevelP = 0.2, pLevel, 
         nLevels, type = 1, smooth = FALSE, eps=0.01) 
  msc.nn.kd(y, x, knn = ncol(x)*3, pLevelP = 0.2, pLevel, nLevels, 
            bw, type = 1, smooth = FALSE, eps=0.01) 
  msc.nn.svm(y, x, knn = 3*ncol(x), pLevelP = 0.2, pLevel, nLevels, 
             cost = 1,  type = 1, smooth=FALSE, precompute = FALSE, eps=0.01 ) 
  msc.graph(y, x, knn, knnd, nLevels, smooth = FALSE) 
}

\arguments{
  \item{y}{Function values at observations x. A numeric vector is expected.}
  \item{x}{Observations, a numeric marix is expected.}
  \item{knn}{Number of nearest neighbors for graph computation or matrix with nn
            indicies}    
  \item{knnd}{Squared nearest nieghbor distances has to be same size as knn}    
  \item{pLevel}{ Compute the Morse-Smale complex for a single persistence level
     given by pLevel. Here, extrema with persistence less than \code{pLevel} are
     ignored.}
  \item{pLevelP}{Same as pLevel, but instead of an absolute persistence value,
    the persistence level is expressed as a percentage of \code{max(y) -
    min(y)}}
  \item{nLevels}{If specified computes a hierarchical sequence of Morse-Smale
    complicies for 2 to \code{nLevels}$+1$ extrema. I.e. from the highest persistence
    level with a single minimum and maximum to a persitence level with
    \code{nLevels}$+1$ extrema}.
  \item{type}{If 1 use classical persistence for merging based on function value
    difference at saddle points. For other valuse use R^2 measure, i.e. merge
      partitions that results in the most increase in adj.  R^2 value.} 
  \item{smooth}{If the data is very noise many extrema are introduced. If
    \code{smooth} is set to true the steepest ascent/descent is not computed
    based on the raw function values y but based on the function value obtained
    by averaging the function values of the k neareast neighbors.  Effectively,
    a smoothing of the observed function.}
  \item{eps}{The knn computation is based on an approximation. The
  parameter \code{eps} specifices how close the approximation should be, i.e,
  the ratio of distance to approximate neareast neighbor to true neareast
  neigbor is at most $1 + $ \code{eps} (see the ANN webpage for more details
  \url{http://www.cs.umd.edu/~mount/ANN/})}
  \item{bw}{Bandwidth for kernel density estimation in each partition.}
  \item{precompute}{Indicates for each level the SVM should be computed and
    stored. This is useful for speedup if repeated predictions at different
      levels are required.}
  \item{cost}{Cost for svm for partition classification (see also \link{svm}).}
}

\value{
  An object of class \code{"msc"}, \code{"msc.kd"} or \code{"msc.svn"} with the following components:
  \item{level}{Containing the Morse-Smale complex at each persistence level.}
  \item{persistence}{Sorted persistence levels at which two extrema merge.}    
  \item{predictLevel}{For the \link{plot.msc}, \link{predict.msc} methods the
    persistence level of the Morse-Smale hierarchy at which prediction/plotting
    is done}
  \item{nLevels}{number of persistence levels computed, if pLevel or pLevelP 
	  is specified this will be 1.}
  
  with \code{"msc$level"} the following components:
  \item{mins}{Indicies into x of mimima for each partition.}
  \item{maxs}{Indicies into x of maxmima for each partition.}
  \item{partition}{Partition assignment for each observation in x}
  \item{partitionSize}{Number of points in each partition}

}

\author{
  Samuel Gerber
}
\seealso{
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
\examples{

data(fourpeaks)
d <- fourpeaks()

#build Morse-Smale complex of m
ms <- msc.nn(y=d[,1], x=d[, 2:3], pLevel=0.1, knn = 15)
ms.kd <- msc.nn.kd(y=d[,1], x=d[, 2:3], pLevel=0.1, knn = 15, bw=0.1)
ms.svm <- msc.nn.svm(y=d[,1], x=d[, 2:3], pLevel=0.1, knn = 15)

#predict partition assignments
p1 <- predict(ms.kd, d[, 2:3])
p2 <- predict(ms.svm, d[, 2:3])

}
\keyword{cluster,multivariate,toplogy,nonparametric,models,nonlinear}
