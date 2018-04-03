#ifndef SVDDGESDD_H
#define SVDDGESDD_H


#include "Linalg.h"
#include "LapackDefs.h"

namespace FortranLinalg{

template <typename TPrecision>
class SVDdgesdd{
#define isDoubleTPrecision() sizeof(TPrecision) == sizeof(double)

  public:
    DenseMatrix<TPrecision> U;
    DenseVector<TPrecision> S;
    DenseMatrix<TPrecision> Vt;
    DenseVector<TPrecision> c;


    SVDdgesdd(DenseMatrix<TPrecision> Xo, bool center=false){

      DenseMatrix<TPrecision> Xc = Linalg<TPrecision>::Copy(Xo);

      if(center){
        c = Linalg<TPrecision>::SumColumns(Xc);
        Linalg<TPrecision>::Scale(c, 1.0/Xc.N(), c);
        Linalg<TPrecision>::SubtractColumnwise(Xc, c, Xc);
      }

      FL_INT n = Xc.N();
      FL_INT m = Xc.M();
      FL_INT s = std::min(m,n);
      
      S  = DenseVector<TPrecision>( s );
      U  = DenseMatrix<TPrecision>( m, s );
      Vt = DenseMatrix<TPrecision>( s, n );
      
      TPrecision *work = new TPrecision[1];
      FL_INT lwork = -1;
      char jobz = 'S';
      FL_INT info = 0;
      FL_INT *iwork = new FL_INT[ s * 8 ];

      if(isDoubleTPrecision()){
        lapack::dgesdd_(&jobz, &m, &n, (double*)Xc.data(), &m,
          (double*)S.data(), (double*)U.data(), &m, (double*)Vt.data(), &s,
          (double*)work, &lwork, iwork, &info);

        //std::cout << info << std::endl;
        FL_INT lwork = work[0];
        delete[] work;
        work =new TPrecision[lwork];        
        lapack::dgesdd_(&jobz, &m, &n, (double*)Xc.data(), &m,
          (double*)S.data(), (double*)U.data(), &m, (double*)Vt.data(), &s,
          (double*)work, &lwork, iwork,  &info);
      }
      else{
        lapack::sgesdd_(&jobz, &m, &n, (float*)Xc.data(), &m,
          (float*)S.data(), (float*)U.data(), &m, (float*)Vt.data(), &s,
          (float*)work, &lwork, iwork,  &info);

        FL_INT lwork = work[0];
        delete[] work;
        work =new TPrecision[lwork];        
        lapack::sgesdd_(&jobz, &m, &n, (float*)Xc.data(), &m,
          (float*)S.data(), (float*)U.data(), &m, (float*)Vt.data(), &s,
          (float*)work, &lwork, iwork, &info);
      } 
      Xc.deallocate();
      delete[] work;
      delete[] iwork;
    };

    void deallocate(){
      U.deallocate();
      S.deallocate();
      Vt.deallocate();
      c.deallocate();
    };

};

}


#endif 
