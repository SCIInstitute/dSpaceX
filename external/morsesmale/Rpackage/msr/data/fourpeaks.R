fourpeaks <- function(N=1000, phi = 0, inX = c()){
dim <- 2

if( length(inX)>0 )
 x=inX
else
 x <- matrix(runif(dim*N), ncol=dim, nrow=N)


y <- 0.5 * exp( -(x[, 1] - 0.25)^2 / 0.3^2 ) + 0.5 * exp( -(x[,2] - 0.25)^2 / 0.3^2) + 0.5 * exp( -(x[, 1] - 0.75)^2 / 0.1^2 ) + 0.5 * exp( -(x[,2] - 0.75)^2 /0.1^2) 
R = matrix(ncol=2, nrow=2)
R[1,1] <- cos(phi)
R[1,2] <- sin(phi)
R[2,1] <- -sin(phi)
R[2,2] <- cos(phi)

x <- x %*% R


df <- data.frame(y=y, x=as.matrix(x))
}
