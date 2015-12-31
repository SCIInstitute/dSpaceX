source("cdf.linear.exp.distance.R")

cdf.distance.all <- function(gamma, std = 0.25){
s <- c()
p <- c()
for(i in 1:101){ 
 s[i] <- 0 + (i-1)/100; 
 p[i] <- cdf.linear.exp.distance(s = s[i], gamma = gamma, std = std)
}
d <- cbind(s, p)
}

cdf.distance.plot <- function(){
path <- "~/research/papers/msregression/figures/"
postscript(paste(path, "msnoise_1sample2.ps", sep=""), width = 6, height = 5, paper="special"   ) 
d <- cdf.distance.all(gamma = 0.5, std=0.25)
plot(d[, 1], 1-d[, 2], xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
d <- cdf.distance.all(gamma = 1, std=0.25)
lines(d[, 1], 1-d[, 2], lwd=3, col = "red")
d <- cdf.distance.all(gamma = 2, std=0.25)
lines(d[, 1], 1-d[, 2], lwd=3, col = "blue")
d <- cdf.distance.all(gamma = 4, std=0.25)
lines(d[, 1], 1-d[, 2], lwd=3, col = "green")
 legend("topright", c("r = 2", "r = 4", "r = 8", "r = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)
dev.off()

postscript(paste(path,"msnoise_100samples2.ps", sep=""), width = 6, height = 5, paper="special"  ) 
d <- cdf.distance.all(gamma = 0.5, std=0.25)
plot(d[, 1], d[, 2]^100, xlab="Distance", ylab = "Probability", lwd=3, type="l", cex.lab =1.5, cex.axis = 1.5, col = "black")
d <- cdf.distance.all(gamma = 1, std=0.25)
lines(d[, 1], d[, 2]^100, lwd=3, col = "red")
d <- cdf.distance.all(gamma = 2, std=0.25)
lines(d[, 1], d[, 2]^100, lwd=3, col = "blue")
d <- cdf.distance.all(gamma = 4, std=0.25)
lines(d[, 1], d[, 2]^100, lwd=3, col = "green")
 legend("bottomright", c("r = 2", "r = 4", "r = 8", "r = 16"), col = c("black", "red", "blue", "green"), lty = c(1,1,1), lwd=3, cex=1.7)
dev.off()
}


