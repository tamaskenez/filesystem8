//  boost/filesystem/v3/config.hpp  ----------------------------------------------------//

//  Copyright Beman Dawes 2003

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------// 

#ifndef FILESYSTEM8_CONFIG_HPP
#define FILESYSTEM8_CONFIG_HPP

#include <filesystem8/export.h>

#if __cplusplus > 201100 || _MSC_VER >= 1900
#define FILESYSTEM8_CONSTEXPR_OR_CONST constexpr
#define FILESYSTEM8_NOEXCEPT noexcept
#define FILESYSTEM8_NOEXCEPT_OR_NOTHROW noexcept
#else
#define FILESYSTEM8_CONSTEXPR_OR_CONST const
#define FILESYSTEM8_NOEXCEPT
#define FILESYSTEM8_NOEXCEPT_OR_NOTHROW throw()
#endif

#define FILESYSTEM8_STATIC_CONSTEXPR static FILESYSTEM8_CONSTEXPR_OR_CONST


#define FILESYSTEM8_ASSERT(C) (C?(void)0:std::terminate())
#define FILESYSTEM8_ASSERT_MSG(C,M) (C?(void)0:std::terminate())

#ifdef _WIN32
#define FILESYSTEM8_WINDOWS_API
#else
#define FILESYSTEM8_POSIX_API
#endif

#define FILESYSTEM8_THROW(EX) throw EX

// #define BOOST_NO_CXX11_RVALUE_REFERENCES

namespace filesystem8 {}

#endif // FILESYSTEM8_CONFIG_HPP
