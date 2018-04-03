\name{diagonal}
\alias{diagonal}
\alias{diagonalToplogy}
\title{Diagonal Function}
\description{
Diagonal Function, cosine along the diagonal of a d-dimensional hypercube
with exponential enevlope orthogonal to diagonal.

\eqn{
f(x) = \frac{1}{2} \cos( \frac{ \sqrt(<x, v>) }{\sqrt(d)p\pi} )
\exp( \frac{ || x ||^2  -  <x, v> }{d} )
}
with  \eqn{v = \frac{\mathbf{1}}{\sqrt(d)} }the unit length digonal vector .


}
\value{
 returns N samples form the diagonal function
}

\author{
  Samuel Gerber
}
\examples{
data(diagonal)
%create 1000 samples
d <- diagonal()
%create 2000 samples in 3 dimensions with period 4
d <- diagonal(d = 3, p = 4, N=2000)

}
\keyword{datasets}
