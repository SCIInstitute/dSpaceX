#ifndef NULL
#define NULL 0
#endif

#define R_NO_REMAP
#define R_PACKAGE

#include <R.h>
#include <Rinternals.h>
#include <stdio.h>

#include "FastCEM.h"


extern "C" {
  


//CEM methods

SEXP cem_create_fast(SEXP Ry, SEXP Rn, SEXP Rmy, SEXP RlambdaY, SEXP RlambdaZ,
    SEXP RlambdaN, SEXP RlambdaM, SEXP RknnX, SEXP RknnY, SEXP RsigmaZ, SEXP RsigmaY, SEXP RsigmaX, SEXP
    Ri, SEXP RnP, SEXP RsZ, SEXP RsBW, SEXP Rverbose, SEXP Rrisk, SEXP Rpenalty, SEXP
    RsigmaAsFactor, SEXP RoptimalSigmaX, SEXP Rquadratic) {

  using namespace FortranLinalg;

  int verbose = *INTEGER(Rverbose);
  int knnX = *INTEGER(RknnX);
  int knnY = *INTEGER(RknnY);
  int iter = *INTEGER(Ri);
  int nPoints = *INTEGER(RnP);
  int n = *INTEGER(Rn);
  int lambdaM = *INTEGER(RlambdaM);
  int lambdaN = *INTEGER(RlambdaN);
  int my = *INTEGER(Rmy);
  int riskI = *INTEGER(Rrisk);
  int penaltyI = *INTEGER(Rpenalty);
  double *y = REAL(Ry);

  double *lambdaZ = REAL(RlambdaZ);
  double *lambdaY = REAL(RlambdaY);
  
  double sZ = *REAL(RsZ);
  double sBW = *REAL(RsBW);
  double sigmaZ = *REAL(RsigmaY);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool sigmaAsFactor = *INTEGER(RsigmaAsFactor);
  bool optimalSigmaX = *INTEGER(RoptimalSigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> LambdaZ(lambdaM, lambdaN, lambdaZ);
  DenseMatrix<double> LambdaY(my, lambdaN, lambdaY);
  DenseMatrix<double> Y(my, n, y);
  LambdaZ = Linalg<double>::Copy(LambdaZ);
  LambdaY = Linalg<double>::Copy(LambdaY);
  Y = Linalg<double>::Copy(Y);
  
  Risk risk = FastCEM<double>::toRisk(riskI);
  Penalty penalty = FastCEM<double>::toPenalty(penaltyI);

  FastCEM<double> cem(Y, LambdaY, knnY, LambdaZ, knnX, sigmaZ, sigmaY, sigmaX, sigmaAsFactor,
      quadratic); 
  cem.gradDescent(iter, nPoints, sZ, sBW, verbose, risk, penalty, optimalSigmaX);
  
  

    
  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 4));

  SEXP Zopt;
  PROTECT(Zopt = Rf_allocMatrix(REALSXP, lambdaM, lambdaN));
  memcpy( REAL(Zopt), cem.getZ().data(), lambdaM * lambdaN * sizeof(double) );
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
    
  SEXP sigmazn;
  PROTECT(sigmazn = Rf_allocVector(REALSXP, 1));
  double *sigmazp = REAL(sigmazn);
  sigmazp[0] = cem.getSigmaZ();
  SET_VECTOR_ELT(list, 3, sigmazn);

  
  UNPROTECT(5);
   
  cem.cleanup();

  return list;  
}



SEXP cem_optimize_fast(SEXP Ry, SEXP Rn, SEXP Rmy, SEXP RlambdaY, SEXP RlambdaZ,
    SEXP RlambdaN, SEXP RlambdaM, SEXP RknnX, SEXP RknnY, SEXP RsigmaZ, SEXP
    RsigmaY, SEXP RsigmaX, SEXP Ri, SEXP RnP, SEXP RsZ, SEXP RsBW, SEXP
    Rverbose, SEXP Rrisk, SEXP Rpenalty, SEXP RoptimalSigmaX, SEXP Rquadratic) {
  using namespace FortranLinalg;
  
  int verbose = *INTEGER(Rverbose);
  int knnX = *INTEGER(RknnX);
  int knnY = *INTEGER(RknnY);
  int iter = *INTEGER(Ri);
  int nPoints = *INTEGER(RnP);
  int n = *INTEGER(Rn);
  int lambdaM = *INTEGER(RlambdaM);
  int lambdaN = *INTEGER(RlambdaN);
  int my = *INTEGER(Rmy);
  int riskI = *INTEGER(Rrisk);
  int penaltyI = *INTEGER(Rpenalty);
  
  double *y = REAL(Ry);

  double *lambdaZ = REAL(RlambdaZ);
  double *lambdaY = REAL(RlambdaY);
  
  double sZ = *REAL(RsZ);
  double sBW = *REAL(RsBW);
  double sigmaZ = *REAL(RsigmaY);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool optimalSigmaX = *INTEGER(RoptimalSigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> LambdaZ(lambdaM, lambdaN, lambdaZ);
  DenseMatrix<double> LambdaY(my, lambdaN, lambdaY);
  DenseMatrix<double> Y(my, n, y);
  LambdaZ = Linalg<double>::Copy(LambdaZ);
  LambdaY = Linalg<double>::Copy(LambdaY);
  Y = Linalg<double>::Copy(Y);
  
  Risk risk = FastCEM<double>::toRisk(riskI);
  Penalty penalty = FastCEM<double>::toPenalty(penaltyI);


  FastCEM<double> cem(Y, LambdaY, knnY, LambdaZ, knnX, sigmaZ,  sigmaY, sigmaX, false, quadratic); 
  cem.gradDescent(iter, nPoints, sZ, sBW, verbose, risk, penalty, optimalSigmaX);
    
    
  SEXP list;
  PROTECT( list = Rf_allocVector(VECSXP, 4));

  SEXP Zopt;
  PROTECT(Zopt = Rf_allocMatrix(REALSXP, lambdaM, lambdaN));
  memcpy( REAL(Zopt), cem.getZ().data(), lambdaM * lambdaN * sizeof(double) );
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
    
  SEXP sigmazn;
  PROTECT(sigmazn = Rf_allocVector(REALSXP, 1));
  double *sigmazp = REAL(sigmazn);
  sigmazp[0] = cem.getSigmaZ();
  SET_VECTOR_ELT(list, 3, sigmazn);
  
  UNPROTECT(5);


  cem.cleanup();

  return list;  
}







SEXP cem_parametrize_fast(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rn, SEXP Rmy, SEXP
    RlambdaY, SEXP RlambdaZ, SEXP RlambdaN, SEXP RlambdaM, SEXP RknnX, SEXP RknnY, SEXP RsigmaZ, SEXP
    RsigmaY, SEXP RsigmaX, SEXP Rquadratic) {
  using namespace FortranLinalg;
  
  int knnX = *INTEGER(RknnX);
  int knnY = *INTEGER(RknnY);
  int n = *INTEGER(Rn);
  int lambdaM = *INTEGER(RlambdaM);
  int lambdaN = *INTEGER(RlambdaN);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);

  double *lambdaZ = REAL(RlambdaZ);
  double *lambdaY = REAL(RlambdaY);
  
  double sigmaZ = *REAL(RsigmaZ);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> LambdaZ(lambdaM, lambdaN, lambdaZ);
  DenseMatrix<double> LambdaY(my, lambdaN, lambdaY);
  DenseMatrix<double> Y(my, n, y);
  LambdaZ = Linalg<double>::Copy(LambdaZ);
  LambdaY = Linalg<double>::Copy(LambdaY);
  Y = Linalg<double>::Copy(Y);
  

  FastCEM<double> cem(Y, LambdaY, knnY,  LambdaZ, knnX, sigmaZ, sigmaY, sigmaX, false, quadratic);
    
  
  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);
  DenseMatrix<double> Ynew(my, nd, data);
  DenseMatrix<double> Xt = cem.parametrize(Ynew);
 

  SEXP Xnew;
  PROTECT(Xnew = Rf_allocMatrix(REALSXP, lambdaM, nd));
  memcpy( REAL(Xnew), Xt.data(), lambdaM*nd*sizeof(double) );
  UNPROTECT(1);
  
  Xt.deallocate();
  cem.cleanup();
  
  return Xnew;  
}




SEXP cem_curvature_fast(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rn, SEXP Rmy, 
    SEXP RlambdaY, SEXP RlambdaZ, SEXP RlambdaN, SEXP RlambdaM,
    SEXP RknnX, SEXP RknnY, SEXP RsigmaZ, SEXP RsigmaY, SEXP RsigmaX, SEXP
    Rquadratic) {
  using namespace FortranLinalg;
  
  int knnX = *INTEGER(RknnX);
  int knnY = *INTEGER(RknnY);
  int n = *INTEGER(Rn);
  int lambdaM = *INTEGER(RlambdaM);
  int lambdaN = *INTEGER(RlambdaN);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);

  double *lambdaZ = REAL(RlambdaZ);
  double *lambdaY = REAL(RlambdaY);
  
  double sigmaZ = *REAL(RsigmaZ);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> LambdaZ(lambdaM, lambdaN, lambdaZ);
  DenseMatrix<double> LambdaY(my, lambdaN, lambdaY);
  DenseMatrix<double> Y(my, n, y);
  LambdaZ = Linalg<double>::Copy(LambdaZ);
  LambdaY = Linalg<double>::Copy(LambdaY);
  Y = Linalg<double>::Copy(Y);
  

  FastCEM<double> cem(Y, LambdaY, knnY,  LambdaZ, knnX, sigmaZ, sigmaY, sigmaX, false, quadratic);
   
  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);   
  DenseMatrix<double> Xnew(lambdaM, nd, data);
  DenseMatrix<double> curvature(LambdaY.M(), Xnew.N());
  DenseVector<double> gauss(Xnew.N());
  DenseVector<double> mean(Xnew.N());
  DenseVector<double> detg(Xnew.N());
  DenseVector<double> detB(Xnew.N());
  DenseVector<double> frob(Xnew.N());

  DenseVector<double> xp(LambdaZ.M());
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



SEXP cem_reconstruct_fast(SEXP Rdata, SEXP Rnd, SEXP Ry, SEXP Rn, SEXP Rmy, 
    SEXP RlambdaY, SEXP RlambdaZ, SEXP RlambdaN, SEXP RlambdaM,
    SEXP RknnX, SEXP RknnY, SEXP RsigmaZ, SEXP RsigmaY, SEXP RsigmaX, SEXP
    Rquadratic) {
  using namespace FortranLinalg;
  
  int knnX = *INTEGER(RknnX);
  int knnY = *INTEGER(RknnY);
  int n = *INTEGER(Rn);
  int lambdaM = *INTEGER(RlambdaM);
  int lambdaN = *INTEGER(RlambdaN);
  int my = *INTEGER(Rmy);
  double *y = REAL(Ry);

  double *lambdaZ = REAL(RlambdaZ);
  double *lambdaY = REAL(RlambdaY);
  
  double sigmaZ = *REAL(RsigmaZ);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  DenseMatrix<double> LambdaZ(lambdaM, lambdaN, lambdaZ);
  DenseMatrix<double> LambdaY(my, lambdaN, lambdaY);
  DenseMatrix<double> Y(my, n, y);
  LambdaZ = Linalg<double>::Copy(LambdaZ);
  LambdaY = Linalg<double>::Copy(LambdaY);
  Y = Linalg<double>::Copy(Y);
  

  FastCEM<double> cem(Y, LambdaY, knnY,  LambdaZ, knnX, sigmaZ, sigmaY, sigmaX, false, quadratic);
   
  double *data = REAL(Rdata);
  int nd = *INTEGER(Rnd);   
  DenseMatrix<double> Xnew(lambdaM, nd, data);
  DenseMatrix<double> *Ty = new DenseMatrix<double>[lambdaM];
  for(unsigned int k=0; k<LambdaZ.M(); k++){
    Ty[k] = DenseMatrix<double>(Y.M(), Xnew.N());
  }
  DenseMatrix<double> Yt(Y.M(), Xnew.N());

  DenseVector<double> yp(Y.M());
  DenseVector<double> xp(LambdaZ.M());
  DenseMatrix<double> J(Y.M(), LambdaZ.M());
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
  PROTECT( list = Rf_allocVector(VECSXP, 1+lambdaM));

  SEXP Ynew;
  PROTECT(Ynew = Rf_allocMatrix(REALSXP, my, nd));
  memcpy( REAL(Ynew), Yt.data(), my*nd*sizeof(double) );
  SET_VECTOR_ELT(list, 0, Ynew);
  Yt.deallocate();

  for(int i=0; i<lambdaM; i++){
    SEXP Tnew;
    PROTECT( Tnew = Rf_allocMatrix(REALSXP, Y.M(), nd));
    memcpy( REAL(Tnew), Ty[i].data(), Y.M()*nd*sizeof(double) );
    SET_VECTOR_ELT(list, i+1, Tnew);
    Ty[i].deallocate();
  }
  delete[] Ty;
  

  
  UNPROTECT(2+lambdaM);


  cem.cleanup();
  
  return list;  
}



SEXP cem_geodesic_fast(SEXP Ry, SEXP Rn, SEXP Rmy, SEXP RlambdaY, SEXP RlambdaZ,
    SEXP RlambdaN, SEXP RlambdaM, SEXP RknnX, SEXP RknnY, SEXP RsigmaZ, SEXP RsigmaY, SEXP RsigmaX, SEXP
    Ri, SEXP Rs, SEXP Rverbose,  SEXP Rquadratic, SEXP Rxs, SEXP Rxe,
    SEXP Rns) {
  using namespace FortranLinalg;
  
  int verbose = *INTEGER(Rverbose);
  int knnX = *INTEGER(RknnX);
  int knnY = *INTEGER(RknnY);
  int iter = *INTEGER(Ri);
  int n = *INTEGER(Rn);
  int lambdaM = *INTEGER(RlambdaM);
  int lambdaN = *INTEGER(RlambdaN);
  int my = *INTEGER(Rmy);
  
  double *y = REAL(Ry);

  double *lambdaZ = REAL(RlambdaZ);
  double *lambdaY = REAL(RlambdaY);
  
  double s = *REAL(Rs);
  double sigmaZ = *REAL(RsigmaZ);
  double sigmaY = *REAL(RsigmaY);
  double sigmaX = *REAL(RsigmaX);
  bool quadratic = *INTEGER(Rquadratic);
  
  double *xs = REAL(Rxs);
  double *xe = REAL(Rxe);
  int ns = *INTEGER(Rns);


  DenseMatrix<double> LambdaZ(lambdaM, lambdaN, lambdaZ);
  DenseMatrix<double> LambdaY(my, lambdaN, lambdaY);
  DenseMatrix<double> Y(my, n, y);
  LambdaZ = Linalg<double>::Copy(LambdaZ);
  LambdaY = Linalg<double>::Copy(LambdaY);
  Y = Linalg<double>::Copy(Y);
  

  FastCEM<double> cem(Y, LambdaY, knnY,  LambdaZ, knnX, sigmaZ, sigmaY, sigmaX, false,
      quadratic); 

  int mz = LambdaZ.M(); 
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
