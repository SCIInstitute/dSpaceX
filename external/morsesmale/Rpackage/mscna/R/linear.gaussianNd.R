source("cdf.lorder.distance.R")

linear.gaussianNd.sample <- function(gamma = 1, sigma = 0.25, k=100, dim = 2){
d <- 0
for(i in 1:dim){
  d <- d + runif(k)^2
}
d <- sqrt(d) / sqrt(dim)
fx <- -d*gamma + rnorm(n=k, sd = sigma)

cbind(d, fx)
}

linear.gaussianNd.sample.full <- function(gamma = 1, sigma = 0.25, k=100, dim = 2){
d <- matrix(nrow = k, ncol = dim)
for(i in 1:dim){
  d[,i] <- runif(k)/sqrt(dim)
}
fx <- -sqrt(rowSums(d^2))*gamma + rnorm(n=k, sd = sigma)

cbind(d, fx)
}




linear.gaussianNd.lorder <- function(s, gamma = 1, sigma = 0.25, k=100, n=1000, dim = 2){

 nbigger <- 0 
 for(i in 1:n){
   d <- linear.gaussianNd.sample(gamma = gamma, sigma=sigma, k=k, dim = dim)
   if( d[which.max(d[,2]), 1] > s ){
     nbigger <- nbigger + 1
   }
 }
 nbigger/n
}



linear.gaussianNd.lorder.cdf <- function(gamma = 1, sigma = 0.25, k=100, n=1000, dim = 2){

  s <- (0:100)/100
  p <- vector(length = 101)
  p[] = 0
  print(k)
  for(i in 1:n){
    d <- linear.gaussianNd.sample(gamma = gamma, sigma=sigma, k=k, dim = dim)
    l <- d[which.max(d[,2]), 1]
    p[s < l] <- p[ s < l] + 1
  }

  p <- p / n
  d <- cbind(s, p)
}



linear.gaussianNd.expected.persistence <- function(gamma, sigma, dim, n=10000, k=100){
  p <- 0
  for(i in 1:n){
    d <-linear.gaussianNd.sample(gamma=gamma, sigma=sigma, k=k, dim = dim)
    p <- p + max(d[,2]) - min(d[,2])
  }
  p <- p/n
  p
}





linear.gaussianNd.squeezek.all <- function(gamma = 1, sigma = 0.25, dim = 2, k=100, n=100000, knn=5*dim){
  library(msr)
  s <- 2*(0:100)/100
  p <- vector(length = 101)
  p[] = 0
  for(i in 1:n){
    d <- linear.gaussianNd.sample.full(gamma = gamma, sigma=sigma, k=k, dim=dim)
    ms <- msc.nn(y = d[, dim+1], x = d[, 1:dim], knn=knn, pLevel = 0);
    if(length(ms$persistence )> 1){
      l <- ms$persistence[length(ms$persistence)-1]
      p[s<l] <- p[s<l] + 1
    }
  } 
  p <- p / n
  d <- cbind(s, p)
}




#create squeeze k observation plot 
linear.gaussianNd.squeezek.plots <- function(file, n=100000, k=100, gamma=1, sigma=0.25){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 2, k=k)
  plot(d[, 1], d[, 2], xlab="Persistence", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 4, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 16, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "green")
  legend("topright", c("d = 2", "d = 4", "d = 8", "d = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)

  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 2, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "black")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 4, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "red")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "blue")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 16, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "green")

  lines(c(1,1), c(0.1,0.4), lwd=2, lty="dashed", col = "gray")
  dev.off()

}

#create largest order statistic plots
linear.gaussianNd.lorder.plots <- function(file, n=10000, k=100){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = 2, k=k)
  plot(d[, 1], d[, 2], xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = 4, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = 8, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = 16, k=k)
  lines(d[, 1], d[, 2], lwd=3, col = "green")
  legend("topright", c("d = 2", "d = 4", "d = 8", "d = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)

  dev.off()

}

#create largest order statistic plots
linear.gaussianNd.lorder.plots.samplesize <- function(file, n=10000, dim=8){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = dim, k=50)
  plot(d[, 1], d[, 2], xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = dim, k=100)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = dim, k=500)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  d <- linear.gaussianNd.lorder.cdf(gamma = 1, sigma=0.25, n=n, dim = dim, k=1000)
  lines(d[, 1], d[, 2], lwd=3, col = "green")
  legend("topright", c("n = 50", "n = 100", "n = 500", "n = 1000"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)

  dev.off()

}


linear.gaussianNd.squeezek.plots.knn <- function(file, n=10000, k=500, gamma=1, sigma=0.25){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=k, knn=15)
  plot(d[, 1], d[, 2], xlab="Persistence", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=k, knn=25)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=k, knn=50)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=k, knn=100)
  lines(d[, 1], d[, 2], lwd=3, col = "green")
  legend("topright", c("knn = 15", "knn = 25", "knn = 50", "knn = 100"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)

  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "black")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "red")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "blue")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=k)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "green")

  lines(c(1,1), c(0.1,0.4), lwd=2, lty="dashed", col = "gray")
  dev.off()

}

linear.gaussianNd.squeezek.plots.samplesize <- function(file, n=100000, gamma=1, sigma=0.25){

  postscript( file , width = 6, height = 5, paper="special"  ) 

  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=1000)
  plot(d[, 1], d[, 2], xlab="Persistence", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "green")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=50)
  lines(d[, 1], d[, 2], lwd=3, col = "black")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=100)
  lines(d[, 1], d[, 2], lwd=3, col = "red")
  d <- linear.gaussianNd.squeezek.all(gamma = gamma, sigma=sigma, n=n, dim = 8, k=500)
  lines(d[, 1], d[, 2], lwd=3, col = "blue")
  legend("topright", c("n = 50", "n = 100", "n = 500", "n = 1000"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)

  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=50)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "black")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=100)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "red")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=500)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "blue")
  e <- linear.gaussianNd.expected.persistence(gamma = gamma, sigma=sigma, dim = 8, n=n, k=1000)
  lines(c(e,e), c(0.1,0.4), lwd=2, lty="dashed", col = "green")

  lines(c(1,1), c(0.1,0.4), lwd=2, lty="dashed", col = "gray")
  dev.off()

}
