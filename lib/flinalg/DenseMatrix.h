#ifndef DENSEMATRIX_H
#define DENSEMATRIX_H

#include <cstddef>
#include "Matrix.h"

namespace FortranLinalg {


    // Create a simple matrix storage that is column-major to match Lapack
    // by default C++ is row-major.
    template<typename TPrecision>
    class DenseMatrix : public Matrix<TPrecision> {


    public:
        DenseMatrix() {
            m = n = 0;
            a = NULL;
            fastAccess = NULL;
        };


        DenseMatrix(unsigned int nrows, unsigned int ncols, TPrecision *data = NULL) {
            m = nrows;
            n = ncols;

            unsigned long l = n;
            l *= m;
            a = data;
            if (a == NULL) {
                a = new TPrecision[l];
            }
            createFastAccess();
            setupFastAccess();
        };

        virtual TPrecision &operator()(unsigned int row_index, unsigned int column_index) {
            //if(i >= m || j >= n) throw "Out of bounds";
            return fastAccess[column_index][row_index];
        };

        unsigned int M() {
            return m;
        };

        unsigned int N() {
            return n;
        };

        TPrecision *data() {
            return a;
        };

        void setDataPointer(TPrecision *data) {
            a = data;
            setupFastAccess();
        };

        void deallocate() {
            if (a != NULL) {
                delete[] a;
                delete[] fastAccess;
                fastAccess = NULL;
                a = NULL;
            }
        };

        TPrecision **getColumnAccessor() {
            return fastAccess;
        };


    protected:
        //Access to data array
        TPrecision *a;
        TPrecision **fastAccess;
        //M ros, N cols
        unsigned int m, n;


    private:
        void createFastAccess() {
            fastAccess = new TPrecision *[n];
        };


        void setupFastAccess() {
            for (unsigned int i = 0; i < n; i++) {
                fastAccess[i] = &a[i * m];
            }
        };

    };
}

#endif
