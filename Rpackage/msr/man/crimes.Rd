\name{uci_crime_subset}
\docType{data}
\alias{crimes}
\title{UCI coummunity and crimes subset}
\description{
Subset of the UCI communities and crime data
set~\url{http://archive.ics.uci.edu/ml/datasets/Communities+and+Crime}. The
data set contains 100 variables, with some of the original values with many
missing values removed.

From the UCI website:\\
Communities within the United States. The data combines socio-economic data
from the 1990 US Census, law enforcement data from the 1990 US LEMAS survey,
and crime data from the 1995 FBI UCR. 

More detail on the individual variables can be found on the website. 
}

\usage{
crimes
}

\author{
  Samuel Gerber
}
\examples{
data(uci_crime_subset)
summary(crimes)
}
\keyword{datasets}
