\name{cem.example.arc}
\alias{cem.example.arc}
\title{Conditional Expectation Manifold Example on Arc}
\description{
This function runs the arc example in:

Samuel Gerber and Ross Whitaker,
Conditional Expectation Curves, 
Submitted 2011.


}


\usage{
cem.example.arc(n=150, noise=0.2, risk=2, sigmaX= 0.1, stepX=0.001,
stepBW=0.01, init = 0, plotEach=1, noiseInit=0.5)
}

\arguments{
  \item{n}{ Sample size.}
  \item{noise}{ Amount of normal distributed noise orthogonal to the arc.}
  \item{risk}{ Optimization objective}
  \item{sigmaX}{Intial bandwidth for the curve \eqn{g}}
  \item{stepBW}{Stepsize for bandwidth optimization}
  \item{stepX}{Stepsize for coordinate optimization}
  \item{init}{Type of initialization. 0 = ground truth, 1 = random, 2 = y-values
              of arc (i.e. close to principal component)}
  \item{plotEach}{Plot curve after \code{plotEach} iterations.}
  \item{noiseInit}{add normal distribution noise to initialization.}
}

\seealso{
  \code{\link{cem}}
}

\author{
  Samuel Gerber
}
\references{
Samuel Gerber, Tolga Tasdizen, Ross Whitaker,
Dimensionality Reduction and Principal Surfaces via Kernel Map Manifolds, 
In Proceedings of the 2009 International Conference on Computer Vison (ICCV 2009).

Samuel Gerber and Ross Whitaker,
Regularization-Free Principal Curve Estimation
Journal of Machine Learning Research 2013.

}

\keyword{nonparametric,models,nonlinear}
