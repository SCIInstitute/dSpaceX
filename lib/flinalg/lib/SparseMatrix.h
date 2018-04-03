#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include "Matrix.h"
#include <map>

namespace FortranLinalg{

//Simple Matrix storage to abstract row and columnwise ordering
template <typename TPrecision>
class SparseMatrix : public Matrix<TPrecision>{

  public:

    typedef typename std::map<unsigned int, TPrecision> SparseEntry;
    typedef typename SparseEntry::iterator SparseEntryIterator;
    typedef typename std::map<unsigned int, SparseEntry*> Sparse;
      
    SparseMatrix(unsigned int nrows, unsigned int ncols, TPrecision defV
        = 0):defaultValue(defV){

      n = ncols;
      m = nrows;

      for(unsigned int i=0; i<m; i++){
        data[i] = new SparseEntry(); 
      }


    };

    void set(unsigned int i, unsigned int j, Precision value){
      SparseEntry *entry = data[i]; 
      entry->operator[](j) = value;

    };

    virtual TPrecision &operator()(unsigned int i, unsigned int j){

        SparseEntry *row = data[i];
        SparseEntryIterator res = row->find(j);
        if(res != row->end()){
          return res->second;
        }
        else{
         return defaultValue;
        } 

    };

    virtual unsigned int M(){
      return m;
    };

    virtual unsigned int N(){
      return n;
    };

    virtual SparseEntry *getEntries(unsigned int index){
      return data[index];
    };
    
    virtual void deallocate(){
        for(unsigned int i=0; i<m; i++){
          delete data[i]; 
        }
    };



  private:
    TPrecision defaultValue;
    Sparse data;
    unsigned int n;
    unsigned int m;


};

}

#endif
