diagonal <- function(d=2, p=1, N=1000, inX = c()){


if( length(inX)>0 )
 x=inX
else
 x <- matrix(runif(d*N), ncol=d, nrow=N)

dl <- sqrt(d)

v <- matrix(1/dl, nrow=d, ncol=1)

ld <- x %*% v
l <- sqrt(rowSums(x^2))
lo <- sqrt(l^2 - ld^2)

sigma <- dl/2

y <- -0.5*cos(ld/dl*p*pi) * exp(-(lo^2/ sigma^2) ) 

df <- data.frame(y=y, x=x)
}


