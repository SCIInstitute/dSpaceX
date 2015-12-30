\name{cem}
\alias{cem}
\alias{cems}
\alias{cem.geodesic}
\alias{cems-package}
\alias{cem.optimize}
\alias{predict.cem}
\title{Conditional Expectation Manifolds}
\description{
This package computes principal surfaces based on the approach described in
Gerber et. al. 2009 and Gerber and Whitaker 2011. 

Prinicpal surfaces are typically found by minimizing \eqn{ E[ || Y
-g(\lambda(Y))||^2 ] } over the functions \eqn{g:R^m \mapsto R^n}{g:R^m -> R^n}
with \eqn{m < n} {m < n} and \eqn{ \lambda_g:R^n \mapsto R^m}{\lambda_g:R^n ->
R^m} defined as an orthogonal projection onto \eqn{g}{g}. 

In Gerber et. al. 2009 the oppoiste approach is described; fixing
\eqn{g_{\lambda}(x) = E[Y | \lambda(Y) = x] }{ g_\lambda(x) = E[Y | \lambda(Y) =
x } 
and minimzing over \eqn{\lambda}{\lambda}, i.e. optimizing the conditonal
expectation manifold (CEM) given \eqn{\lambda}{\lambda}. Gerber et. al. 2009
called this approach kernel map manifolds (KMM) since both mappings where defined
by kernel regression.

In Gerber and Whitaker 2011 the same formulation is exploited to provide an
approach that solves the model selection problem for principal curves.  The
orthogonal projection distance minimization
\eqn{E[ ||Y - g_\lambda(Y)||^2]}
yields principal curves that are saddle points and thus model selection (i.e.
bandwidth slection in this implementation) is not possible even with
cross-validation.  The approach in Gerber and Whitaker 2011 formulates an
alternate optimization problem minimzing orthogonality
\eqn{E[ < Y - g_\lambda(Y), \frac{d}{ds} g(s)|_{s=|lambda(Y)}>^2 ] }
which leads to principal curves at minima.

This package implements the approach in Gerber et. al. 2009 for both
formulation, i.e. minimzing  projection distance
\eqn{E[ ||Y - g_\lambda(Y)||^2]} or orthogonality
\eqn{E[ < Y - g_\lambda(Y), \frac{d}{ds} g(s)|_{s=|lambda(Y)}>^2 ] }.
. The implementation is based on a kernel regression for \eqn{\lambda} and
\eqn{g} and uses a numerical gradient descent for minimization. The gradient
descent includes an optimization of the bandwidth, i.e. model selection. For
minimzing the projection distance this dopes not lead to good results since
principal curves are saddle points. Thus \code{stepBW} should be set to 0 in
this case.


}


\usage{
cem( y, x,  knnX=50, sigmaX= 1/3, iter =100, nPoints =
    nrow(y), stepX = 0.25, stepBW = 0.1, verbose=1, risk=2, penalty=0,
    sigmaAsFactor = T, optimalSigmaX =  F , quadratic = F)
cem.optimize(object, iter = 100, nPoints = nrow(object$y), stepX=1, stepBW=0.1,
    verbose=1, optimalSigmaX =F ) 
\method{predict}{cem}(object, newdata = object$y, type=c("coordinates",
      "curvature"), ...)
cem.geodesic(object, xs, xe, iter = 100, step = 0.01,
    verbose=1, ns=100)
}

\arguments{
  \item{y}{\eqn{n}{n}-dimensional data to compute conditional expectation
manifold for.}
  \item{x}{Initalization for low dimensional mapping \eqn{\lambda}{\lambda}. For
example an isomap or lle or PCA embedding of \eqn{ly}{ly}.}
  \item{knnX}{Number of nearest neighbors for kernel regression of \eqn{g}, i.e.
the regression is trunctaed to only the knnX nearest neighbors}    
 \item{sigmaX}{Initalize bandwidth of \eqn{g} to sigmaX. If \code{sigamAsFactor}
is set to true the bandiwdth is computed as sigmaX times average knnX nearest
neighbor distance.}  
  \item{iter}{Number of optimization iterations, i.e. number of gradient desecent with
line search setps.}
  \item{stepX}{Gradient descent step size for optimizing coordinate mapping}
  \item{stepBW}{Gradient descent step size for optimizing bandwidths}
  \item{verbose}{Report gradient descent information. 1 reports iteration number
and mean squared projetcion distances. 2 has additonal information on step size
and line search.}
  \item{sigmaAsFactor}{Use sigmaX and sigmaY as multipliers of the average
nearest neighbor distances in \eqn{Y} and \eqn{\lambda(Y)}. respectively.}
  \item{optimalSigmaX}{If true optimizes sigmaX before every iteration - will
not work for MSE minimzation, i.e. sigmaX will go to 0- owrk well for orthogonal
projection and speeds up computation significantly}
  \item{risk}{Which objective function should be minimized. 0 = \eqn{E[ ||Y -
g_\lambda(Y)||^2]}. 1 = \eqn{E[<g(f(y)) - y , g'(f(y))>^2]}. 2 = 1 but with
\eqn{g'(f(y))>} ortho normalized. 3=2 with \eqn{g(f(y)) - y } normalized.}
  \item{penalty}{  0 = No penalty, 1 = Deviation from arc length parametrization}
  \item{quadratic}{ Use a locally quadratic regression instead of linear for
    \eqn{g}{g} }
  \item{nPoints}{ Number of points that are sampled for computing gradient descent
    directions}
  \item{object}{CEM object to do prediction for}
  \item{newdata}{Data to do prediction for. If \code{ncol(newdata) == m} for each
point x in newdata g(x) is computed. If \code{col{newdata} == n} for each point
y in newdata \eqn{\lambda(y)}{\lambda(y)} is computed.}
  \item{type}{Prediction type: coorindates or curvatures of the manifold model}
  \item{...}{Additional arguments have no effect.}
  \item{xs}{Start point for geodesic}
  \item{xe}{End point for geodesic}
  \item{step}{Step size for optimizing geoesic}
  \item{ns}{Number of segments for dicretizing geodesic}
  
}

\value{
  An object of class \code{"cem"}.
}

\author{
  Samuel Gerber
}
\seealso{
  \code{\link{cem.example.arc}}
  \code{\link{cem.example.sr}}
}
\references{
Samuel Gerber, Tolga Tasdizen, Ross Whitaker,
Dimensionality Reduction and Principal Surfaces via Kernel Map Manifolds, 
In Proceedings of the 2009 International Conference on Computer Vison (ICCV 2009).

Samuel Gerber and Ross Whitaker,
Regularization-Free Principal Curve Estimation
Journal of Machine Learning Research 2013.
}
\examples{

##Noisy half circle example
phi <- runif(100)*pi
arc <- cbind(cos(phi), sin(phi)) * (1+rnorm(100) * 0.1)

pc <-  cem(y=arc, x=phi, knnX=10, iter=10, optimalSigmaX=TRUE, verbose=2)

#predict original data
y <- predict(pc, pc$x);

#predict new data
xt <- seq(min(pc$x), max(pc$x), length.out=100)
yt <- predict(pc, xt)

#plot things
arc0 <- cbind(cos(phi), sin(phi))
o0 <- order(phi) 

par(mar=c(5,5,4,2))
plot(arc, xlab=expression(y[1]), ylab=expression(y[2]), col = "#00000020",
     pch=19, asp=1, cex.lab=1.5, cex.axis=1.5, cex=2, bty="n")

lines(arc0[o0,], lwd=4, col="black", lty=6)
lines(yt$y, col="dodgerblue", lwd=4, lty=1)


}
\keyword{nonparametric,models,nonlinear}
