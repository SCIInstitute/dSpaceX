\name{fourpeaks}
\alias{fourpeaks}
\title{Fourpeaks Function}
\description{
Fourpeaks is a two-dimensional, additively separable function of four Gaussian
peaks

\eqn{
f(x) = \frac{1}{2} \bigl ( e^{ -(x_1 - \frac{1}{4})^2 / 0.3^2 } +  e^{ -(x_2 -
\frac{1}{4})^2 / 0.3^2} +  e^{ -(x[, 1] - \frac{3}{4})^2 / 0.1^2 } + e^{ -(x_2 -
\frac{3}{4})^2 /0.1^2} \bigr ).
}

On \eqn{[0,1]^2}{[0,1]^2} the function has 4 maxima and 9 minima
}
\value{
 returns N samples form the fourpeaks function
}

\author{
  Samuel Gerber
}
\examples{
data(fourpeaks)
%create 1000 samples
d <- fourpeaks()
%create 2000 samples
d <- fourpeaks(2000)
%rotate domain by 90 degrees
d <- fourpeaks(N=2000, phi=pi/4)
}
\keyword{datasets}
