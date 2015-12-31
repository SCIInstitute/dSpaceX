
#ifndef KMEANS_H
#define KMEANS_H

#include <Eigen/Dense>
#include <vector>
#include "Random.h"

#include <cmath>
#include <limits>

template <typename TPrecision>
class KmeansData{
  public:    
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;
   
    virtual ~KmeansData(){};

    virtual VectorXp getPoint(int index) = 0;
    virtual int getNumberOfPoints() = 0;

    virtual VectorXp getMean( std::vector<int> &pts ) = 0;
    
    virtual TPrecision getSquaredSimilarity(int index, const VectorXp &p) = 0;

};



template <typename TPrecision>
class KmeansCenter{
  public:
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;

    KmeansCenter(){
      radius = 0;
      mse = 0;
    };


    VectorXp center;
    TPrecision radius;
    TPrecision mse;
    std::vector<int> points;

};


template <typename TPrecision>
class Kmeans{

  private:
    int maxIter;
    TPrecision threshold;
    int minPoints;

    void update( std::vector< KmeansCenter<TPrecision> > &centers,
        KmeansData<TPrecision> &data ){
      
      for(int i=0; i<centers.size(); i++){
        centers[i].points.clear();
        centers[i].mse = 0;
        centers[i].radius = 0;
      } 


      for(int i=0; i<data.getNumberOfPoints(); i++){
        TPrecision dist = std::numeric_limits<TPrecision>::max();
        
        int index =-1;
        for(int j=0; j<centers.size(); j++){
          TPrecision tmp = data.getSquaredSimilarity(i, centers[j].center);
          if(tmp < dist){
            dist = tmp;
            index = j;
          }
        }

        centers[index].mse += dist;
        centers[index].radius = std::max(centers[index].radius, dist);
        centers[index].points.push_back(i);
      }
      

      //update means
      for(int i=0; i<centers.size(); i++){
        centers[i].center =  data.getMean(centers[i].points);
      } 


    };






  public:
    typedef typename Eigen::Matrix<TPrecision, Eigen::Dynamic, 1> VectorXp;
 
    Kmeans(int mIter = 100, TPrecision t = 0.01){
      maxIter = mIter;
      threshold = t;
    }; 

    ~Kmeans(){};



    std::vector< KmeansCenter<TPrecision> > run( int nClusters, KmeansData<TPrecision> &data ){
      if(nClusters > data.getNumberOfPoints() ){
        nClusters = data.getNumberOfPoints();
      }

      std::vector<int> perm = Random<int>::PermutationFisherYates( data.getNumberOfPoints() );

      std::vector< VectorXp > centers(nClusters);
      for(int i=0; i<nClusters; i++){
        centers[i] = data.getPoint( perm[i] );
      }

      return run(centers, data); 
    };



    std::vector< KmeansCenter<TPrecision> > run( std::vector< VectorXp > &centers, KmeansData<TPrecision> &data ){
      std::vector< KmeansCenter<TPrecision> > km( centers.size() );
      for(int i=0; i<km.size(); i++){
        km[i].center = centers[i];
      }
      return run(km, data);
    };



    std::vector< KmeansCenter<TPrecision> > run( std::vector< KmeansCenter<TPrecision> > centers, 
                                                 KmeansData<TPrecision> &data ){
     
       
      TPrecision totalMSE = std::numeric_limits<TPrecision>::max();
      TPrecision mse = 0; 
      int iter = 0; 
      for(iter=0; iter<maxIter; iter++){
        
        update( centers, data );
      
        mse = 0;
        for(int j=0; j<centers.size(); j++){
          mse += centers[j].mse;
        }
        mse /= data.getNumberOfPoints();
        
        if( mse == 0 || (1 - mse/totalMSE) < threshold){
          break;
        }
        totalMSE = mse;
      }


#ifdef VERBOSE
      std::cout << totalMSE << std::endl;
      std::cout << "MSE: " << mse << std::endl;
      std::cout << "Change: " << 1 - mse/totalMSE << std::endl;
      std::cout << "nIter: " << iter << std::endl;
      std::cout << "nCenters: " << centers.size() << std::endl << std::endl;
#endif

      for(int j=0; j<centers.size(); j++){
        centers[j].radius = sqrt( centers[j].radius );
        centers[j].mse /= centers[j].points.size();
      }

      return centers;
    };






    
    //iteratively refine kmeans until each center has radius at most maxRadius
    std::vector< KmeansCenter<TPrecision> > run( TPrecision maxRadius, int maxCenters, 
        KmeansData<TPrecision> &data ){

      //TODO: uneseccary shuffle just pick a ranomd point
      std::vector<int> perm  = Random<int>::PermutationFisherYates(data.getNumberOfPoints() );
      std::vector< VectorXp > centers(1);
      for(int i=0; i<centers.size(); i++){
        centers[i] = data.getPoint( perm[i] );
      }

      return run(maxRadius, maxCenters, centers, data);
    };




    std::vector< KmeansCenter<TPrecision> > run( TPrecision maxRadius, int
        maxCenters, std::vector<VectorXp> &centers, KmeansData<TPrecision> &data
        ){

      std::vector< KmeansCenter<TPrecision> > km = run(centers, data);
      bool added = true;
      while(added && km.size() < maxCenters ){
        added = false;
        
        for(int i=0; i<km.size(); i++){
          KmeansCenter<TPrecision> &c = km[i];
        
          if(c.radius > maxRadius){
            KmeansCenter<TPrecision> add;
            int el = (  rand() / (double) RAND_MAX ) * c.points.size();
            if(el == c.points.size() ){
              el = c.points.size()-1;
            }
            add.center = data.getPoint( c.points[el] );
            km.push_back(add);
            added=true;
            if( km.size() == maxCenters){
              break;
            }
          }

        }

        if(added){
          km = run(km, data);
        }
      }

      return km;

    };
   

       
    //iteratively refine kmeans until each center has at least minPoints in it
    std::vector< KmeansCenter<TPrecision> > run( int maxCenters, 
        KmeansData<TPrecision> &data, int minPoints ){

      std::vector<int> perm = Random<int>::PermutationFisherYates( data.getNumberOfPoints() );
      std::vector< VectorXp > centers(maxCenters);
      for(int i=0; i<centers.size(); i++){
        centers[i] = data.getPoint( perm[i] );
      }

      return run(centers, data, minPoints);
    };






    std::vector< KmeansCenter<TPrecision> > run( std::vector<VectorXp>
        &centers, KmeansData<TPrecision> &data, int minPoints){

      std::vector< KmeansCenter<TPrecision> > km = run(centers, data);
      bool toMany = true;
      while( toMany  ){
        toMany=false; 
        for(typename std::vector< KmeansCenter<TPrecision> >::iterator it =
            km.begin(); it != km.end(); ++it){ 
          if( (*it).points.size() < minPoints ){
            toMany = true;
            km.erase(it);
            break;
          }
        }
        if(toMany){
          km = run(km, data);
        }
      }

      return km;

    }; 
};
#endif
