\name{plot.msc}
\alias{plot.msc}
\alias{plot.msc.svm}
\alias{plot.msc.kd}
\alias{plot.mscPlot}
\title{Visualization of the Morse-Smale Summary of High-Dimensional
  Scalar Functions}
\description{ Visualize the Morse-Smale summary description of a
  high-dimensional scalar function \eqn{ y=f(\mathbf{x}) }{ y = f(x)}
  with parameters \eqn{ \mathbf{x} \in R^n }{x in R^n}.  For each
  partition of the Morse-Smale complex, an inverse regression curve is
  computed that summarizes the domain in that partition.  This forms a
  network of regression curves that connect the extremal points of the
  function. This network is then embedded in 2D for visualization. The
  function value of the regression curves is encoded by color and by
  height in the 3rd dimension for each regression curve.  Optional tubes
  around the regression curves indicate the standard deviation along the
  curve, representing the approximate extent of the partition. An
  additional window plots the regression curve for each parameter in
  \eqn{\mathbf{x}}{x}, which allows to examine the behaviour of each
  partition.  Users can select, by mouse-click on the corresponding
  regression curve, which partitions the plots of the underlying
  parameters will be shown.  In addition, a subset of the parameters
  \eqn{\mathbf{x}}{x} can be selected using \code{mscPlot$plotList}.
}
\usage{
\method{plot}{msc}(x, drawStdDev=FALSE, span=0.5, nsamples=50,
plot=TRUE, colorMap=0, ...)
\method{plot}{msc.kd}(x, drawStdDev=FALSE, span=0.5, nsamples=50,  plot=TRUE, colorMap=0, ...)
\method{plot}{msc.svm}(x, drawStdDev=FALSE, span=0.5, nsamples=50,
plot=TRUE,  colorMap=0, ...)
\method{plot}{mscPlot}(x, drawStdDev=FALSE, axesOn=TRUE, ...)
}
\arguments{
  \item{x}{The Morse-Smale complex object for \code{plot.msc} or the
    \code{mscPlot} object for \code{plot.mscPlot}.}
  \item{drawStdDev}{Draw the standard deviation tubes around the plots
    (default FALSE).}
  \item{axesOn}{Draw the alignment axes (default TRUE).}
  \item{nsamples}{ NUmber of samples for piecewise linear approximation to regression curve in each
    partition}
  \item{span}{Span argument of \code{\link{loess}} for computIng regression
    curves}
  \item{plot}{Show visualization (TRUE) or just return the plotting
    object (FALSE)}
  \item{colorMap}{The choice of colormap.  0 = Blue-Green-Red, 1 =
    Blue-White-Red, 2 = Purple-White-Green}
  \item{...}{additional args have no effect}
}
\value{
  An object of class \code{mscPlot} is used to plot the Morse-Smale
  summary and allows to manipulate the plotting behaviour.
  The object \code{mscPlot} has the following components:
  \item{geom}{which describes the geometry of the summary.}
  \item{scene}{which describes the components of the visualization scene.}
  \item{plotList}{which defines the specific plots to compare. This is a
  list containing the plot numbers.}
}
\author{Kristin Potter and Samuel Gerber}
\seealso{
  \code{\link{msc.nn}},
  \code{\link{msc.nn.svm}},
  \code{\link{msc.nn.kd}}
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

data(fourpeaks)
d <- fourpeaks()

#Build Morse-Smale complex of d
ms <- msc.nn(y=d[,1], x=d[, 2:3], pLevel=0.1, knn = 15)

# Plot the ms summary
p <- plot(ms)

# Change the plots to compare
p$plotList <- c(1)

# Re-plot the summary
plot(p)

}
\keyword{hplot,cluster,multivariate,toplogy,nonparametric,models,nonlinear}
