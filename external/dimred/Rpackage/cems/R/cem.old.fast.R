


cem <- function (y, lz, ly, knnX = 100, knnY = 100, sigmaZ=  1/3, sigmaY=1/3,
    sigmaX=1/3, iter = 100, nPoints = nrow(y), stepZ = 0.25, stepBW = 0.1,
    verbose=1, risk=2, penalty = 0, sigmaAsFactor=T, optimalSigmaX = F,
    quadratic=F ) 
{
    this.call <- match.call()
    if(is.null(nrow(y))){
      y <- as.matrix(y, ncol=1)
    }
    else{ 
      y <- as.matrix(y)
    }    
    if(is.null(nrow(lz))){
      lz <- as.matrix(lz, ncol=1)
    }
    else{ 
      lz <- as.matrix(lz)
    }    
    if(is.null(nrow(ly))){
      ly <- as.matrix(ly, ncol=1)
    }
    else{ 
      ly <- as.matrix(ly)
    }    


    nry <- nrow(y)
    nrz <- nrow(lz)
    if(nrz != nrow(ly)){
      stop("coordinate mapping misspecified: lz and ly don't have the same number of observations") 
    }
    ncy <- ncol(y)
    ncz <- ncol(lz)
    res <- .Call("cem_create_fast", as.double(t(y)), nry, ncy, as.double(t(ly)),
        as.double(t(lz)), nrz, ncz, as.integer(knnX), as.integer(knnY),
        as.double(sigmaZ), as.double(sigmaY), as.double(sigmaX),
        as.integer(iter), as.integer(nPoints), as.double(stepZ),
        as.double(stepBW), as.integer(verbose), as.integer(risk),
        as.integer(penalty), as.integer(sigmaAsFactor),
        as.integer(optimalSigmaX), as.integer(quadratic) )  
   
    obj <- structure( list( y=y, ly = ly, lz=t(as.matrix(res[[1]])), knnX =
          knnX, knnY= knnY, sigmaX = res[[2]], sigmaY = res[[3]],
          sigmaZ=res[[4]], risk=risk,
          penalty = penalty, quadratic=quadratic ), class="cem") 
    obj

}




#Optimize existing CEM further
cem.optimize <- function(object, iter = 100, nPoints = nrow(object$y),
    stepZ=1, stepBW=0.1, verbose=1, optimalSigmaX =  F ){

  y <- object$y
  lz <- object$lz
  ly <- object$ly
  knnX <- object$knnX
  knnY <- object$knnY
  sigmaX <- object$sigmaX
  sigmaY <- object$sigmaY
  sigmaZ <- object$sigmaZ
  nry <- nrow(y)
  nrz <- nrow(lz)
  ncy <- ncol(y)
  ncz <- ncol(lz)
  res <- .Call("cem_optimize_fast", as.double(t(y)), nry, ncy, as.double(t(ly)),
      as.double(t(lz)), nrz, ncz, as.integer(knnX), as.integer(knnY), as.double(sigmaZ),
      as.double(sigmaY), as.double(sigmaX), as.integer(iter),
      as.integer(nPoints), as.double(stepZ), as.double(stepBW),
      as.integer(verbose), as.integer(object$risk), as.integer(object$penalty),
      as.integer(optimalSigmaX), as.integer(object$quadratic) ) 
    
  object$lz = t(as.matrix(res[[1]]))
  object$sigmaX = res[[2]] 
  object$sigmaY = res[[3]] 
  object$sigmaZ = res[[4]] 
  object
  
}


#Compute geodesic
cem.geodesic <- function(object, xs, xe, iter = 100, step = 0.01,
    verbose=1, ns=100){

  y <- object$y
  lz <- object$lz
  ly <- object$ly
  knnX <- object$knnX
  knnY <- object$knnY
  sigmaX <- object$sigmaX
  sigmaY <- object$sigmaY
  sigmaZ <- object$sigmaZ
  nry <- nrow(y)
  nrz <- nrow(lz)
  ncy <- ncol(y)
  ncz <- ncol(lz)
  res <- .Call("cem_geodesic_fast", as.double(t(y)), nry, ncy, as.double(t(ly)),
      as.double(t(lz)), nrz, ncz, as.integer(knnX), as.integer(knnY), as.double(sigmaZ), as.double(sigmaY),
      as.double(sigmaX), as.integer(iter), as.double(step), as.integer(verbose),
      as.integer(object$quadratic), as.double(xs), as.double(xe), as.integer(ns)) 
    
  t(res)
}


predict.cem <- function(object, newdata = object$y, type=c("coordinates",
      "curvature" ) ){

  type=match.arg(type)

  if( is.null( nrow(newdata) ) ){
    data <- as.matrix(newdata, ncol=1)
  }
  else{
    data <- as.matrix(newdata)
  }
  y <- object$y
  ly <- object$ly
  lz <- object$lz
  knnX <- object$knnX
  knnY <- object$knnY
  sigmaZ <- object$sigmaZ
  sigmaY <- object$sigmaY
  sigmaX <- object$sigmaX
  nry <- as.integer(nrow(y))
  nrz <- as.integer(nrow(lz))
  ncy <- as.integer(ncol(y))
  ncz <- as.integer(ncol(lz))
  nrd <- as.integer(nrow(data))

  if(type == "coordinates"){

    if(ncol(data)  == ncol(object$y) ){
      res <- .Call("cem_parametrize_fast", as.double(t(data)), nrd,
          as.double(t(y)), nry, ncy, as.double(t(ly)), as.double(t(lz)), nrz,
          ncz, as.integer(knnX),as.integer(knnY), as.double(sigmaZ),
          as.double(sigmaY), as.double(sigmaX), as.integer(object$quadratic) )  
      res <- t(as.matrix(res))
    }
    else{ 
      res <- .Call("cem_reconstruct_fast", as.double(t(data)), nrd,
          as.double(t(y)), nry, ncy, as.double(t(ly)), as.double(t(lz)), nrz,
          ncz, as.integer(knnX), as.integer(knnY), as.double(sigmaZ),
          as.double(sigmaY), as.double(sigmaX), as.integer(object$quadratic))
      tangents = c()
      for(i in 1:ncz){
        tangents[[i]] = t(as.matrix(res[[1+i]]))
      }
      res <- list(y = t(as.matrix(res[[1]])), tangents=tangents)
    }
  }

  else if(type == "curvature"){
     res <- .Call("cem_curvature_fast", as.double(t(data)), nrd,
     as.double(t(y)), nry, ncy, as.double(t(ly)), as.double(t(lz)), nrz,
     ncz, as.integer(knnX), as.integer(knnY), as.double(sigmaY), as.double(sigmaX),
     as.integer(object$quadratic))
      res <- list( principal = t(as.matrix(res[[1]])), 
                   mean =   as.vector(res[[2]]), 
                   gauss =  as.vector(res[[3]]), 
                   detg =   as.vector(res[[3]]), 
                   detB =   as.vector(res[[4]]), 
                   frob =   as.vector(res[[5]]) 
                 )
     
  }

res
}


