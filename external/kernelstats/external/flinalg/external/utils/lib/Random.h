#ifndef RANDOM_H
#define RANDOM_H

#include <math.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <stdlib.h>     


template <typename TPrecision>
class Random {

  public:
    Random(){
#ifndef USE_R_RNG
      srand( time(NULL) );
#endif
    };



    //Marsaglia-polar algorithm for sampling from Normal(0, 1)
    void Normal(TPrecision &s1, TPrecision &s2){
#ifdef USE_R_RNG
    GetRNGstate();

    s1 = rnorm(0, 1);
    s2 = rnorm(0, 1);

    PutRNGstate();
#else

        double x1, x2, w;
         do {
           x1 = 2.0 * rand()/(double) RAND_MAX - 1.0;
           x2 = 2.0 * rand()/(double) RAND_MAX - 1.0;
           w = x1 * x1 + x2 * x2;
         } while ( w >= 1.0 );

         w = sqrt( (-2.0 * log( w ) ) / w );
         s1 =(TPrecision)( x1 * w );
         s2 =(TPrecision)( x2 * w );
#endif
    };
    

    //Marsaglia- Normal(0, sigma)
    void Normal(TPrecision &s1, TPrecision &s2, TPrecision sigma){
      Normal(s1, s2);
      s1*=sigma;
      s2*=sigma;
    };

    //Marsaglia
    TPrecision Normal(){
      TPrecision s1, s2;
      Normal(s1, s2);
      return s1;
    };



    TPrecision Uniform(){
#ifdef USE_R_RNG
      return runif(0,1);
#else
      return rand()/(TPrecision) RAND_MAX; 
#endif
    };



    static std::vector<unsigned int> Permutation(unsigned int N){
      std::vector<unsigned int> a(N);

      for( unsigned int i=0; i<N; i++){
        a[i]; 
      }

      std::random_shuffle(a.begin(), a.end());

      return a;
    };



    static std::vector<int> PermutationFisherYates(int n){      
      std::vector<int> perm( n );
      perm[0] = 0;
      for(int j=1; j < perm.size(); j++){
#ifdef USE_R_RNG
        int el = runif(0,1) * (j+1);
#else
        int el = (  rand() / (double) RAND_MAX ) * (j+1);
#endif
 
        if(el == j+1){ el =j; }

        if(el != j){
          perm[j] = perm[el];
        }
        perm[el] = j;
      }
      return perm;

    };

  
};

#endif
