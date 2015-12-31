source("cdf.lorder.plots.R")

sample.gaussian <- function(gamma = 1, sigma = 0.25, k=100){

x <- runif(k)
fx <- -x*gamma + rnorm(n=k, sd = sigma)

cbind(x, fx)
}


lorder.gaussian <- function(s, gamma = 1, sigma = 0.25, k=100, n=1000){

 nbigger <- 0 
 for(i in 1:n){
   d <- sample.gaussian(gamma = gamma, sigma=sigma, k=k)
   if( d[which.max(d[,2]), 1] > s ){
     nbigger <- nbigger + 1
   }
 }
 nbigger/n
}

cdf.lorder.gaussian <- function(gamma = 1, sigma = 0.25, k=100, n=1000){

s <- c()
p <- c()
for(i in 1:101){ 
 s[i] <- (i-1)/100; 
 p[i] <- lorder.gaussian(s = s[i], gamma = gamma, sigma = sigma, k=k, n=n)
}
d <- cbind(s, p)

}

squeeze.gaussian <- function(a, gamma = 1, sigma = 0.25, n=1000){
 nsqueeze <- 0 
 for(i in 1:n){
   d <- sample.gaussian(gamma = gamma, sigma=sigma, k=3)
   o <- order(d[,1])
   if( (min(d[o[1], 2], d[o[3], 2]) - d[o[2], 2] ) > a){
     nsqueeze <- nsqueeze+1
   }
 }
 nsqueeze/n
}


squeeze.gaussian.all <- function(gamma = 1, sigma = 0.25, n=1000){

  s <- 2*(0:100)/100
  p <- vector(length = 101)
  p[] = 0
  for(i in 1:n){
    d <- sample.gaussian(gamma = gamma, sigma=sigma, k=3)
    o <- order(d[,1])
    for(i in 1:101){ 
      if( (min(d[o[1], 2], d[o[3], 2]) - d[o[2], 2] ) > s[i]){
        p[i] <- p[i] + 1
      }
      else{
        break
      }
    }
  }
  p <- p / n 
  d <- cbind(s, p)
}

linear.gaussian.squeezek.all <- function(gamma = 1, sigma = 0.25, k=100, n=100000){

  s <- 2.6*(0:200)/200
  p <- vector(length = 201)
  p[] = 0
  for(i in 1:n){
    d <- sample.gaussian(gamma = gamma, sigma=sigma, k=k)
    o <- order(d[,1])
    dmin1 <- d[o[1], 2] 
    dmax1 <- d[o[1], 2]
    dmin2 <- NA
    dmax2 <- NA
    maxs <- -Inf
    mins <- Inf
    
    diffMax <- 0
    diffMin <- 0
    for(j in o){
      cf <- d[j,2]
     
      if(maxs < cf){
        maxs <- cf
      }
      if(mins > cf){
        mins <- cf
      }
      
      if(dmin1 >  cf){
        if(!is.na(dmin2)){
          diffMin <- maxs - dmin1
        }
        maxs <- -Inf
        dmin2 <- NA
        dmin1 <- cf
      }
      else if(is.na(dmin2) | dmin2 > cf){
       dmin2 <-  cf
       diffMin <- maxs - dmin2
      }


      if(dmax1 <  cf){
        if(!is.na(dmax2)){
          diffMax <- dmax1 - mins
        }
        mins <- Inf
        dmax2 <- NA
        dmax1 <- cf
      }
      else if(is.na(dmax2) | dmax2 < cf){
       dmax2 <-  cf
       diffMax <- dmax2 - mins
      }

    }
    l <- max(diffMin, diffMax)
    p[s<l] <- p[s<l] + 1
  }
  p <- p / n 
  d <- cbind(s, p)
}

linear.gaussian.expected.persistence <- function(gamma, sigma, n=10000, k=100){
  p <- 0
  for(i in 1:n){
    d <-sample.gaussian(gamma=gamma, sigma=sigma, k=k)
    p <- p + max(d[,2]) - min(d[,2])
  }
  p <- p/n
  p
}


linear.gaussian.lorder.plots.ratios <- function(file, k=50){


postscript( file, width = 6, height = 5, paper="special"   ) 
d <- cdf.lorder.gaussian(gamma = 0.5, sigma=0.25, n=10000, k=k)
plot(d[, 1], d[, 2], xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black", lty = "dashed")
d <- cdf.lorder.gaussian(gamma = 1, sigma=0.25, n=10000, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "red", lty = "dashed")
d <- cdf.lorder.gaussian(gamma = 2, sigma=0.25, n=10000, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "blue", lty = "dashed")
d <- cdf.lorder.gaussian(gamma = 4, sigma=0.25, n=10000, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "green", lty = "dashed")
 legend("topright", c("r = 2", "r = 4", "r = 8", "r = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)


d <- cdf.lorder.distance.all(gamma = 0.5, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "black")
d <- cdf.lorder.distance.all(gamma = 1, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "red")
d <- cdf.lorder.distance.all(gamma = 2, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "blue")
d <- cdf.lorder.distance.all(gamma = 4, k=k)
lines(d[, 1], d[, 2], lwd=3, col = "green")

dev.off()

}




linear.gaussian.lorder.plots.samplesize <- function(file, n= 10000){


postscript( file, width = 6, height = 5, paper="special"   ) 
d <- cdf.lorder.gaussian(gamma = 2, sigma=0.25, n=n, k=10)
plot(d[, 1], d[, 2], xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black", lty = "dashed")
d <- cdf.lorder.gaussian(gamma = 2, sigma=0.25, n=n, k=20)
lines(d[, 1], d[, 2], lwd=3, col = "red", lty = "dashed")
d <- cdf.lorder.gaussian(gamma = 2, sigma=0.25, n=n, k=100)
lines(d[, 1], d[, 2], lwd=3, col = "blue", lty = "dashed")
d <- cdf.lorder.gaussian(gamma = 2, sigma=0.25, n=n, k=200)
lines(d[, 1], d[, 2], lwd=3, col = "green", lty = "dashed")
 legend("topright", c("n = 10", "n = 20", "n = 100", "n = 200"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)


d <- cdf.lorder.distance.all(gamma = 2, std = 0.25, k=10)
lines(d[, 1], d[, 2], lwd=3, col = "black")
d <- cdf.lorder.distance.all(gamma = 2, std = 0.25, k=20)
lines(d[, 1], d[, 2], lwd=3, col = "red")
d <- cdf.lorder.distance.all(gamma = 2, std = 0.25, k=100)
lines(d[, 1], d[, 2], lwd=3, col = "blue")
d <- cdf.lorder.distance.all(gamma = 2, std = 0.25, k=200)
lines(d[, 1], d[, 2], lwd=3, col = "green")

dev.off()

}





linear.gaussian.squeezek.plots <- function(file, n=10000, k=50){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussian.squeezek.all(gamma = 0.5, sigma=0.25, n=n, k=k)
  plot(d[, 1], d[, 2], xlab="Persistence", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
  d <- linear.gaussian.squeezek.all(gamma = 1, sigma=0.25, n=n, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussian.squeezek.all(gamma = 2, sigma=0.25, n=n, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  d <- linear.gaussian.squeezek.all(gamma = 4, sigma=0.25, n=n, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "green")
  legend("topright", c("r = 2", "r = 4", "r = 8", "r = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)


  e <- linear.gaussian.expected.persistence(gamma = 0.5, sigma=0.25, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "black")
  e <- linear.gaussian.expected.persistence(gamma = 1, sigma=0.25, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "red")
  e <- linear.gaussian.expected.persistence(gamma = 2, sigma=0.25, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "blue")
  e <- linear.gaussian.expected.persistence(gamma = 4, sigma=0.25, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "green")


  lines(c(0.5,0.5), c(0.1,0.4), lwd=1, lty="dashed", col = "black")
  lines(c(1,1), c(0.1,0.4), lwd=1, lty="dashed", col = "red")
  lines(c(2,2), c(0.1,0.4), lwd=1, lty="dashed", col = "blue")
  lines(c(4,4), c(0.1,0.4), lwd=1, lty="dashed", col = "green")

  dev.off()

}


linear.gaussian.squeezek.plots.k <- function(file, n=10000){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussian.squeezek.all(gamma = 2, sigma=0.25, n=n, k=200)
  plot(d[, 1], d[, 2], xlab="Persistence", ylab= "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "green")
  d <- linear.gaussian.squeezek.all(gamma = 2, sigma=0.25, n=n, k=10)
  lines(d[, 1], d[, 2], lwd=3, col = "black")
  d <- linear.gaussian.squeezek.all(gamma = 2, sigma=0.25, n=n, k=20)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussian.squeezek.all(gamma = 2, sigma=0.25, n=n, k=100)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  legend("topright", c("n = 10", "n = 20", "n = 100", "n = 200"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)


  e <- linear.gaussian.expected.persistence(gamma = 2, sigma=0.25, n=n, k=10)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "black")
  e <- linear.gaussian.expected.persistence(gamma = 2, sigma=0.25, n=n, k=20)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "red")
  e <- linear.gaussian.expected.persistence(gamma = 2, sigma=0.25, n=n, k=100)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "blue")
  e <- linear.gaussian.expected.persistence(gamma = 2, sigma=0.25, n=n, k=200)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "green")


  lines(c(2,2), c(0.1,0.4), lwd=1, lty="dashed", col = "gray")

  dev.off()

}


#create squeeze k observation plot 
linear.gaussian.squeezek.plots.sigma <- function(file, n=10000, k=50){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussian.squeezek.all(gamma = 1, sigma=0.05, n=n, k=k)
  plot(d[, 1], d[, 2], xlab="Persistence", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
  d <- linear.gaussian.squeezek.all(gamma = 1, sigma=0.1, n=n, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussian.squeezek.all(gamma = 1, sigma=0.15, n=n, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  d <- linear.gaussian.squeezek.all(gamma = 1, sigma=0.2, n=n, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "green")
  legend("topright", c("sigma = 0.05", "sigma = 0.1", "sigma = 0.15", "sigma = 0.2"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)


  e <- linear.gaussian.expected.persistence(gamma = 1, sigma=0.05, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "black")
  e <- linear.gaussian.expected.persistence(gamma = 1, sigma=0.1, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "red")
  e <- linear.gaussian.expected.persistence(gamma = 1, sigma=0.15, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "blue")
  e <- linear.gaussian.expected.persistence(gamma = 1, sigma=0.2, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dotted", col = "green")


  lines(c(1,1), c(0.1,0.4), lwd=1, lty="dashed", col = "gray")

  dev.off()

}
