//Mex file for reading linalg matrices in matlab
#include <string.h>
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "mex.h"   //--This one is required


#ifdef DOUBLE
typedef double Precision;
#define CLASS mxDOUBLE_CLASS
#endif

#ifdef SINGLE
typedef float Precision;
#define CLASS mxSINGLE_CLASS
#endif

#ifdef INT64
typedef long Precision;
#define CLASS mxINT64_CLASS
#endif

#ifdef INT32
typedef int Precision;
#define CLASS mxINT32_CLASS
#endif

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

  using namespace FortranLinalg;

    char * filename = mxArrayToString(prhs[0]);
    
    
    DenseMatrix<Precision> in = LinalgIO<Precision>::readMatrix(filename);
    
    mxArray *lhs[1];

    lhs[0] = mxCreateNumericMatrix(in.M(), in.N(), CLASS, mxREAL);

    Precision *out = (Precision*) mxGetPr(lhs[0]);
    

    memcpy(out, in.data(), in.M()*in.N()*sizeof(Precision));

    plhs[0] = lhs[0]; 

    

    in.deallocate();
      
    mxFree(filename);
    
    return;
}
