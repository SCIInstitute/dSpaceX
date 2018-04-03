//Mex file for writing linalg matrices & vectors in matlab
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
    
    Precision *data = (Precision*) mxGetPr(prhs[1]);
    int M = mxGetM(prhs[1]);
    int N = mxGetN(prhs[1]);
    


    DenseMatrix<Precision> out(M, N, data)
    LinalgIO<Precision>::writeMatrix(filename, out);

    
    return;
}
