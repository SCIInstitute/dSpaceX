source("cdf.lorder.distance.R")




cdf.lorder.distance.all <- function(gamma, std=0.25, k = 100){
s <- c()
p <- c()
for(i in 1:41){ 
 s[i] <- 0 + (i-1)/40; 
 p[i] <- cdf.lorder.distance(s = s[i], gamma = gamma, std = std, k=k)
}
d <- cbind(s, p)
}


cdf.lorder.plots <- function(file){
postscript(file, width = 6, height = 5, paper="special"  ) 
d <- cdf.lorder.distance.all(gamma = 0.5)
plot(d[, 1], d[, 2], xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
d <- cdf.lorder.distance.all(gamma = 1)
lines(d[, 1], d[, 2], lwd=3, col = "red")
d <- cdf.lorder.distance.all(gamma = 2)
lines(d[, 1], d[, 2], lwd=3, col = "blue")
d <- cdf.lorder.distance.all(gamma = 4)
lines(d[, 1], d[, 2], lwd=3, col = "green")
 legend("topright", c("r = 2", "r = 4", "r = 8", "r = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)
dev.off()
}
