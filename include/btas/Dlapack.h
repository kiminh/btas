#ifndef _BTAS_DLAPACK_H
#define _BTAS_DLAPACK_H 1

#include <algorithm>
#include <numeric>
#include <btas/DArray.h>
#include <btas/Dblas.h>
#include <btas/lapack_cxx_interface.h>

extern "C"
{
#include <mkl_cblas.h>
#include <mkl_lapack.h>
}

namespace btas
{

//
// solve linear equation Ax = b
//
template<int N>
void Dgesv(const DArray<2*N>& a, DArray<N>& x)
{
  TinyVector<int, N> x_shape;
  for(int i = 0; i < N; ++i) x_shape[i] = a.extent(i);
  if(x.data()) {
    if(!std::equal(x_shape.begin(), x_shape.end(), x.shape().begin()))
      BTAS_THROW(false, "btas::Dgesv array size is not consistent");
  }
  else {
    x.resize(x_shape);
    x = 0.0f;
  }
  DArray<2*N> acopy;
  Dcopy(a, acopy);
  int ndim = std::accumulate(x_shape.begin(), x_shape.end(), 1, std::multiplies<int>());
  pivot_info ipiv;
  if(clapack_dgesv(ndim, 1, acopy.data(), ndim, ipiv, x.data(), ndim) < 0)
    BTAS_THROW(false, "btas::Dgesv terminated abnormally");
}

//
// series of eigenvalue decomposition:
// + input matrix : (2*N-2)D-array regarding as a square matrix
// + eigenvalue   : stored as 1D-array
// + eigenvector  : stored as (N)D-array where the 1st dim. is used as eigenstate index
//

//
// full diagonalization for real symmetric tensor:
//
template<int N>
void Dsyev(const DArray<2*N-2>& a, DArray<1>& d, DArray<N>& z, CLAPACK_CALCVECTOR jobt = ClapackCalcVector)
{
  if(!a.data()) BTAS_THROW(false, "btas::Dsyev(tensor) array data not found");
  const TinyVector<int, 2*N-2>& a_shape = a.shape();
  TinyVector<int, N-1> column_shape;
  for(int i = 0; i < N-1; ++i) column_shape[i] = a_shape[i];
  int ncols = std::accumulate(column_shape.begin(), column_shape.end(), 1, std::multiplies<int>());
  TinyVector<int, N> z_shape;
  z_shape[0] = ncols;
  for(int i = 0; i < N-1; ++i) z_shape[i+1] = column_shape[i];
  Dreshape(a, z_shape, z);
  d.resize(ncols);
  if(clapack_dsyev(jobt, ClapackUseUpper, ncols, z.data(), ncols, d.data()) < 0)
    BTAS_THROW(false, "btas::Dsyev(tensor) terminated abnormally");
}

//
// solve eigenvalue problem for non-hermitian matrix
//
template<int N>
void Dgeev(const DArray<2*N-2>& a, DArray<1>& wr, DArray<1>& wi, DArray<N>& vl, DArray<N>& vr, CLAPACK_CALCVECTOR jobt = ClapackCalcVector)
{
  if(!a.data()) BTAS_THROW(false, "btas::Dgeev(tensor) array data of 'a' not found");
  DArray<2*N-2> ascr; Dcopy(a, ascr);
  const TinyVector<int, 2*N-2>& a_shape = a.shape();
  TinyVector<int, N-1> column_shape;
  for(int i = 0; i < N-1; ++i) column_shape[i] = a_shape[i];
  int ncols = std::accumulate(column_shape.begin(), column_shape.end(), 1, std::multiplies<int>());
  TinyVector<int, N> v_shape;
  v_shape[0] = ncols;
  for(int i = 0; i < N-1; ++i) v_shape[i+1] = column_shape[i];
  vl.resize(v_shape); vl = 0.0;
  vr.resize(v_shape); vr = 0.0;
  wr.resize(ncols); wr = 0.0;
  wi.resize(ncols); wi = 0.0;
  if(clapack_dgeev(jobt, jobt, ncols, ascr.data(), ncols, wr.data(), wi.data(), vl.data(), ncols, vr.data(), ncols) < 0)
    BTAS_THROW(false, "btas::Dgeev(tensor) terminated abnormally");
}

//
// solve generalized eigenvalue problem for real-symmetric tensor
//
template<int N>
void Dsygv(const DArray<2*N-2>& a, const DArray<2*N-2>& b, DArray<1>& d, DArray<N>& z, CLAPACK_CALCVECTOR jobt = ClapackCalcVector)
{
  if(!a.data()) BTAS_THROW(false, "btas::Dsygv(tensor) array data of 'a' not found");
  if(!b.data()) BTAS_THROW(false, "btas::Dsygv(tensor) array data of 'b' not found");
  const TinyVector<int, 2*N-2>& a_shape = a.shape();
  if(!std::equal(a_shape.begin(), a_shape.end(), b.shape().begin()))
    BTAS_THROW(false, "btas::Dsygv(tensor) shapes of 'a' and 'b' are inconsistent");
  TinyVector<int, N-1> column_shape;
  for(int i = 0; i < N-1; ++i) column_shape[i] = a_shape[i];
  int ncols = std::accumulate(column_shape.begin(), column_shape.end(), 1, std::multiplies<int>());
  TinyVector<int, N> z_shape;
  z_shape[0] = ncols;
  for(int i = 0; i < N-1; ++i) z_shape[i+1] = column_shape[i];
  Dreshape(a, z_shape, z);
  DArray<2*N-2> x;
  Dcopy(b, x);
  d.resize(ncols);
  if(clapack_dsygv(1, jobt, ClapackUseUpper, ncols, z.data(), ncols, x.data(), ncols, d.data()) < 0)
    BTAS_THROW(false, "btas::Dsygv(tensor) terminated abnormally");
}

//
// solve generalized eigenvalue problem for non-hermitian matrix pair
//
template<int N>
void Dggev(const DArray<2*N-2>& a, const DArray<2*N-2>& b, DArray<1>& alphar, DArray<1>& alphai, DArray<1>& beta,
           DArray<N>& vl, DArray<N>& vr, CLAPACK_CALCVECTOR jobt = ClapackCalcVector)
{
  if(!a.data()) BTAS_THROW(false, "btas::Dggev(tensor) array data of 'a' not found");
  if(!b.data()) BTAS_THROW(false, "btas::Dggev(tensor) array data of 'b' not found");
  DArray<2*N-2> ascr; Dcopy(a, ascr);
  DArray<2*N-2> bscr; Dcopy(b, bscr);
  const TinyVector<int, 2*N-2>& a_shape = a.shape();
  if(!std::equal(a_shape.begin(), a_shape.end(), b.shape().begin()))
    BTAS_THROW(false, "btas::Dggev(tensor) shapes of 'a' and 'b' are inconsistent");
  TinyVector<int, N-1> column_shape;
  for(int i = 0; i < N-1; ++i) column_shape[i] = a_shape[i];
  int ncols = std::accumulate(column_shape.begin(), column_shape.end(), 1, std::multiplies<int>());
  TinyVector<int, N> v_shape;
  v_shape[0] = ncols;
  for(int i = 0; i < N-1; ++i) v_shape[i+1] = column_shape[i];
  vl.resize(v_shape); vl = 0.0;
  vr.resize(v_shape); vr = 0.0;
  alphar.resize(ncols); alphar = 0.0;
  alphai.resize(ncols); alphai = 0.0;
  beta.resize(ncols); beta = 0.0;
  if(clapack_dggev(jobt, jobt, ncols, ascr.data(), ncols, bscr.data(), ncols,
                   alphar.data(), alphai.data(), beta.data(), vl.data(), ncols, vr.data(), ncols) < 0)
    BTAS_THROW(false, "btas::Dggev(tensor) terminated abnormally");
}

//
// specialized for matrix
//
//template<>
//void Dsyev<1>(const DArray<2>& a, DArray<1>& d, DArray<2>& z)
//{
//  if(!a.data()) BTAS_THROW(false, "btas::Dsyev(matrix) array data not found");
//  Dcopy(a, z);
//  int ncols = a.cols();
//  d.resize(ncols);
//  if(clapack_dsyev(ClapackCalcVector, ClapackUseUpper, ncols, z.data(), ncols, d.data()) < 0)
//    BTAS_THROW(false, "btas::Dsyev(tensor) terminated abnormally");
//}

//
// thin singular value decomposition
//
template<int NA, int NU>
void Dgesvd(const DArray<NA>& a, DArray<1>& s, DArray<NU>& u, DArray<NA-NU+2>& vt, CLAPACK_CALCVECTOR jobt = ClapackCalcThinVector)
{
  const int K  = NA - NU + 1;
  const int NV = K + 1;
  if(!a.data()) BTAS_THROW(false, "btas::Dgesvd(tensor) array data not found");
  const TinyVector<int, NA>& a_shape = a.shape();
  int nrows = std::accumulate(a_shape.begin(),     a_shape.begin()+NA-K, 1, std::multiplies<int>());
  int ncols = std::accumulate(a_shape.begin()+NA-K, a_shape.end(),       1, std::multiplies<int>());
  int nsval = std::min(nrows, ncols);
  TinyVector<int, NU> u_shape;
  for(int i = 0; i < NA-K; ++i) u_shape[i] = a_shape[i];
  int ucols = (jobt == ClapackCalcThinVector) ? nsval : nrows;
  u_shape[NA-K] = ucols;
  TinyVector<int, NV> vt_shape;
  int vrows = (jobt == ClapackCalcThinVector) ? nsval : ncols;
  vt_shape[0] = vrows;
  for(int i = 0; i < K; ++i) vt_shape[i+1] = a_shape[i+NA-K];
  s.resize(nsval);
  u.resize(u_shape);
  vt.resize(vt_shape);
  DArray<NA> acpy;
  Dcopy(a, acpy);
  int info = clapack_dgesvd(jobt, jobt, nrows, ncols, acpy.data(), ncols, s.data(), u.data(), ucols, vt.data(), ncols);
  if(info < 0) BTAS_THROW(false, "btas::Dgesvd(tensor) terminated abnormally");
}

//
// specialized for matrix
//
//template<>
//void Dgesvd<2, 2>(const DArray<2>& a, DArray<1>& s, DArray<2>& u, DArray<2>& vt)
//{
//  if(!a.data()) BTAS_THROW(false, "btas::Dgesvd(matrix) array data not found");
//  int nrows = a.rows();
//  int ncols = a.cols();
//  int nsval = std::min(nrows, ncols);
//  s.resize(nsval);
//  u.resize(nrows, nsval);
//  vt.resize(nsval, ncols);
//  DArray<2> aref; aref.reference(a);
//  int info = clapack_dgesvd(ClapackCalcThinVector, ClapackCalcThinVector,
//                            nrows, ncols, aref.data(), ncols, s.data(), u.data(), nsval, vt.data(), ncols);
//  if(info < 0) BTAS_THROW(false, "btas::Dgesvd(matrix) terminated abnormally");
//}

}; // namespace btas

#endif // _BTAS_DLAPACK_H
