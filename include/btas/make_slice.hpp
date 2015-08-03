#ifndef __BTAS_MAKE_SLICE_HPP
#define __BTAS_MAKE_SLICE_HPP

#include <btas/TensorBase.hpp>
#include <btas/TensorWrapper.hpp>
#include <btas/TensorView.hpp>

namespace btas {

// make_slice

// For Arbitral type

template<class Arbitral>
TensorView<typename Arbitral::iterator,Arbitral::RANK,Arbitral::ORDER>
make_slice (
        Arbitral& x,
  const typename Arbitral::index_type& lower,
  const typename Arbitral::index_type& upper)
{
  typedef TensorView<typename Arbitral::iterator,Arbitral::RANK,Arbitral::ORDER> return_type;
  typename return_type::extent_type ext;
  for(size_t i = 0; i < Arbitral::RANK; ++i) ext[i] = upper[i]-lower[i]+1;
  return return_type(x.begin()+x.ordinal(lower),ext,x.stride());
}

template<class Arbitral>
TensorView<typename Arbitral::const_iterator,Arbitral::RANK,Arbitral::ORDER>
make_slice (
  const Arbitral& x,
  const typename Arbitral::index_type& lower,
  const typename Arbitral::index_type& upper)
{
  typedef TensorView<typename Arbitral::const_iterator,Arbitral::RANK,Arbitral::ORDER> return_type;
  typename return_type::extent_type ext;
  for(size_t i = 0; i < Arbitral::RANK; ++i) ext[i] = upper[i]-lower[i]+1;
  return return_type(x.begin()+x.ordinal(lower),ext,x.stride());
}

template<class Arbitral>
TensorView<typename Arbitral::const_iterator,Arbitral::RANK,Arbitral::ORDER>
make_cslice (
  const Arbitral& x,
  const typename Arbitral::index_type& lower,
  const typename Arbitral::index_type& upper)
{
  typedef TensorView<typename Arbitral::const_iterator,Arbitral::RANK,Arbitral::ORDER> return_type;
  typename return_type::extent_type ext;
  for(size_t i = 0; i < Arbitral::RANK; ++i) ext[i] = upper[i]-lower[i]+1;
  return return_type(x.begin()+x.ordinal(lower),ext,x.stride());
}

// For TensorBase

template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<T*,N,Order> make_slice (
        TensorBase<T,N,Order>& x,
  const typename TensorBase<T,N,Order>::index_type& lower,
  const typename TensorBase<T,N,Order>::index_type& upper)
{
  typename TensorView<T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<const T*,N,Order> make_slice (
  const TensorBase<T,N,Order>& x,
  const typename TensorBase<T,N,Order>::index_type& lower,
  const typename TensorBase<T,N,Order>::index_type& upper)
{
  typename TensorView<const T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<const T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

/// force to make const_slice
template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<const T*,N,Order> make_cslice (
  const TensorBase<T,N,Order>& x,
  const typename TensorBase<T,N,Order>::index_type& lower,
  const typename TensorBase<T,N,Order>::index_type& upper)
{
  typename TensorView<const T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<const T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

// For TensorWrapper

template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<T*,N,Order> make_slice (
        TensorWrapper<T*,N,Order>& x,
  const typename TensorWrapper<T*,N,Order>::index_type& lower,
  const typename TensorWrapper<T*,N,Order>::index_type& upper)
{
  typename TensorView<T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<const T*,N,Order> make_slice (
  const TensorWrapper<T*,N,Order>& x,
  const typename TensorWrapper<T*,N,Order>::index_type& lower,
  const typename TensorWrapper<T*,N,Order>::index_type& upper)
{
  typename TensorView<const T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<const T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

/// force to make const_slice
template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<const T*,N,Order> make_cslice (
  const TensorWrapper<T*,N,Order>& x,
  const typename TensorWrapper<T*,N,Order>::index_type& lower,
  const typename TensorWrapper<T*,N,Order>::index_type& upper)
{
  typename TensorView<const T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<const T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

/// force to make const_slice
template<typename T, size_t N, CBLAS_ORDER Order>
TensorView<const T*,N,Order> make_cslice (
  const TensorWrapper<const T*,N,Order>& x,
  const typename TensorWrapper<const T*,N,Order>::index_type& lower,
  const typename TensorWrapper<const T*,N,Order>::index_type& upper)
{
  typename TensorView<const T*,N,Order>::extent_type ext;
  for(size_t i = 0; i < N; ++i) ext[i] = upper[i]-lower[i]+1;
  return TensorView<const T*,N,Order>(x.data()+x.ordinal(lower),ext,x.stride());
}

} // namespace btas

#endif // __BTAS_MAKE_SLICE_HPP