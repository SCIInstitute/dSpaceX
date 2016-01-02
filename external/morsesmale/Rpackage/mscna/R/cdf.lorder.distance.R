source("cdf.linear.exp.distance.R")
source("cdf.linear.exp.R")
source("pdf.linear.exp.R")

cdf.lorder.distance <- function(s, gamma = 1, std = 0.25, n = 1000, low = -10, high  = 11, k = 100){

h <- 0:n
h <- low + h*(high-low)/n
p <- 0
for(q in h){
  cdf <- cdf.linear.exp(h = q, gamma = gamma, std = std)
  pdf <- pdf.linear.exp(h = q, gamma = gamma, std = std)
  cdfd <- cdf.linear.exp.distance(s = s, h = q, gamma=gamma, std = std)
  if(cdf < 1){
    p <- p + (1-cdfd)/(1-cdf) * cdf^(k-2)* (1-cdf)*pdf  
  }
}

p <- p *(k^2-k) * (high-low)/n
p
}
