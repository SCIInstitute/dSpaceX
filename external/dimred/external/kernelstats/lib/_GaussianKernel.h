#ifndef _GAUSSIANKERNEL_H
#define _GAUSSIANKERNEL_H

#include "Kernel.h"
#include "EuclideanMetric.h"
#include "Linalg.h"

#include <cmath>



template <typename TPrecision>
class _GaussianKernel : public GaussianKernel<TPrecision>{

  private:
    /*EuclideanMetric<TPrecision> metric;
    TPrecision var;
    TPrecision ng;
    TPrecision nh;
    TPrecision c;
    unsigned int d; 
    DenseVector<TPrecision> diff;*/

    //DenseVector<TPrecision> var;
    //DenseVector<TPrecision> ng;
    //DenseVector<TPrecision> nh;
    //DenseVector<TPrecision> c;   	 
    //DenseVector<TPrecision> diff;

 		unsigned int d;
		unsigned int n;
		DenseVector<TPrecision> sig;	

  public:

		void display(DenseVector<TPrecision> x){
			for(int i=0;i<x.N();i++){
				std::cout<<"["<<i<<"]"<<x(i);
			}
		}

		void display(DenseMatrix<TPrecision> X){
			for(int i=0;i<X.M();i++){
				for(int j=0;j<X.N();j++)
				std::cout<<"["<<i<<"]"<<"["<<j<<"]"<<X(i,j);
			}
		}


		void displayKernelParam()
		{
			std::cout<<"d="<<d<<", n="<<n<<std::endl<<"sig=[ ";
			for(int i=0;i<n;i++)
			{
				std::cout<<sig(i)<<" ";
			}
			std::cout<<"]"<<std::endl;
		}

    _GaussianKernel(){
      d=1;
			n=1;
			DenseVector<TPrecision> s(n);
			sig=s;
    };  

    _GaussianKernel(unsigned int dim,unsigned int np){
      d=dim;
			n=np;
			DenseVector<TPrecision> s(n);
			sig=s;
    };

    _GaussianKernel(TPrecision *sigma,unsigned int dim,unsigned int np){
      d=dim;
			n=np;
			DenseVector<TPrecision> s(n,sigma);
			sig=s;
    };

    virtual ~_GaussianKernel(){
      //diff.deallocate();
    };

    /*TPrecision f(TPrecision dsquared){
      return c * exp( - dsquared / var);
    };*/
		TPrecision f(TPrecision dsquared,int i){
			GaussianKernel<TPrecision> gk(sig(i),d);			
			return gk.f(dsquared);
    };



    
    /*TPrecision f(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
      return f( metric.distanceSquared(x1, x2) );
    };*/
    TPrecision f(Vector<TPrecision> &x1, Vector<TPrecision> &x2,int i){
			GaussianKernel<TPrecision> gk(sig(i),d);
			return gk.f(x1,x2);
    };
  
    /*TPrecision f(Vector<TPrecision> &x1, Matrix<TPrecision> &X2, int i2){
      return f( metric.distanceSquared(X2, i2, x1 ) );
    };*/
		TPrecision f(Vector<TPrecision> &x1, Matrix<TPrecision> &X2, int i2, int i){
			GaussianKernel<TPrecision> gk(sig(i),d);      
			return gk.f(x1,X2,i2);
    };
  
  
    /*TPrecision f(Matrix<TPrecision> &X1, int i1, Matrix<TPrecision> &X2, int i2){
      return f( metric.distanceSquared(X1, i1, X2, i2 ) );
    };*/
    TPrecision f(Matrix<TPrecision> &X1, int i1, Matrix<TPrecision> &X2, int i2,int i){
			GaussianKernel<TPrecision> gk(sig(i),d);      
			return gk.f(X1,i1,X2,i2);
    };



/*************not implement
    void grad(Vector<TPrecision> &x1, Vector<TPrecision> &x2, Vector<TPrecision> &g){
      gradf(x1, x2, g); 
    };



    //Hessian
    DenseMatrix<TPrecision> hessian(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
      DenseMatrix<TPrecision> H(d, d);
      hessian(x1, x2, H);
      return H;

    };    
    
    //Hessian
    void hessian(Vector<TPrecision> &x1, Vector<TPrecision>
        &x2, DenseMatrix<TPrecision> H){

      Linalg<TPrecision>::Subtract(x1, x2, diff);

      TPrecision val = 0;
      for(unsigned int i=0; i<diff.N(); i++){
        val += diff(i)*diff(i);
      }       
      val = exp( -val / var)*nh;

      Linalg<TPrecision>::OuterProduct(diff, diff, H);
      for(unsigned int i=0; i<H.N(); i++){
        H(i, i) -= 1;
      }
      Linalg<TPrecision>::Scale(H, val, H);
      
    };
    
    //Hessian
    void hessian(Matrix<TPrecision> &X1, unsigned int i1,  
                 Matrix<TPrecision> &X2, unsigned int i2,
                 DenseMatrix<TPrecision> &H){

      Linalg<TPrecision>::Subtract(X1, i1, X2, i2, diff);

      TPrecision val = 0;
      for(unsigned int i=0; i<diff.N(); i++){
        val += diff(i)*diff(i);
      } 
      val = exp( -val / var)*nh;

      Linalg<TPrecision>::OuterProduct(diff, diff, H);
      for(unsigned int i=0; i<H.N(); i++){
        H(i, i) -= 1;
      }
      Linalg<TPrecision>::Scale(H, val, H);
    };
*/


    //grad and function value
    /*TPrecision gradf(Vector<TPrecision> &x1, Vector<TPrecision> &x2,
      Vector<TPrecision> &g){
  
      Linalg<TPrecision>::Subtract(x1, x2, g);
      TPrecision val = 0;
      for(unsigned int i=0; i<g.N(); i++){
        val += g(i)*g(i);
      } 
      val = exp( -val / var);
      for(unsigned int i=0; i<g.N(); i++){
        g(i) *= ng * val;
      }

      return val*c;
    };*/
		TPrecision gradf(Vector<TPrecision> &x1, Vector<TPrecision> &x2,
      Vector<TPrecision> &g,int i){
			GaussianKernel<TPrecision> gk(sig(i),d);
      return gk.gradf(x1,x2,g);
    };




    /*TPrecision gradf(Matrix<TPrecision> &x1, int i1, Matrix<TPrecision> &x2, int
        i2, Vector<TPrecision> &g){
  
      Linalg<TPrecision>::Subtract(x1, i1, x2, i2, g);
      TPrecision val = 0;
      for(unsigned int i=0; i<g.N(); i++){
        val += g(i)*g(i);
      } 
      val = exp( -val / var);
      for(unsigned int i=0; i<g.N(); i++){
        g(i) *= ng * val;
      }

      return val*c;
    };*/
		TPrecision gradf(Matrix<TPrecision> &x1, int i1, Matrix<TPrecision> &x2, int
        i2, Vector<TPrecision> &g,int i){
			GaussianKernel<TPrecision> gk(sig(i),d);
      return gk.gradf(x1,i1,x2,i2,g);
    };    
    
    /*TPrecision gradf(Vector<TPrecision> &x1, Matrix<TPrecision> &x2, int
        i2, Vector<TPrecision> &g){
  
      Linalg<TPrecision>::Subtract(x1, x2, i2, g);
      TPrecision val = 0;
      for(unsigned int i=0; i<g.N(); i++){
        val += g(i)*g(i);
      } 
      val = exp( -val / var);
      for(unsigned int i=0; i<g.N(); i++){
        g(i) *= ng * val;
      }

      return val*c;
    };*/
		TPrecision gradf(Vector<TPrecision> &x1, Matrix<TPrecision> &x2, int
        i2, Vector<TPrecision> &g,int i){
			GaussianKernel<TPrecision> gk(sig(i),d);
      return gk.gradf(x1,x2,i2,g);
    };



/************not implement yet
    TPrecision gradKernelParam(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
      return -2.0/(var*sqrt(var)) * f(x1, x2);         
    };
*/    
  
    /*void setKernelParam(TPrecision sigma){
      var = 2.0*sigma*sigma;
      c = 1.0/ pow(2.0*M_PI*sigma*sigma, d/2.0) ;
      ng = -2.0/var*c;
      nh = -ng/(sigma*sigma);
    };*/
		void setKernelParam(TPrecision *sigma){
			for(int i=0;i<n;i++) sig(i)=sigma[i];
    }
		void setKernelParam(TPrecision sigma){
			for(int i=0;i<n;i++) sig(i)=sigma;
    }
		void setKernelParam(unsigned int index,TPrecision sigma){
			sig(index)=sigma;
    }
		void setKernelParam(TPrecision *sigma,unsigned int dim,unsigned int np){
			d=dim;
			n=np;
			sig=sigma;
    }

		void setKernelParam(DenseVector<TPrecision> sigma){
			sig=sigma;
    }

  
    
    /*TPrecision getKernelParam(){
      return sqrt(var/2.0);
    };*/
    DenseVector<TPrecision> getKernelParam(){
      return sig;
    }

    /*TPrecision getKernelParam(){
			TPrecision res=0;
			for(int i=0;i<n;i++){
				res+=sig(i);
			}
      return res/n;
    }*/

    TPrecision getKernelParam(unsigned int n){
      return sig(n);
    }
		unsigned int D(){
      return d;
    }
		unsigned int N(){
      return n;
    }



    /*GaussianKernel<TPrecision> &operator=(const GaussianKernel<TPrecision> &rhs){
      if(this == &rhs){
        return *this;
      }
      this->var = rhs.var;
      this->ng = rhs.ng;
      this->nh = nh;
      this->c = rhs.c;
      this->d = rhs.d;
      diff.deallocate();
      diff = DenseVector<TPrecision>(d);
      
      return *this;
    };*/
		_GaussianKernel<TPrecision> &operator=(const _GaussianKernel<TPrecision> &rhs){
      if(this == &rhs){
        return *this;
      }
      this->d = rhs.d;
      this->n = rhs.n;
      this->sig = rhs.sig;
      
			return *this;
    };


};
#endif
