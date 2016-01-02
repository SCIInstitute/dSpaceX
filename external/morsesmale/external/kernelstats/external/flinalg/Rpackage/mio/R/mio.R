

write.matrix.double <- function (x, filename){
    .Call("write_matrix_double", nrow(x), ncol(x), as.double(x), as.character(filename)  )  
}


read.matrix.double <- function (filename){
    X <- .Call("read_matrix_double", as.character(filename) )
    X  
}




write.matrix.int <- function (x, filename){
    .Call("write_matrix_int", nrow(x), ncol(x), as.integer(x), as.character(filename)  )  
}


read.matrix.int <- function (filename){
    X <- .Call("read_matrix_int", as.character(filename) )
    X  
}







write.vector.double <- function (x, filename){
    .Call("write_vector_double", length(x), as.double(x), as.character(filename)  )  
}


read.vector.double <- function (filename){
    X <- .Call("read_vector_double", as.character(filename) )
    X  
}




write.vector.int <- function (x, filename){
    .Call("write_vector_int", length(x), as.integer(x), as.character(filename)  )  
}


read.vector.int <- function (filename){
    X <- .Call("read_vector_int", as.character(filename) )
    X  
}
