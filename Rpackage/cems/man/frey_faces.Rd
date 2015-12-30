\name{frey_faces}
\alias{frey_faces}
\alias{faces}
\title{Frey faces}
\description{
Set of 1995 face iamges from a single subject with different facial expression
as well as different orientations. (from
\url{http://www.cs.nyu.edu/~roweis/data.html})
}
\author{
  Samuel Gerber
}
\examples{
data("frey_faces")
im <- matrix(faces[1, 560:1], 20, 28)
image(1:nrow(im), 1:ncol(im), im, xlab="", ylab="")

}
\keyword{datasets}
