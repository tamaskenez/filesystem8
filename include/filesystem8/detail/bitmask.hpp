//  boost/detail/bitmask.hpp  ------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0
//  http://www.boost.org/LICENSE_1_0.txt

//  Usage:  enum foo { a=1, b=2, c=4 };
//          FILESYSTEM8_BITMASK( foo );
//
//          void f( foo arg );
//          ...
//          f( a | c );

#ifndef FILESYSTEM8_BITMASK_HPP
#define FILESYSTEM8_BITMASK_HPP

#include <cstdint>

#define FILESYSTEM8_BITMASK(Bitmask)                                            \
                                                                          \
  inline Bitmask operator| (Bitmask x , Bitmask y )                       \
  { return static_cast<Bitmask>( static_cast<std::int_least32_t>(x)     \
      | static_cast<std::int_least32_t>(y)); }                          \
                                                                          \
  inline Bitmask operator& (Bitmask x , Bitmask y )                       \
  { return static_cast<Bitmask>( static_cast<std::int_least32_t>(x)     \
      & static_cast<std::int_least32_t>(y)); }                          \
                                                                          \
  inline Bitmask operator^ (Bitmask x , Bitmask y )                       \
  { return static_cast<Bitmask>( static_cast<std::int_least32_t>(x)     \
      ^ static_cast<std::int_least32_t>(y)); }                          \
                                                                          \
  inline Bitmask operator~ (Bitmask x )                                   \
  { return static_cast<Bitmask>(~static_cast<std::int_least32_t>(x)); } \
                                                                          \
  inline Bitmask & operator&=(Bitmask & x , Bitmask y)                    \
  { x = x & y ; return x ; }                                              \
                                                                          \
  inline Bitmask & operator|=(Bitmask & x , Bitmask y)                    \
  { x = x | y ; return x ; }                                              \
                                                                          \
  inline Bitmask & operator^=(Bitmask & x , Bitmask y)                    \
  { x = x ^ y ; return x ; }                                              

#endif // FILESYSTEM8_BITMASK_HPP
