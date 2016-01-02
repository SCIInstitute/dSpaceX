#ifndef GRAPHALGORITHMS_H
#define GRAPHALGORITHMS_H

#include <algorithm>

#include "Matrix.h"
#include "SparseMatrix.h"

#include <limits>

#include "MinHeap.h"

template <typename TPrecision>
class GraphAlgorithms{
  public:

    struct Path{
      Path(int start, int N){
        d = new TPrecision[N];
        p = new int[N];
        
        for(int i=0; i < N; i++){
          d[i] = std::numeric_limits<TPrecision>::max();
          p[i] = -1;
        };
        d[start] = 0;
      }
      //distances
      TPrecision *d;
      //predecessor
      int *p;
    };


   // inplace floyd warshall to compute all pairs shortest path O(N^3), 
   // N = nodes
   static void floydWarshall(FortranLinalg::Matrix<TPrecision> &m){
     int N = m.getNumberOfNodes();

     TPrecision tmp1;
     TPrecision tmp2;

     for(int k = 0; k < N; k++){
     for(int i = 0; i < N; i++){
     for(int j = 0; j < N; j++){
        
       tmp1 = m(i, j);
       tmp2 = m(i, k) + m(k, j);
       
       m(i, j) = std::min(tmp1, tmp2);
       
     }}}
     
   };

   static void all_pairs_dijkstra(FortranLinalg::SparseMatrix<TPrecision> &adj,
       FortranLinalg::Matrix<TPrecision> &distances){
       
      for(unsigned int i=0; i<adj.N(); i++){
        //std::cout << "Computing paths for: " << i << std::endl;
        Path p=dijkstra(adj, i);
        for(unsigned int j=0; j<adj.N(); j++){
          distances(i, j) = p.d[j];
        }
        delete[] p.d;
        delete[] p.p;
      } 

   };

   //dijkstra
   static Path dijkstra(FortranLinalg::SparseMatrix<TPrecision> &m, int start){
     
      Path path(start, m.N());

      MinHeap<TPrecision> heap(m.N());
      //efficent initialization
      heap.init(m.N(), std::numeric_limits<TPrecision>::max(), start, 0);

      while(heap.getNumberOfElements() > 0){
        int min = heap.extractRootIndex();
        typename FortranLinalg::SparseMatrix<TPrecision>::SparseEntry *adj = m.getEntries(min);
        typename FortranLinalg::SparseMatrix<TPrecision>::SparseEntryIterator it = adj->begin();
         
        for(; it!=adj->end(); ++it){
          if( relax(min, it->first, it->second, path, m)){
            heap.changeOrigElement(it->first, path.d[it->first]);
          }
        }
      };

      return path;
   };



   static bool relax(int u, int v, TPrecision dist, Path &path, FortranLinalg::SparseMatrix<TPrecision> &m){
     TPrecision tmp = path.d[u] + dist;
     if( path.d[v] > tmp ){
      path.d[v] = tmp;
      path.p[v] = u;
      return true;
     }
     return false;
   };

};

#endif
