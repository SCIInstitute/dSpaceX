pdf.linear.exp <- function(h, gamma = 1, std = 0.25){

sigma <- std/sqrt(2)
p <- 0
fb <-  - gamma
if(h <= fb){
 p <- 1/ (2*gamma) * ( exp((h+gamma)/sigma) - exp( h/sigma)  )
}
else if( h >= 0){
  p <- 1/ (2*gamma) * ( exp( - h/sigma) -  exp(-(h+gamma)/sigma) )

}
else{
 p <- 1/ (2*gamma) * ( -exp(-(h+gamma)/sigma) - exp( h/sigma)  ) +1/gamma
}
p

}
