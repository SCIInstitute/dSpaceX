#R^2 based merging criterion
cemr <- function (y, z, l, lambda, knnY = 15, knnX=15, iter = 100, step = 0.1, fudge = 2.5, verbose=1) 
{
    this.call <- match.call()
  
    if(is.null(nrow(y))){
      y <- as.matrix(y, ncol=1)
    }
    else{ 
      y <- as.matrix(y)
    }    
    if(is.null(nrow(z))){
      z <- as.matrix(z, ncol=1)
    }
    else{ 
      z <- as.matrix(z)
    }    
    
    if(is.null(nrow(l))){
      l <- as.matrix(l, ncol=1)
    }
    else{ 
      l <- as.matrix(l)
    }


    nry <- nrow(y)
    nrz <- nrow(z)
    nrl <- nrow(l)
    if(nry != nry & nry != nrl){
      stop("z and y don't have the same number of observations") 
    }
    ncy <- ncol(y)
    ncz <- ncol(z)
    ncl <- ncol(l)
    res <- .Call("cemr_create", as.double(t(y)), as.double(t(z)),
        as.double(t(l)), nry, ncy, ncz, ncl,   as.integer(knnY),  as.integer(knnX),
        as.double(fudge), as.double(lambda), as.integer(iter), as.double(step),
        as.integer(verbose) )  
   
    obj <- structure( list( y=y, z=t(as.matrix(res[[1]])), l=l, knnY= knnY, knnX = knnX,
                            sigma=res[[2]], kT = res[[5]], kM = res[[4]], kV =
                            res[[3]] ), class="cem") 
 
    obj

}


#Optimize existing CEM further
cemr.optimize <- function(object, iter = 50, step=0.1, verbose=1){

  y <- object$y
  z <- object$z
  l <- object$l
  knnX <- object$knnX
  knnY <- object$knnY
  sigma <- object$sigma
  lambda <- object$lambda
  nry <- nrow(y)
  nrz <- nrow(z)
  ncy <- ncol(y)
  ncz <- ncol(z)    
  ncl <- ncol(l)
  res <- .Call("cemr_optimize", as.double(t(y)), as.double(t(z)),
      as.double(t(l)),  nry, ncy, ncz, ncl, as.integer(knnY), as.integer(knnX),
      as.double(sigma), as.double(lambda),  as.integer(iter), as.double(step),
      as.integer(verbose), as.double(object$kT),
      as.double(object$kV), as.double(object$kM) ) 
    
  object$z = t(as.matrix(res[[1]]))
  object$kT = as.matrix(res[[5]])
  object$kM = as.matrix(res[[4]])
  object$kV = as.matrix(res[[3]])
  
  object

}





predict.cemr <- function(object, newdata = object$y, ...){

  if( is.null( nrow(newdata) ) ){
    data <- as.matrix(newdata, ncol=1)
  }
  else{
    data <- as.matrix(newdata)
  }
  y <- object$y
  z <- object$z
  knnX <- object$knnX
  knnY <- object$knnY
  sigma <- object$sigma
  nry <- as.integer(nrow(y))
  nrz <- as.integer(nrow(z))
  ncy <- as.integer(ncol(y))
  ncz <- as.integer(ncol(z))
  nrd <- as.integer(nrow(data))
  if(ncol(data)  == ncol(object$y) ){
    res <- .Call("cem_parametrize", as.double(t(data)), nrd, as.double(t(y)),
        t(z), nry, ncy, ncz, as.integer(knnY), as.integer(knnX),
        as.double(sigma), as.double(object$kT), as.double(object$kV),
        as.double(object$kM) )  
    res <- t(as.matrix(res))
  }
  else{    
    res <- .Call("cem_reconstruct", as.double(t(data)), nrd, as.double(t(y)),
        t(z), nry, ncy, ncz, as.integer(knnY), as.integer(knnX),
        as.double(sigma), as.double(object$kT), as.double(object$kV),
        as.double(object$kM) )  
    T = c()
    for(i in 1:ncz){
      T[[i]] = t(as.matrix(res[[1+i]]))
    }
    res <- list(y = t(as.matrix(res[[1]])), tangents=T)
  }
  res
}
