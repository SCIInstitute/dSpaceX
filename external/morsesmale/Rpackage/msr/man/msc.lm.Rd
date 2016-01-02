\name{msc.lm}
\alias{msc.elnet}
\alias{msc.lm}
\title{Morse Smale Complex Linear Regression}
\description{
Piecewise linear regression  on the decomposition of the domain based on the
partion induced by the Morse-Smale complex. For \code{msc.elnet} an elastic net is
fitted insetad of a simple lienar regression.

For prediction the linear model are either avergaed based on weighting the
contributions from each partition for a predicting point or predicted based on
the linear model corresponding to the  highest partition probability. The
weights for each partition are computed depending on the underlying Morse-Smale
complex type (see \code{\link{msc.nn}}). The functions can be called with
\code{\link{msc.nn}} without predictive capacities, then prediction of unseen
data is not supported.
 
}
\usage{
msc.lm(ms, nfold = 10, modelSelect=FALSE, blend=FALSE, verbose=FALSE)
msc.elnet(ms, nfold = 10, blend=FALSE)
}
\arguments{
  \item{ms}{A Morse-Smale complex object, see \code{\link{msc.nn}}}.
  \item{nfold}{Number of folds for crossvlaidation, used for selecting an
    appropriate persitence level if the underlying Morse-Smale complex objects
      has multiple levels.}
  \item{modelSelect}{Do a forward stepwise model selection for each linear model 
    (for each parttion ther eis on linear model)}
  \item{blend}{Use blending for model preidtcion. FALSE results in pecewise
    linear model.}
  \item{verbose}{Print model fitting information}  
}
\value{
  An object of class \code{c("msc.lm")} or \code{c("msc.elnet")}, that can be used for prediction with
\code{\link{predict}}. 

  The object \code{c("msc.lm")} has the following components:
  \item{ms}{The Morse-Smale complex, see \code{\link{msc.nn}} }
  \item{lms}{The linear models and crossvalidation results for each level in ms.}
  \item{blend}{Use blending for model prediction.}
  
  The object \code{c("msc.elnet")} has the following components:
  \item{ms}{The Morse-Smale complex, see \code{\link{msc.nn}} }
  \item{elnet}{The elastic net models and crossvalidation results for each level in ms.}
}
\author{
  Samuel Gerber
}
\seealso{
  \code{\link{msc.nn}},
  \code{\link{predict.msc.lm}},  
  \code{\link{glmnet}}
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
\examples{
#create Morse-Smale complex  regression of fourpeaks2d data set  
data(fourpeaks)
d <- fourpeaks()
#build Morse-Smale complex
ms <- msc.nn.svm(y=d[,1], x=d[, 2:3], pLevel=0.1, knn = 10)
msr <- msc.lm(ms)
#show slected persitence level by cross validtaion
msr$ms$predictLevel
#print mean squared crossvalidated error
msr$lms[[msr$ms$predictLevel]]$cv
#predict 
fp <- predict(msr, d[, 2:3])

#fit an elastic model insteaed
msr <- msc.elnet(ms)
#prediction for ealstic model
fp <- predict(msr, d[, 2:3])
}
\keyword{cluster,multivariate,toplogy,nonparametric,models,nonlinear}
