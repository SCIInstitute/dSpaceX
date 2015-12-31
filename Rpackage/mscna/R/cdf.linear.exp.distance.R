cdf.linear.exp.distance <- function(s, h=0, gamma = 1, std = 0.25){
sigma = std/sqrt(2)
p <- 0
fb <- - gamma
r <- -h/gamma
if(h <= fb){
  p <- sigma/ (2*gamma) * ( exp((h+gamma)/sigma) - exp(  (h+gamma*s)/sigma )  )
}
else if( s >= r){
  p <- 1 - sigma/ (2*gamma) * ( exp( -(h+gamma*s)/sigma ) - exp(-(h+gamma)/sigma) )
}
else{
  p <- sigma / (2*gamma) * ( -exp(  (h + gamma*s)/sigma)  +  exp(-( h + gamma)/sigma)  ) + 1 + h/gamma
}
p

}
