#ifndef NULL
#define NULL 0
#endif

#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <stdio.h>

#include "CEM.h"


extern "C" {
  


//CEM methods
SEXP cem_create(SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy, SEXP Rmz,
    SEXP RknnY, SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Ri, 
    SEXP RsZ, SEXP RsBW, SEXP Rverbose, SEXP Rtype, SEXP RsigmaAsFactor, SEXP
    RoptimalSigmaX, SEXP Rquadratic) {
  
  int verbose = *INTEGER(Rverbose);
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int iter = *INTEGER(Ri);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  int type = *INTEGER(Rtype);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sZ = *REAL(RsZ);
  double sBW = *REAL(RsBW);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool sigmaAsFactor = *INTEGER(RsigmaAsFactor);
  bool optimalSigmaX = *INTEGER(RoptimalSigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);
  

  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, sigmaAsFactor, quadratic);
  cem.gradDescent(iter, sZ, sBW, verbose, type, optimalSigmaX);
  
  

    
  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 3));

  SEXP Zopt;
  PROTECT(Zopt = Rf_allocMatrix(REALSXP, mz, n));
  memcpy( REAL(Zopt), cem.getZ().data(), mz*n*sizeof(double) );
  SET_VECTOR_ELT(list, 0, Zopt);

  SEXP sigmaXn;
  PROTECT(sigmaXn = Rf_allocVector(REALSXP, 1));
  double *sigmaXp = REAL(sigmaXn);
  sigmaXp[0] = cem.getSigmaX();
  SET_VECTOR_ELT(list, 1, sigmaXn);

  SEXP sigmaYn;
  PROTECT(sigmaYn = Rf_allocVector(REALSXP, 1));
  double *sigmaYp = REAL(sigmaYn);
  sigmaYp[0] = cem.getSigmaY();
  SET_VECTOR_ELT(list, 2, sigmaYn);

  
  UNPROTECT(4);
   
  cem.cleanup();

  return list;  
}



SEXP cem_optimize(SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy, SEXP Rmz, SEXP RknnY,
    SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Ri, SEXP RsZ, SEXP RsBW, SEXP
    Rverbose, SEXP Rtype,  SEXP RoptimalSigmaX, SEXP Rquadratic) {
    
  int verbose = *INTEGER(Rverbose); 
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  int type = *INTEGER(Rtype);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  int iter = *INTEGER(Ri);
  double sZ = *REAL(RsZ);
  double sBW = *REAL(RsBW);
  bool optimalSigmaX = *INTEGER(RoptimalSigmaX);
  bool quadratic = *INTEGER(Rquadratic);



  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);

  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);
  cem.gradDescent(iter, sZ, sBW, verbose, type, optimalSigmaX);


  


    
  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 3));

  SEXP Zopt;
  PROTECT(Zopt = Rf_allocMatrix(REALSXP, mz, n));
  memcpy( REAL(Zopt), cem.getZ().data(), mz*n*sizeof(double) );
  SET_VECTOR_ELT(list, 0, Zopt);

  SEXP sigmaxn;
  PROTECT(sigmaxn = Rf_allocVector(REALSXP, 1));
  double *sigmaxp = REAL(sigmaxn);
  sigmaxp[0] = cem.getSigmaX();
  SET_VECTOR_ELT(list, 1, sigmaxn);

  SEXP sigmayn;
  PROTECT(sigmayn = Rf_allocVector(REALSXP, 1));
  double *sigmayp = REAL(sigmayn);
  sigmayp[0] = cem.getSigmaY();
  SET_VECTOR_ELT(list, 2, sigmayn);
  
  UNPROTECT(4);


  cem.cleanup();

  return list;  
}




SEXP cem_variance(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy,
		SEXP Rmz, SEXP RknnY, SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Rquadratic) {

  
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);

  


  
  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);
     
  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);
  
  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);
  DenseMatrix<double> Xt(mz, nd, data);
  DenseVector<double> var = cem.variance(Xt);
 

  SEXP Xvar;
  PROTECT(Xvar = Rf_allocMatrix(REALSXP, 1, nd));
  memcpy( REAL(Xvar), var.data(), nd*sizeof(double) );
  UNPROTECT(1);
  
  var.deallocate();
  cem.cleanup();
  
  return Xvar;  
}



SEXP cem_densityX(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy,
		SEXP Rmz, SEXP RknnY, SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Rquadratic) {

  
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);

  


  
  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);
     
  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);
  
  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);
  DenseMatrix<double> Xt(mz, nd, data);
  DenseVector<double> px = cem.densityX(Xt);
 

  SEXP Xpx;
  PROTECT(Xpx = Rf_allocMatrix(REALSXP, 1, nd));
  memcpy( REAL(Xpx), px.data(), nd*sizeof(double) );
  UNPROTECT(1);
  
  px.deallocate();
  cem.cleanup();
  
  return Xpx;  
}




SEXP cem_advect(SEXP Rya, SEXP RxEnd, SEXP Rstep, SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy,
		SEXP Rmz, SEXP RknnY, SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Rquadratic) {

  
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);

  


  
  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);
     
  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);
  
  DenseVector<double> ya(my, REAL(Rya) );
  ya = Linalg<double>::Copy(ya);
 

  DenseVector<double> xEnd(mz, REAL(RxEnd));   
  double stepsize = *REAL(Rstep);
  

  cem.advect(ya, xEnd, stepsize);
  

  SEXP RyEnd;
  PROTECT(RyEnd = Rf_allocVector(REALSXP, my));
  memcpy( REAL(RyEnd), ya.data(), my*sizeof(double) );
  UNPROTECT(1);
  
  ya.deallocate();
  cem.cleanup();
  
  return RyEnd;  
}





SEXP cem_parametrize(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy,
		SEXP Rmz, SEXP RknnY, SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Rquadratic) {

  
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);

  


  
  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);
     
  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);
  
  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);
  DenseMatrix<double> Ynew(my, nd, data);
  DenseMatrix<double> Xt = cem.parametrize(Ynew);
 

  SEXP Xnew;
  PROTECT(Xnew = Rf_allocMatrix(REALSXP, mz, nd));
  memcpy( REAL(Xnew), Xt.data(), mz*nd*sizeof(double) );
  UNPROTECT(1);
  
  Xt.deallocate();
  cem.cleanup();
  
  return Xnew;  
}




SEXP cem_reconstruct(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy,
    SEXP Rmz, SEXP RknnY, SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Rquadratic) {
   
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int nd = *INTEGER(Rnd);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *data = REAL(Rdata);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);



  
  DenseMatrix<double> Z(mz, n, z);
  Z = Linalg<double>::Copy(Z);
  DenseMatrix<double> Y(my, n, y);
  Y = Linalg<double>::Copy(Y);
  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);

  DenseMatrix<double> Xnew(mz, nd, data);
  DenseMatrix<double> *Ty = new DenseMatrix<double>[Z.M()];
  for(int k=0; k<Z.M(); k++){
    Ty[k] = DenseMatrix<double>(Y.M(), Xnew.N());
  }
  DenseMatrix<double> Yt(Y.M(), Xnew.N());

  DenseVector<double> yp(Y.M());
  DenseVector<double> xp(Z.M());
  DenseMatrix<double> J(Y.M(), Z.M());
  for(int i=0; i<Xnew.N(); i++){
	  Linalg<double>::ExtractColumn(Xnew, i, xp);
	  cem.g(xp, yp, J);
	  Linalg<double>::SetColumn(Yt, i, yp);
	  //Linalg<double>::QR_inplace(J);
	  for(int j=0; j<J.N(); j++){
	    Linalg<double>::SetColumn(Ty[j], i, J, j);
	  }
  }
  yp.deallocate();
  xp.deallocate();
  J.deallocate();

  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 1+Z.M()));

  SEXP Ynew;
  PROTECT(Ynew = Rf_allocMatrix(REALSXP, my, nd));
  memcpy( REAL(Ynew), Yt.data(), my*nd*sizeof(double) );
  SET_VECTOR_ELT(list, 0, Ynew);
  Yt.deallocate();

  for(int i=0; i<Z.M(); i++){
    SEXP Tnew;
    PROTECT( Tnew = Rf_allocMatrix(REALSXP, Y.M(), nd));
    memcpy( REAL(Tnew), Ty[i].data(), Y.M()*nd*sizeof(double) );
    SET_VECTOR_ELT(list, i+1, Tnew);
    Ty[i].deallocate();
  }
  delete[] Ty;
  

  
  UNPROTECT(2+Z.M());


  cem.cleanup();
  
  return list;  
}




SEXP cem_geodesic(SEXP Ry, SEXP Rz, SEXP Rn, SEXP Rmy, SEXP Rmz, SEXP RknnY,
    SEXP RknnX, SEXP RsigmaY, SEXP RsigmaX, SEXP Ri, SEXP Rs, SEXP
    Rverbose, SEXP Rtype, SEXP Rquadratic, SEXP Rxs, SEXP Rxe, SEXP Rns) {
    
  int verbose = *INTEGER(Rverbose); 
  int knnY = *INTEGER(RknnY);
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mz = *INTEGER(Rmz);
  int my = *INTEGER(Rmy);
  int type = *INTEGER(Rtype);
  double *y = REAL(Ry);
  double *z = REAL(Rz);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  int iter = *INTEGER(Ri);
  double s = *REAL(Rs);
  bool quadratic = *INTEGER(Rquadratic);
  
  double *xs = REAL(Rxs);
  double *xe = REAL(Rxe);
  int ns = *INTEGER(Rns);
 


  DenseMatrix<double> Z(mz, n, z);
  DenseMatrix<double> Y(my, n, y);
  Z = Linalg<double>::Copy(Z);
  Y = Linalg<double>::Copy(Y);


  CEM<double> cem(Y, Z, knnY, knnX, sigmaY, sigmaX, false, quadratic);
  


  DenseVector<double> xS(mz, xs);
  DenseVector<double> xE(mz, xe);
  DenseMatrix<double> geo = cem.geodesic(xS, xE, s, ns, iter);


  SEXP Xg;
  PROTECT(Xg = Rf_allocMatrix(REALSXP, mz, ns));
  memcpy( REAL(Xg), geo.data(), mz*ns*sizeof(double) );
  UNPROTECT(1);
  
  geo.deallocate();
  cem.cleanup();
  
  return Xg;  
  
}


}//end extern C
