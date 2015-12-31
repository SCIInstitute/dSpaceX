//Mex file for reading linalg matrices in matlab
#include <string.h>
#include "LinalgIO.h"

#include "mex.h"   

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
    
    DenseVector<Precision> in = LinalgIO<Precision>::readVector(filename);
    plhs[0] = mxCreateNumericMatrix(in.N(), 1, CLASS, mxREAL);
    
    Precision *out = (Precision*) mxGetPr(plhs[0]);
    memcpy(out, in.data(), in.N()*sizeof(Precision));

    in.deallocate();      
    mxFree(filename);
    
    return;
}
