#ifndef NULL
#define NULL 0
#endif

#define R_NO_REMAP

#define R_PACKAGE
#define USE_R_RNG

#include <R.h>
#include <Rmath.h>
#include <Rinternals.h>
#include <stdio.h>

#include "BlockCEM.h"


extern "C" {
  


//CEM methods

SEXP cem_create(SEXP Ry, SEXP Rn, SEXP Rmy, SEXP Rx, SEXP Rnx, SEXP Rmx, SEXP
    RknnX, SEXP RsigmaX, SEXP Ri, SEXP RnP, SEXP RsX, SEXP RsBW, SEXP Rverbose,
    SEXP Rrisk, SEXP Rpenalty, SEXP RsigmaAsFactor, SEXP RoptimalSigmaX, SEXP
    Rquadratic) {

  using namespace FortranLinalg;

  int verbose = *INTEGER(Rverbose);
  int knnX = *INTEGER(RknnX);
  int iter = *INTEGER(Ri);
  int nPoints = *INTEGER(RnP);
  int n = *INTEGER(Rn);
  int mx = *INTEGER(Rmx);
  int nx = *INTEGER(Rnx);
  int my = *INTEGER(Rmy);
  int riskI = *INTEGER(Rrisk);
  int penaltyI = *INTEGER(Rpenalty);
  double *y = REAL(Ry);

  double *x = REAL(Rx);
  
  double sX = *REAL(RsX);
  double sBW = *REAL(RsBW);
  double sigmaX = *REAL(RsigmaX);
  bool sigmaAsFactor = *INTEGER(RsigmaAsFactor);
  bool optimalSigmaX = *INTEGER(RoptimalSigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> X(mx, nx, x);
  DenseMatrix<double> Y(my, n, y);
  X = Linalg<double>::Copy(X);
  Y = Linalg<double>::Copy(Y);
  
  BlockCEM<double>::Risk risk = BlockCEM<double>::toRisk(riskI);
  BlockCEM<double>::Penalty penalty = BlockCEM<double>::toPenalty(penaltyI);

  BlockCEM<double> cem(Y, X, knnX, sigmaX, sigmaAsFactor, quadratic); 
  cem.gradDescent(iter, nPoints, sX, sBW, verbose, risk, penalty, optimalSigmaX);
  
  

    
  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 2));

  SEXP Xopt;
  PROTECT(Xopt = Rf_allocMatrix(REALSXP, mx, nx));
  memcpy( REAL(Xopt), cem.getX().data(), mx * nx * sizeof(double) );
  SET_VECTOR_ELT(list, 0, Xopt);

  SEXP sigmaXn;
  PROTECT(sigmaXn = Rf_allocVector(REALSXP, 1));
  double *sigmaXp = REAL(sigmaXn);
  sigmaXp[0] = cem.getSigmaX();
  SET_VECTOR_ELT(list, 1, sigmaXn);

  UNPROTECT(3);
   
  cem.cleanup();

  return list;  
}



SEXP cem_optimize(SEXP Ry, SEXP Rn, SEXP Rmy, SEXP Rx, SEXP Rnx, SEXP Rmx, SEXP
    RknnX, SEXP RsigmaX, SEXP Ri, SEXP RnP, SEXP RsX, SEXP RsBW, SEXP Rverbose,
    SEXP Rrisk, SEXP Rpenalty, SEXP RoptimalSigmaX, SEXP Rquadratic) {
  using namespace FortranLinalg;
  
  int verbose = *INTEGER(Rverbose);
  int knnX = *INTEGER(RknnX);
  int iter = *INTEGER(Ri);
  int nPoints = *INTEGER(RnP);
  int n = *INTEGER(Rn);
  int mx = *INTEGER(Rmx);
  int nx = *INTEGER(Rnx);
  int my = *INTEGER(Rmy);
  int riskI = *INTEGER(Rrisk);
  int penaltyI = *INTEGER(Rpenalty);
  
  double *y = REAL(Ry);
  double *x = REAL(Rx);
  
  double sX = *REAL(RsX);
  double sBW = *REAL(RsBW);
  double sigmaX = *REAL(RsigmaX);
  bool optimalSigmaX = *INTEGER(RoptimalSigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> Y(my, n, y);
  DenseMatrix<double> X(mx, nx, x);
  X = Linalg<double>::Copy(X);
  Y = Linalg<double>::Copy(Y);
  
  BlockCEM<double>::Risk risk = BlockCEM<double>::toRisk(riskI);
  BlockCEM<double>::Penalty penalty = BlockCEM<double>::toPenalty(penaltyI);


  BlockCEM<double> cem(Y, X, knnX,sigmaX, false, quadratic); 
  cem.gradDescent(iter, nPoints, sX, sBW, verbose, risk, penalty, optimalSigmaX);
     

    
  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 2));

  SEXP Xopt;
  PROTECT(Xopt = Rf_allocMatrix(REALSXP, mx, nx));
  memcpy( REAL(Xopt), cem.getX().data(), mx * nx * sizeof(double) );
  SET_VECTOR_ELT(list, 0, Xopt);

  SEXP sigmaXn;
  PROTECT(sigmaXn = Rf_allocVector(REALSXP, 1));
  double *sigmaXp = REAL(sigmaXn);
  sigmaXp[0] = cem.getSigmaX();
  SET_VECTOR_ELT(list, 1, sigmaXn);

  UNPROTECT(3);
   
  cem.cleanup();

  return list;  
 
    
}







SEXP cem_parametrize(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rn, SEXP Rmy, SEXP Rx,
    SEXP Rnx, SEXP Rmx, SEXP RknnX, SEXP RsigmaX, SEXP Rquadratic) {
  using namespace FortranLinalg;
  
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mx = *INTEGER(Rmx);
  int nx = *INTEGER(Rnx);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *x = REAL(Rx);

  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> Y(my, n, y);
  DenseMatrix<double> X(mx, nx, x);  
  X = Linalg<double>::Copy(X);
  Y = Linalg<double>::Copy(Y);
  

  BlockCEM<double> cem(Y, X, knnX,sigmaX, false, quadratic); 

  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);
  DenseMatrix<double> Ynew(my, nd, data);
  
  DenseMatrix<double> Xnew = cem.parametrize(Ynew);
  SEXP res;
  PROTECT(res = Rf_allocMatrix(REALSXP, mx, nd));
  memcpy( REAL(res), Xnew.data(), mx*nd*sizeof(double) );
  Xnew.deallocate();

  UNPROTECT(1);

  return res;  
}




SEXP cem_curvature(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rn, SEXP Rmy,
    SEXP Rx, SEXP Rnx, SEXP Rmx, SEXP RknnX, SEXP RsigmaX, SEXP Rquadratic) {
  using namespace FortranLinalg;
  
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mx = *INTEGER(Rmx);
  int nx = *INTEGER(Rnx);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *x = REAL(Rx);

  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> Y(my, n, y);
  DenseMatrix<double> X(mx, nx, x);  
  X = Linalg<double>::Copy(X);
  Y = Linalg<double>::Copy(Y);
  
  BlockCEM<double> cem(Y, X, knnX,sigmaX, false, quadratic); 

  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);

 
  DenseMatrix<double> Xnew(mx, nd, data);
  DenseMatrix<double> curvature(Y.M(), Xnew.N());
  DenseVector<double> gauss(Xnew.N());
  DenseVector<double> mean(Xnew.N());
  DenseVector<double> detg(Xnew.N());
  DenseVector<double> detB(Xnew.N());
  DenseVector<double> frob(Xnew.N());

  DenseVector<double> xp(X.M());
  for(unsigned int i=0; i<Xnew.N(); i++){
	  Linalg<double>::ExtractColumn(Xnew, i, xp);
	  DenseVector<double> c = cem.curvature(xp, mean(i), gauss(i), detg(i), detB(i), frob(i));
	  Linalg<double>::SetColumn(curvature, i, c);
    c.deallocate();
  }
  xp.deallocate();

  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 6));

  SEXP Rcurv;
  PROTECT(Rcurv = Rf_allocMatrix(REALSXP, curvature.M(), curvature.N()));
  memcpy( REAL(Rcurv), curvature.data(), curvature.M()*curvature.N()*sizeof(double) );
  SET_VECTOR_ELT(list, 0, Rcurv);
  curvature.deallocate();
  
  SEXP Rmean;
  PROTECT(Rmean = Rf_allocVector(REALSXP, mean.N()));
  memcpy( REAL(Rmean), mean.data(), mean.N()*sizeof(double) );
  SET_VECTOR_ELT(list, 1, Rmean);
  mean.deallocate();
  
  SEXP Rgauss;
  PROTECT(Rgauss = Rf_allocVector(REALSXP, gauss.N()));
  memcpy( REAL(Rgauss), gauss.data(), gauss.N()*sizeof(double) );
  SET_VECTOR_ELT(list, 2, Rgauss);
  gauss.deallocate();
  
  SEXP Rdetg;
  PROTECT(Rdetg = Rf_allocVector(REALSXP, detg.N()));
  memcpy( REAL(Rdetg), detg.data(), detg.N()*sizeof(double) );
  SET_VECTOR_ELT(list, 3, Rdetg);
  detg.deallocate();
  
  SEXP RdetB;
  PROTECT(RdetB = Rf_allocVector(REALSXP, detB.N()));
  memcpy( REAL(RdetB), detB.data(), detB.N()*sizeof(double) );
  SET_VECTOR_ELT(list, 4, RdetB);
  detB.deallocate();
  
  SEXP Rfrob;
  PROTECT(Rfrob = Rf_allocVector(REALSXP, frob.N()));
  memcpy( REAL(Rfrob), frob.data(), frob.N()*sizeof(double) );
  SET_VECTOR_ELT(list, 5, Rfrob);
  frob.deallocate();




  UNPROTECT(7);


  cem.cleanup();
  
  return list;  
}



SEXP cem_reconstruct(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rn, SEXP Rmy, 
    SEXP Rx, SEXP Rnx, SEXP Rmx, SEXP RknnX, SEXP RsigmaX, SEXP Rquadratic) {
  using namespace FortranLinalg;
  
  int knnX = *INTEGER(RknnX);
  int n = *INTEGER(Rn);
  int mx = *INTEGER(Rmx);
  int nx = *INTEGER(Rnx);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);
  double *x = REAL(Rx);

  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> Y(my, n, y);
  DenseMatrix<double> X(mx, nx, x);  
  X = Linalg<double>::Copy(X);
  Y = Linalg<double>::Copy(Y);
  
  BlockCEM<double> cem(Y, X, knnX,sigmaX, false, quadratic); 

  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);
  
  DenseMatrix<double> Xnew(mx, nd, data);
  DenseMatrix<double> *Ty = new DenseMatrix<double>[X.M()];
  for(unsigned int k=0; k<X.M(); k++){
    Ty[k] = DenseMatrix<double>(Y.M(), Xnew.N());
  }
  DenseMatrix<double> Yt(Y.M(), Xnew.N());

  DenseVector<double> yp(Y.M());
  DenseVector<double> xp(X.M());
  DenseMatrix<double> J(Y.M(), X.M());
  for(unsigned int i=0; i<Xnew.N(); i++){
	  Linalg<double>::ExtractColumn(Xnew, i, xp);
	  cem.g(xp, yp, J);
	  Linalg<double>::SetColumn(Yt, i, yp);
	  //Linalg<double>::QR_inplace(J);
	  for(unsigned int j=0; j<J.N(); j++){
	    Linalg<double>::SetColumn(Ty[j], i, J, j);
	  }
  }
  yp.deallocate();
  xp.deallocate();
  J.deallocate();

  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 1+X.M()));

  SEXP Ynew;
  PROTECT(Ynew = Rf_allocMatrix(REALSXP, my, nd));
  memcpy( REAL(Ynew), Yt.data(), my*nd*sizeof(double) );
  SET_VECTOR_ELT(list, 0, Ynew);
  Yt.deallocate();

  for(int i=0; i<X.M(); i++){
    SEXP Tnew;
    PROTECT( Tnew = Rf_allocMatrix(REALSXP, Y.M(), nd));
    memcpy( REAL(Tnew), Ty[i].data(), Y.M()*nd*sizeof(double) );
    SET_VECTOR_ELT(list, i+1, Tnew);
    Ty[i].deallocate();
  }
  delete[] Ty;
  

  
  UNPROTECT(2+X.M());


  cem.cleanup();
  
  return list;  
}



SEXP cem_geodesic(SEXP Ry, SEXP Rn, SEXP Rmy, SEXP Rx, SEXP Rnx, SEXP Rmx, 
    SEXP RknnX, SEXP RsigmaX, SEXP Ri, SEXP Rs, SEXP Rverbose,  SEXP Rquadratic, SEXP
    Rxs, SEXP Rxe, SEXP Rns) {
  using namespace FortranLinalg;
  
  int verbose = *INTEGER(Rverbose);
  int knnX = *INTEGER(RknnX);
  int iter = *INTEGER(Ri);
  int n = *INTEGER(Rn);
  int mx = *INTEGER(Rmx);
  int nx = *INTEGER(Rnx);
  int my = *INTEGER(Rmy);
  
  double *y = REAL(Ry);
  double *x = REAL(Rx);

  
  double s = *REAL(Rs);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  double *xs = REAL(Rxs);
  double *xe = REAL(Rxe);
  int ns = *INTEGER(Rns);


  DenseMatrix<double> Y(my, n, y);
  DenseMatrix<double> X(mx, nx, x);
  Y = Linalg<double>::Copy(Y);
  X = Linalg<double>::Copy(X);
  

  BlockCEM<double> cem(Y, X, knnX, sigmaX, false, quadratic); 

  DenseVector<double> xS(mx, xs);
  DenseVector<double> xE(mx, xe);
  DenseMatrix<double> geo = cem.geodesic(xS, xE, s, ns, iter);
 

  SEXP Xg;
  PROTECT(Xg = Rf_allocMatrix(REALSXP, mx, ns));
  memcpy( REAL(Xg), geo.data(), mx*ns*sizeof(double) );
  UNPROTECT(1);
  
  geo.deallocate();
  cem.cleanup();
  
  return Xg;  
      
}


}//end extern C
