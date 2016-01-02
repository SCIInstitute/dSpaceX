\name{swissroll}
\alias{swissroll}
\title{Fourpeaks Function}
\description{
Swissroll data set.
}
\author{
  Samuel Gerber
}
\examples{
library(rgl)
data(swissroll)
#create 1000 samples with standard parameters
d <- swissroll()

#X contains orginal data
plot3d(d$X)
#Xn conatins data with gaussina noise added orthogoally
plot3d(d$Xn)

#create 2000 samples with different parameters
#phi - number of revolutions
#nstd - std of normal noise added orthogonally
d <- swissroll(2000, nstd = 0.5, height = 5, phi = 2*pi)
plot3d(d$Xn)
}
\keyword{datasets}
