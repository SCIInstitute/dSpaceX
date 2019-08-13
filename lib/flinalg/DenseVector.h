#ifndef DENSEVECTOR_H
#define DENSEVECTOR_H

#include <cstddef>
#include "Vector.h"

namespace FortranLinalg {

/**
 * Represents dense vector - I am assuming this means every location has a value?
 * Original Comment: Simple Matrix storage to abstract row and column wise ordering
 * @tparam TPrecision
 */
    template<typename TPrecision>
    class DenseVector : public Vector<TPrecision> {


    public:
        /**
         * Constructors
         */
        DenseVector() {
            n = 0;
            a = NULL;
        };

        explicit DenseVector(unsigned int nrows, TPrecision *data = NULL) {
            n = nrows;
            a = data;
            if (a == NULL) {
                a = new TPrecision[n];
            }
        };

        /**
         * Destructor
         */
        virtual ~DenseVector() = default;

        /**
         * Returns the value in the vector at given index (i)
         * @param i - index of value to retrieve
         * @return value in the vector at given index (i)
         */
        virtual TPrecision &operator()(unsigned int i) {
            return a[i];
        };

        /**
         * Returns number of rows
         * @return (int) number of rows
         */
        unsigned int N() {
            return n;
        };

        /**
         * Returns pointer to data array
         * @return
         */
        TPrecision *data() {
            return a;
        };

        /**
         * Deallocates the memory used by vector
         */
        void deallocate() {
            if (a != NULL) {
                delete[] a;
                a = NULL;
            }
        };

    protected:
        TPrecision *a; // Access to data array
        unsigned int n; // Number of rows in vector
    };
}
#endif
