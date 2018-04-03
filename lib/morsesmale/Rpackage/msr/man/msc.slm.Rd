\name{msc.slm}
\alias{msc.slm}
\alias{msc.slm.elnet}
\title{Morse Smale Complex Simultaneous Linear Regression}
\description{
Fit a simoultaneous linear model using the Morse-Smale decomposition of
the domain. For each crystal a new varibale is introduced, each observation for
the variables is weighted by the weight of belonging to that crystal. The weights are
computed  depening on the underlying Morse-Samle complex type (see \code{\link{msc.nn}}).
}
\usage{
msc.slm(ms, nfold = 10, modelSelect = FALSE)
msc.slm.elnet(ms, nfold = 10)
}
\arguments{  
  \item{ms}{A Morse-Smale complex object, see \code{\link{msc.nn}}}.  
  \item{nfold}{Number of folds for crossvlaidation, used for selecting an
    appropriate persitence level if the underlying Morse-Smale complex objects
    has multiple levels.}
  \item{modelSelect}{Do a forward stepwise model selection for each linear model 
    (for each parttion ther eis on linear model)}

}
\value{
  An object of class \code{c("msc.slm")}, that can be used for prediction with
\code{\link{predict}}. 

  The object has the following components:
  \item{ms}{The Morse-Smale complex, see \code{\link{msc.nn.kd}} }
  \item{slm}{The linear model based on the weighted observation and variables
    for each crystals.}


}
\author{
  Samuel Gerber
}
\seealso{
  \code{\link{predict.msc.slm}}
  \code{\link{msc.nn}},  
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
#build model using Morse-Smale decomposition ms
msr <- msc.slm(ms)
#print simultaneous linear model cv error
msr$slm[[msr$ms$predictLevel]]$cv
#predict for all data points
fp <- predict(msr, d[, 2:3])

#use elastic net for fitting instead
msr <- msc.slm.elnet(ms)
fp <- predict(msr, d[, 2:3])

}
\keyword{cluster,multivariate,toplogy,nonparametric,models,nonlinear}
