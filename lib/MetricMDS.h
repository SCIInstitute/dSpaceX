#ifndef METRICMDS_H
#define METRICMDS_H

#include <math.h>

#include "Linalg.h"

#include "Geometry.h"

#include "DenseMatrix.h"
#include "DenseVector.h"
#include "SymmetricEigensystem.h"

template <typename TPrecision>
class MetricMDS{


  public:
    FortranLinalg::DenseMatrix<TPrecision>
      embed(FortranLinalg::Matrix<TPrecision> &data, Metric<TPrecision>
        &metric, unsigned int ndims){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> distances(data.N(), data.N());
      Geometry<TPrecision>::computeDistances(data, metric, distances);
      DenseMatrix<TPrecision> result = embed(distances, ndims);
      distances.deallocate();
      return result;
    };

    FortranLinalg::DenseMatrix<TPrecision> embed(FortranLinalg::DenseMatrix<TPrecision> &m, int ndims){
      using namespace FortranLinalg;

      TPrecision *tmp = m.data();
      for(unsigned int i=0; i < m.M() * m.N(); i++){
          tmp[i] = (TPrecision)( -0.5 * tmp[i] * tmp[i] );
      }
      
      //Center matrix m
      //remove row mean
      DenseVector<TPrecision> rowMean = Linalg<TPrecision>::SumRows(m);
      Linalg<TPrecision>::Scale(rowMean, (TPrecision) 1.0/m.M(), rowMean);
      Linalg<TPrecision>::SubtractRowwise(m, rowMean, m);
      rowMean.deallocate();

      //remove column mean
      DenseVector<TPrecision> colMean = Linalg<TPrecision>::SumColumns(m);
      Linalg<TPrecision>::Scale(colMean, (TPrecision) 1.0/m.N(), colMean);
      Linalg<TPrecision>::SubtractColumnwise(m, colMean, m);
      colMean.deallocate();


      //do the scaling
      SymmetricEigensystem<TPrecision> eigs(m, m.N()-ndims+1, m.N());

      for(unsigned int i=0; i<eigs.ew.N(); i++){
        eigs.ew(i) = sqrt( fabs(eigs.ew(i)) );
        for(unsigned int j=0; j < eigs.ev.M(); j++){
          eigs.ev(j, i) = eigs.ew(i) * eigs.ev(j, i);
        }
      }
      for(unsigned int i=eigs.ew.N(); i<eigs.ev.N(); i++){
        for(unsigned int j=0; j < eigs.ev.M(); j++){
          eigs.ev(j, i) = 0;
        }
      }

      eigs.ew.deallocate();
      DenseMatrix<TPrecision> embed = Linalg<TPrecision>::Transpose(eigs.ev);
      eigs.ev.deallocate();
      return embed;
        
    };

};

#endif
