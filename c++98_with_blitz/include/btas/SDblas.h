#ifndef _BTAS_SDBLAS_H
#define _BTAS_SDBLAS_H 1

#include <algorithm>
#include <functional>
#include <btas/SDArray.h>
#include <btas/Dblas.h>
#include <btas/contract_shape.h>
#include <btas/arglist.h>

#ifndef SERIAL_REPLICATION_LIMIT
#define SERIAL_REPLICATION_LIMIT 1
#endif

#ifndef SERIAL_CONTRACTION_LIMIT
#define SERIAL_CONTRACTION_LIMIT 1
#endif

namespace btas
{

//####################################################################################################
// Sparse BLAS wrappers: loop over non-zero blocks / serial computation for small # blocks
//####################################################################################################
template<int N>
void SerialSDcopy(const SDArray<N>& x, SDArray<N>& y, bool Do_up_cast)
{
  for(typename SDArray<N>::const_iterator ix = x.begin(); ix != x.end(); ++ix) {
    typename SDArray<N>::iterator iy = y.reserve(ix->first);
    if(iy != y.end())
      Dcopy(*(ix->second), *(iy->second));
    else
      BTAS_THROW(Do_up_cast, "btas::SerialSDcopy; requested block must be zero, could not be reserved");
  }
}

template<int N>
double SerialSDdot(const SDArray<N>& x, const SDArray<N>& y)
{
  double xysum = 0.0;
  for(typename SDArray<N>::const_iterator ix = x.begin(); ix != x.end(); ++ix) {
    typename SDArray<N>::const_iterator iy = y.find(ix->first);
    if(iy == y.end()) continue;
    xysum += Ddot(*(ix->second), *(iy->second));
  }
  return xysum;
}

template<int N>
void SerialSDscal(const double& alpha, SDArray<N>& x)
{
  for(typename SDArray<N>::iterator ix = x.begin(); ix != x.end(); ++ix) {
    Dscal(alpha, *(ix->second));
  }
}

template<int N>
void SerialSDaxpy(const double& alpha, const SDArray<N>& x, SDArray<N>& y)
{
  for(typename SDArray<N>::const_iterator ix = x.begin(); ix != x.end(); ++ix) {
    typename SDArray<N>::iterator iy = y.reserve(ix->first);
    if(iy == y.end())
      BTAS_THROW(false, "btas::SerialSDaxpy; requested block must be zero, could not be reserved");
    Daxpy(alpha, *(ix->second), *(iy->second));
  }
}

//####################################################################################################
// BLAS arglist for threaded call derived from T_replication_arglist
//####################################################################################################
template<int N>
class Dcopy_arglist : public T_replication_arglist<N>
{
  using T_replication_arglist<N>::m_arglist;
public:
  // calling Dcopy
  void call() const
  {
    Dcopy(*(m_arglist.first), *(m_arglist.second));
  }
  Dcopy_arglist()
  {
  }
  Dcopy_arglist(const shared_ptr<DArray<N> >& x_ptr, const shared_ptr<DArray<N> >& y_ptr)
  : T_replication_arglist<N>(x_ptr, y_ptr)
  {
  }
};

template<int N>
class Dscal_arglist : public T_replication_arglist<N>
{
  using T_replication_arglist<N>::m_arglist;
private:
  double
    m_alpha;
public:
  // calling Dscal
  void call() const
  {
    Dscal(m_alpha, *(m_arglist.second));
  }
  Dscal_arglist()
  {
  }
  Dscal_arglist(const double& alpha, const shared_ptr<DArray<N> >& x_ptr, const shared_ptr<DArray<N> >& y_ptr)
  : m_alpha(alpha), T_replication_arglist<N>(x_ptr, y_ptr)
  {
  }
  void reset(const double& alpha, const shared_ptr<DArray<N> >& x_ptr, const shared_ptr<DArray<N> >& y_ptr)
  {
    m_alpha = alpha;
    T_replication_arglist<N>::reset(x_ptr, y_ptr);
  }
};

template<int N>
class Daxpy_arglist : public T_replication_arglist<N>
{
  using T_replication_arglist<N>::m_arglist;
private:
  double
    m_alpha;
public:
  // calling Daxpy
  void call() const
  {
    Daxpy(m_alpha, *(m_arglist.first), *(m_arglist.second));
  }
  Daxpy_arglist()
  {
  }
  Daxpy_arglist(const double& alpha, const shared_ptr<DArray<N> >& x_ptr, const shared_ptr<DArray<N> >& y_ptr)
  : m_alpha(alpha), T_replication_arglist<N>(x_ptr, y_ptr)
  {
  }
  void reset(const double& alpha, const shared_ptr<DArray<N> >& x_ptr, const shared_ptr<DArray<N> >& y_ptr)
  {
    m_alpha = alpha;
    T_replication_arglist<N>::reset(x_ptr, y_ptr);
  }
};

//####################################################################################################
// BLAS arglist for threaded call derived from T_contraction_arglist
//####################################################################################################
template<int NA, int NB, int NC>
class Dgemv_arglist : public T_contraction_arglist<NA, NB, NC>
{
  using T_contraction_arglist<NA, NB, NC>::m_arglist;
  using T_contraction_arglist<NA, NB, NC>::m_c_ptr;
private:
  std::vector<double>
    m_scale;
  BTAS_TRANSPOSE
    m_transa;
  double
    m_alpha;
  double
    m_beta;
public:
  // calling blas subroutine from derived arglist
  void call() const
  {
    for(int i = 0; i < m_arglist.size(); ++i) {
      Dgemv(m_transa, m_scale[i] * m_alpha, *(m_arglist[i].first), *(m_arglist[i].second), m_beta, *(m_c_ptr));
    }
  }
  Dgemv_arglist()
  : m_transa(NoTrans), m_alpha(1.0), m_beta(1.0)
  {
  }
  Dgemv_arglist(const BTAS_TRANSPOSE& transa, const double& alpha, const double& beta)
  : m_transa(transa), m_alpha(alpha), m_beta(beta)
  {
  }
  void add(const shared_ptr<DArray<NA> >& a_ptr, const shared_ptr<DArray<NB> >& b_ptr, double scale = 1.0)
  {
    m_scale.push_back(scale);
    T_contraction_arglist<NA, NB, NC>::add(a_ptr, b_ptr, a_ptr->size());
  }
};

template<int NA, int NB, int NC>
class Dger_arglist : public T_contraction_arglist<NA, NB, NC>
{
  using T_contraction_arglist<NA, NB, NC>::m_arglist;
  using T_contraction_arglist<NA, NB, NC>::m_c_ptr;
private:
  std::vector<double>
    m_scale;
  double
    m_alpha;
public:
  // calling blas subroutine from derived arglist
  void call() const
  {
    for(int i = 0; i < m_arglist.size(); ++i) {
      Dger(m_scale[i] * m_alpha, *(m_arglist[i].first), *(m_arglist[i].second), *(m_c_ptr));
    }
  }
  Dger_arglist()
  : m_alpha(1.0)
  {
  }
  Dger_arglist(const double& alpha)
  : m_alpha(alpha)
  {
  }
  void add(const shared_ptr<DArray<NA> >& a_ptr, const shared_ptr<DArray<NB> >& b_ptr, double scale = 1.0)
  {
    m_scale.push_back(scale);
    T_contraction_arglist<NA, NB, NC>::add(a_ptr, b_ptr, a_ptr->size());
  }
};

template<int NA, int NB, int NC>
class Dgemm_arglist : public T_contraction_arglist<NA, NB, NC>
{
  using T_contraction_arglist<NA, NB, NC>::m_arglist;
  using T_contraction_arglist<NA, NB, NC>::m_c_ptr;
private:
  std::vector<double>
    m_scale;
  BTAS_TRANSPOSE
    m_transa;
  BTAS_TRANSPOSE
    m_transb;
  double
    m_alpha;
  double
    m_beta;
  // compute flop count
  long mf_compute_flops(const DArray<NA>& a, const DArray<NB>& b) const
  {
    const int K = ( NA + NB - NC ) / 2;
    const TinyVector<int, NB>& b_shape = b.shape();
    long flops = a.size();
    if(m_transb == NoTrans) {
      flops = std::accumulate(b_shape.begin()+K, b_shape.end(), flops, std::multiplies<long>());
    }
    else {
      flops = std::accumulate(b_shape.begin(), b_shape.begin()+NB-K, flops, std::multiplies<long>());
    }
    return flops;
  }
public:
  // calling blas subroutine from derived arglist
  void call() const
  {
    for(int i = 0; i < m_arglist.size(); ++i) {
      Dgemm(m_transa, m_transb, m_scale[i] * m_alpha, *(m_arglist[i].first), *(m_arglist[i].second), m_beta, *(m_c_ptr));
    }
  }
  Dgemm_arglist()
  : m_transa(NoTrans), m_transb(NoTrans), m_alpha(1.0), m_beta(1.0)
  {
  }
  Dgemm_arglist(const BTAS_TRANSPOSE& transa, const BTAS_TRANSPOSE& transb, const double& alpha, const double& beta)
  : m_transa(transa), m_transb(transb), m_alpha(alpha), m_beta(beta)
  {
  }
  void add(const shared_ptr<DArray<NA> >& a_ptr, const shared_ptr<DArray<NB> >& b_ptr, double scale = 1.0)
  {
    m_scale.push_back(scale);
    T_contraction_arglist<NA, NB, NC>::add(a_ptr, b_ptr, mf_compute_flops(*a_ptr, *b_ptr));
  }
};

//####################################################################################################
// Sparse BLAS wrappers: loop over non-zero blocks / threaded by OpenMP
//####################################################################################################
template<int N>
void ThreadSDcopy(const SDArray<N>& x, SDArray<N>& y, bool Do_up_cast)
{
  std::vector<Dcopy_arglist<N> > task_list;
  task_list.reserve(x.size());
  for(typename SDArray<N>::const_iterator ix = x.begin(); ix != x.end(); ++ix) {
    typename SDArray<N>::iterator iy = y.reserve(ix->first);
    if(iy != y.end())
      task_list.push_back(Dcopy_arglist<N>(ix->second, iy->second));
    else
      BTAS_THROW(Do_up_cast, "btas::ThreadSDcopy; requested block must be zero, could not be reserved");
  }
  parallel_call(task_list);
}

template<int N>
void ThreadSDscal(const double& alpha, SDArray<N>& x)
{
  std::vector<Dscal_arglist<N> > task_list;
  task_list.reserve(x.size());
  for(typename SDArray<N>::iterator ix = x.begin(); ix != x.end(); ++ix) {
    task_list.push_back(Dscal_arglist<N>(alpha, ix->second, ix->second));
  }
  parallel_call(task_list);
}

template<int N>
void ThreadSDaxpy(const double& alpha, const SDArray<N>& x, SDArray<N>& y)
{
  std::vector<Daxpy_arglist<N> > task_list;
  task_list.reserve(x.size());
  for(typename SDArray<N>::const_iterator ix = x.begin(); ix != x.end(); ++ix) {
    typename SDArray<N>::iterator iy = y.reserve(ix->first);
    if(iy == y.end())
      BTAS_THROW(false, "btas::ThreadSDaxpy; requested block must be zero, could not be reserved");
    task_list.push_back(Daxpy_arglist<N>(alpha, ix->second, iy->second));
  }
  parallel_call(task_list);
}

template<int NA, int NB, int NC>
void ThreadSDgemv(const BTAS_TRANSPOSE& transa,
                  const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  int nrows  = std::accumulate(a.shape().begin(), a.shape().begin()+NC, 1, std::multiplies<int>());
  int stride = std::accumulate(b.shape().begin(), b.shape().end(),      1, std::multiplies<int>());
  // contraction list for thread parallelism
  std::vector<Dgemv_arglist<NA, NB, NC> > task_list;
  task_list.reserve(a.size());
  // block contraction
  for(int i = 0; i < nrows; ++i) {
    int a_lbound = i * stride;
    int a_ubound = a_lbound + stride - 1;
    typename SDArray<NA>::const_iterator ialo = a.lower_bound(a_lbound);
    typename SDArray<NA>::const_iterator iaup = a.upper_bound(a_ubound);
    if(ialo == iaup) continue;
    if(!c.allowed(i)) continue;

    Dgemv_arglist<NA, NB, NC> gemv_list(transa, alpha, 1.0);
    for(typename SDArray<NA>::const_iterator ia = ialo; ia != iaup; ++ia) {
      typename SDArray<NB>::const_iterator ib = b.find(ia->first % stride);
      if(ib != b.end()) {
        gemv_list.add(ia->second, ib->second);
      }
    }
    if(gemv_list.size() == 0) continue;

    // allocate block element @ i
    typename SDArray<NC>::iterator ic = c.reserve(i);
    if(ic == c.end())
      BTAS_THROW(false, "btas::ThreadSDgemv required block could not be allocated");

    gemv_list.set(ic->second);
    task_list.push_back(gemv_list);
  }
  parallel_call(task_list);
}

template<int NA, int NB, int NC>
void ThreadSDger(const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  int stride = std::accumulate(b.shape().begin(), b.shape().end(), 1, std::multiplies<int>());
  // contraction list for thread parallelism
  std::vector<Dger_arglist<NA, NB, NC> > task_list;
  task_list.reserve(a.size() * b.size());
  // block contraction
  for(typename SDArray<NA>::const_iterator ia = a.begin(); ia != a.end(); ++ia) {
    int c_irow = ia->first * stride;
    for(typename SDArray<NB>::const_iterator ib = b.begin(); ib != b.end(); ++ib) {
      int c_tag = c_irow + ib->first;
      if(!c.allowed(c_tag)) continue;

      Dger_arglist<NA, NB, NC> ger_list(alpha);
      ger_list.add(ia->second, ib->second);

      // allocate block element @ c_tag
      typename SDArray<NC>::iterator ic = c.reserve(c_tag);
      if(ic == c.end())
        BTAS_THROW(false, "btas::ThreadSDger required block could not be allocated");

      ger_list.set(ic->second);
      task_list.push_back(ger_list);
    }
  }
  parallel_call(task_list);
}

template<int NA, int NB, int NC>
void ThreadSDgemm(const BTAS_TRANSPOSE& transa, const BTAS_TRANSPOSE& transb,
                  const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  const int K = ( NA + NB - NC ) / 2;
  int nrows  = std::accumulate(a.shape().begin(), a.shape().begin()+NA-K, 1, std::multiplies<int>());
  int stride = std::accumulate(a.shape().begin()+NA-K, a.shape().end(),   1, std::multiplies<int>());
  int ncols  = std::accumulate(b.shape().begin(), b.shape().begin()+NB-K, 1, std::multiplies<int>());
  // contraction list for thread parallelism
  std::vector<Dgemm_arglist<NA, NB, NC> > task_list;
  task_list.reserve(std::max(a.size(), b.size()));
  // block contraction
  for(int i = 0; i < nrows; ++i) {
    int a_lbound = i * stride;
    int a_ubound = a_lbound + stride - 1;
    typename SDArray<NA>::const_iterator ialo = a.lower_bound(a_lbound);
    typename SDArray<NA>::const_iterator iaup = a.upper_bound(a_ubound);
    if(ialo == iaup) continue;

    int c_irow = i * ncols;
    for(int j = 0; j < ncols; ++j) {
      int c_tag = c_irow + j;
      if(!c.allowed(c_tag)) continue;

      int b_lbound = j * stride;
      int b_ubound = b_lbound + stride - 1;
      typename SDArray<NB>::const_iterator iblo = b.lower_bound(b_lbound);
      typename SDArray<NB>::const_iterator ibup = b.upper_bound(b_ubound);
      if(iblo == ibup) continue;

      Dgemm_arglist<NA, NB, NC> gemm_list(transa, transb, alpha, 1.0);
      for(typename SDArray<NA>::const_iterator ia = ialo; ia != iaup; ++ia) {
        for(typename SDArray<NB>::const_iterator ib = iblo; ib != ibup; ++ib) {
          if((ia->first % stride) == (ib->first % stride)) {
            gemm_list.add(ia->second, ib->second);
          }
        }
      }
      if(gemm_list.size() == 0) continue;

      // allocate block element @ c_tag
      typename SDArray<NC>::iterator ic = c.reserve(c_tag);
      if(ic == c.end())
        BTAS_THROW(false, "btas::ThreadSDgemm required block could not be allocated");

      gemm_list.set(ic->second);
      task_list.push_back(gemm_list);
    }
  }
  parallel_call(task_list);
}

//####################################################################################################
// Sparse BLAS wrappers with index-based contraction scaling functor
//####################################################################################################

template<int NA, int NB, int NC>
void ThreadSDgemv(const function<double(const TinyVector<int, NA>&,
                                        const TinyVector<int, NB>&,
                                        const TinyVector<int, NC>&)>& f_scale,
                  const BTAS_TRANSPOSE& transa,
                  const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  int nrows  = std::accumulate(a.shape().begin(), a.shape().begin()+NC, 1, std::multiplies<int>());
  int stride = std::accumulate(b.shape().begin(), b.shape().end(),      1, std::multiplies<int>());
  // contraction list for thread parallelism
  std::vector<Dgemv_arglist<NA, NB, NC> > task_list;
  task_list.reserve(a.size());
  // block contraction
  for(int i = 0; i < nrows; ++i) {
    int a_lbound = i * stride;
    int a_ubound = a_lbound + stride - 1;
    typename SDArray<NA>::const_iterator ialo = a.lower_bound(a_lbound);
    typename SDArray<NA>::const_iterator iaup = a.upper_bound(a_ubound);
    if(ialo == iaup) continue;
    if(!c.allowed(i)) continue;

    TinyVector<int, NC> c_index(c.index(i));
    Dgemv_arglist<NA, NB, NC> gemv_list(transa, alpha, 1.0);
    for(typename SDArray<NA>::const_iterator ia = ialo; ia != iaup; ++ia) {
      typename SDArray<NB>::const_iterator ib = b.find(ia->first % stride);
      if(ib != b.end()) {
        gemv_list.add(ia->second, ib->second, f_scale(a.index(ia->first), b.index(ib->first), c_index));
      }
    }
    if(gemv_list.size() == 0) continue;

    // allocate block element @ i
    typename SDArray<NC>::iterator ic = c.reserve(i);
    if(ic == c.end())
      BTAS_THROW(false, "btas::ThreadSDgemv required block could not be allocated");

    gemv_list.set(ic->second);
    task_list.push_back(gemv_list);
  }
  parallel_call(task_list);
}

template<int NA, int NB, int NC>
void ThreadSDger(const function<double(const TinyVector<int, NA>&,
                                       const TinyVector<int, NB>&,
                                       const TinyVector<int, NC>&)>& f_scale,
                 const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  int stride = std::accumulate(b.shape().begin(), b.shape().end(), 1, std::multiplies<int>());
  // contraction list for thread parallelism
  std::vector<Dger_arglist<NA, NB, NC> > task_list;
  task_list.reserve(a.size() * b.size());
  // block contraction
  for(typename SDArray<NA>::const_iterator ia = a.begin(); ia != a.end(); ++ia) {
    int c_irow = ia->first * stride;
    TinyVector<int, NA> a_index(a.index(ia->first));

    for(typename SDArray<NB>::const_iterator ib = b.begin(); ib != b.end(); ++ib) {
      int c_tag = c_irow + ib->first;
      if(!c.allowed(c_tag)) continue;

      Dger_arglist<NA, NB, NC> ger_list(alpha);
      ger_list.add(ia->second, ib->second, f_scale(a_index, b.index(ib->first), c.index(c_tag)));

      // allocate block element @ c_tag
      typename SDArray<NC>::iterator ic = c.reserve(c_tag);
      if(ic == c.end())
        BTAS_THROW(false, "btas::ThreadSDger required block could not be allocated");

      ger_list.set(ic->second);
      task_list.push_back(ger_list);
    }
  }
  parallel_call(task_list);
}

template<int NA, int NB, int NC>
void ThreadSDgemm(const function<double(const TinyVector<int, NA>&,
                                        const TinyVector<int, NB>&,
                                        const TinyVector<int, NC>&)>& f_scale,
                  const BTAS_TRANSPOSE& transa, const BTAS_TRANSPOSE& transb,
                  const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  const int K = ( NA + NB - NC ) / 2;
  int nrows  = std::accumulate(a.shape().begin(), a.shape().begin()+NA-K, 1, std::multiplies<int>());
  int stride = std::accumulate(a.shape().begin()+NA-K, a.shape().end(),   1, std::multiplies<int>());
  int ncols  = std::accumulate(b.shape().begin(), b.shape().begin()+NB-K, 1, std::multiplies<int>());
  // contraction list for thread parallelism
  std::vector<Dgemm_arglist<NA, NB, NC> > task_list;
  task_list.reserve(std::max(a.size(), b.size()));
  // block contraction
  for(int i = 0; i < nrows; ++i) {
    int a_lbound = i * stride;
    int a_ubound = a_lbound + stride - 1;
    typename SDArray<NA>::const_iterator ialo = a.lower_bound(a_lbound);
    typename SDArray<NA>::const_iterator iaup = a.upper_bound(a_ubound);
    if(ialo == iaup) continue;

    int c_irow = i * ncols;
    for(int j = 0; j < ncols; ++j) {
      int c_tag = c_irow + j;
      if(!c.allowed(c_tag)) continue;

      int b_lbound = j * stride;
      int b_ubound = b_lbound + stride - 1;
      typename SDArray<NB>::const_iterator iblo = b.lower_bound(b_lbound);
      typename SDArray<NB>::const_iterator ibup = b.upper_bound(b_ubound);
      if(iblo == ibup) continue;

      Dgemm_arglist<NA, NB, NC> gemm_list(transa, transb, alpha, 1.0);
      for(typename SDArray<NA>::const_iterator ia = ialo; ia != iaup; ++ia) {
        TinyVector<int, NA> a_index(a.index(ia->first));
        for(typename SDArray<NB>::const_iterator ib = iblo; ib != ibup; ++ib) {
          if((ia->first % stride) == (ib->first % stride)) {
            gemm_list.add(ia->second, ib->second, f_scale(a_index, b.index(ib->first), c.index(c_tag)));
          }
        }
      }
      if(gemm_list.size() == 0) continue;

      // allocate block element @ c_tag
      typename SDArray<NC>::iterator ic = c.reserve(c_tag);
      if(ic == c.end())
        BTAS_THROW(false, "btas::ThreadSDgemm required block could not be allocated");

      gemm_list.set(ic->second);
      task_list.push_back(gemm_list);
    }
  }
  parallel_call(task_list);
}

//####################################################################################################
// BLAS-like interfaces to SDArray contractions
//####################################################################################################

//
// BLAS level 1
//

// Do_up_cast: enables to up-cast for QSDArray (copying only non-zero candidates)
template<int N>
void SDcopy(const SDArray<N>& x, SDArray<N>& y, bool Do_up_cast = false)
{
  if(Do_up_cast) {
    if(x.shape() != y.shape()) {
      BTAS_THROW(false, "btas::SDcopy; array shape mismatched despite up-casting was specified");
    }
  }
  else {
    y.resize(x.shape());
  }
#ifdef SERIAL
  SerialSDcopy(x, y, Do_up_cast);
#else
  ThreadSDcopy(x, y, Do_up_cast);
#endif
}

template<int N>
void SDscal(const double& alpha, SDArray<N>& x)
{
#ifdef SERIAL
  SerialSDscal(alpha, x);
#else
  ThreadSDscal(alpha, x);
#endif
}

template<int N>
double SDdot(const SDArray<N>& x, const SDArray<N>& y)
{
  if(!std::equal(x.shape().begin(), x.shape().end(), y.shape().begin()))
    BTAS_THROW(false, "btas::SDdot: sizes of x and y mismatched");
  return SerialSDdot(x, y);
}

template<int N>
void SDaxpy(const double& alpha, const SDArray<N>& x, SDArray<N>& y)
{
  const TinyVector<int, N>& x_shape = x.shape();
  if(y.size() > 0) {
    if(!std::equal(x_shape.begin(), x_shape.end(), y.shape().begin()))
      BTAS_THROW(false, "btas::SDaxpy: size of y mismatched");
  }
  else {
    y.resize(x_shape);
  }
#ifdef SERIAL
  SerialSDaxpy(alpha, x, y);
#else
  ThreadSDaxpy(alpha, x, y);
#endif
}

//
// BLAS level 2
//
template<int NA, int NB, int NC>
void SDgemv(const BTAS_TRANSPOSE& transa,
            const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, const double& beta, SDArray<NC>& c)
{
  // check/resize contraction shape
  TinyVector<int, NC> c_shape;
  gemv_contract_shape(transa, a.shape(), b.shape(), c_shape);
  if(c.size() > 0) {
    if(!std::equal(c_shape.begin(), c_shape.end(), c.shape().begin()))
      BTAS_THROW(false, "btas::SDgemv: size of c mismatched");
    SDscal(beta, c);
  }
  else {
    c.resize(c_shape);
  }
  // calling block-sparse blas wrapper
  if(transa == NoTrans) {
    ThreadSDgemv(transa, alpha, a, b, c);
  }
  else {
    TinyVector<int, NA> aperm;
    for(int i = 0; i < NB;  ++i) aperm[i+NC] = i;
    for(int i = NB; i < NA; ++i) aperm[i-NB] = i;
    ThreadSDgemv(transa, alpha, a.transpose_view(NB), b, c);
  }
}

template<int NA, int NB, int NC>
void SDger(const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, SDArray<NC>& c)
{
  // check/resize contraction shape
  TinyVector<int, NC> c_shape;
  ger_contract_shape(a.shape(), b.shape(), c_shape);
  if(c.size() > 0) {
    if(!std::equal(c_shape.begin(), c_shape.end(), c.shape().begin()))
      BTAS_THROW(false, "btas::SDger: size of c mismatched");
  }
  else {
    c.resize(c_shape);
  }
  // calling block-sparse blas wrapper
  ThreadSDger(alpha, a, b, c);
}

//
// BLAS level 3
//
template<int NA, int NB, int NC>
void SDgemm(const BTAS_TRANSPOSE& transa, const BTAS_TRANSPOSE& transb,
            const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b, const double& beta, SDArray<NC>& c)
{
  const int K = (NA + NB - NC) / 2;
  // check/resize contraction shape
  TinyVector<int, K> contracts;
  TinyVector<int, NC> c_shape;
  gemm_contract_shape(transa, transb, a.shape(), b.shape(), contracts, c_shape);
  if(c.size() > 0) {
    if(!std::equal(c_shape.begin(), c_shape.end(), c.shape().begin()))
      BTAS_THROW(false, "btas::SDgemm: size of c mismatched");
    SDscal(beta, c);
  }
  else {
    c.resize(c_shape);
  }
  // calling block-sparse blas wrapper
  if(transa == NoTrans && transb == NoTrans) {
    ThreadSDgemm(transa, transb, alpha, a, b.transpose_view(K), c);
  }
  else if(transa == NoTrans && transb != NoTrans) {
    ThreadSDgemm(transa, transb, alpha, a, b, c);
  }
  else if(transa != NoTrans && transb == NoTrans) {
    ThreadSDgemm(transa, transb, alpha, a.transpose_view(K), b.transpose_view(K), c);
  }
  else if(transa != NoTrans && transb != NoTrans) {
    ThreadSDgemm(transa, transb, alpha, a.transpose_view(K), b, c);
  }
}

// (general matrix) * (diagonal matrix)
template<int NA, int NB>
void SDdimd(SDArray<NA>& a, const SDArray<NB>& b)
{
  int stride = b.size_total();
  for(typename SDArray<NA>::iterator ia = a.begin(); ia != a.end(); ++ia) {
    typename SDArray<NB>::const_iterator ib = b.find(ia->first % stride);
    if(ib != b.end()) Ddimd((*ia->second), (*ib->second));
  }
}

// (diagonal matrix) * (general matrix)
template<int NA, int NB>
void SDdidm(const SDArray<NA>& a, SDArray<NB>& b)
{
  const TinyVector<int, NB>& b_shape = b.shape();
  int stride = std::accumulate(b_shape.begin()+NA, b_shape.end(), 1, std::multiplies<int>());
  for(typename SDArray<NB>::iterator ib = b.begin(); ib != b.end(); ++ib) {
    typename SDArray<NA>::const_iterator ia = a.find(ib->first / stride);
    if(ia != a.end()) Ddidm((*ia->second), (*ib->second));
  }
}

// BLAS WRAPPER
template<int NA, int NB, int NC>
void SDblas_wrapper(const double& alpha, const SDArray<NA>& a, const SDArray<NB>& b,
                    const double& beta, SDArray<NC>& c)
{
  const int CNT = (NA + NB - NC) / 2;

  if(NA == CNT) {
    SDgemv(Trans,   alpha, b, a, beta, c);
  }
  else if(NB == CNT) {
    SDgemv(NoTrans, alpha, a, b, beta, c);
  }
  else {
    SDgemm(NoTrans, NoTrans, alpha, a, b, beta, c);
  }
}

}; // namespace btas

#endif // _BTAS_SDBLAS_H
