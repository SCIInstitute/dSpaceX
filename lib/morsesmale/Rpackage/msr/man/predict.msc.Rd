\name{predict.msc}
\alias{predict.msc}
\alias{predict.msc.kd}
\alias{predict.msc.svm}
\alias{predict.msc.lm}
\alias{predict.msc.elnet}
\alias{predict.msc.slm}
\alias{predict.msc.slm.elnet}
\title{Preditcion of partition probabilites of Morse-Smale Complex or regression
prediction for Morse-Smale regression models}
\description{
   For \code{\link{msc.kd}} and \code{\link{msc.svm}} ompute probabilities for
     each crystal in the Morse-Smale complex for each point in X based on a
     kernel density estimat or and one-against all \code{\link{svm}}.   For
     \code{\link{msc.lm}},\code{\link{msc.slm}}  and \code{\link{msc.slm.elnet}}
   the prediction based on the fitted regression models.}

\usage{
\method{predict}{msc.kd}(object, newdata, addExtrema=TRUE, ...)
\method{predict}{msc.svm}(object, newdata, ...)
\method{predict}{msc.lm}(object, newdata, ...)
\method{predict}{msc.slm}(object, newdata, ...)
\method{predict}{msc.slm.elnet}(object, newdata, ...)
}
\arguments{
  \item{object}{Morse-Smale complex object.}
  \item{newdata}{Observations to predict, if missing the sample form the
    Morse-Smale complex are used.}
  \item{addExtrema}{Add the extrema indices of this partion (default TRUE)}
  \item{...}{Further arguments are ignored in these functions}
}
\value{
   For Morse-Smale complex objects a (number of points) x (number of partitions)
   matrix with probabilities $p(C_i|x)$ of belonging to each crystal. For
   regression model objects the predicted function values.
}
\author{
  Samuel Gerber
}
\seealso{
  \code{\link{msc.nn}}
  \code{\link{msc.lm}}
  \code{\link{msc.slm}}
}
\examples{
data(fourpeaks)
d <- fourpeaks()
#build Morse-Smale complex
ms <- msc.nn.svm(y=d[,1], x=d[, 2:3], nLevels=15, knn = 10)
#predict parttion assignments at level 15
ms$predictLevel = 13
p <- predict(ms, d[, 2:3])
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
\keyword{classif}
