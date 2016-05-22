//  boost/filesystem/v3/config.hpp  ----------------------------------------------------//

//  Copyright Beman Dawes 2003

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------// 

#ifndef BOOST_FILESYSTEM3_CONFIG_HPP
#define BOOST_FILESYSTEM3_CONFIG_HPP

#include <filesystem8/export.h>
#undef BOOST_FILESYSTEM_DEPRECATED
#define BOOST_FILESYSTEM_DECL BOOST_FILESYSTEM_EXPORT
#define BOOST_SYMBOL_VISIBLE BOOST_FILESYSTEM_EXPORT

#define BOOST_CONSTEXPR_OR_CONST constexpr
#define BOOST_STATIC_CONSTEXPR static BOOST_CONSTEXPR_OR_CONST
#define BOOST_NOEXCEPT noexcept
#define BOOST_NOEXCEPT_OR_NOTHROW noexcept


#define BOOST_SCOPED_ENUM_START(X) enum class X
#define BOOST_SCOPED_ENUM_END
#define BOOST_SCOPED_ENUM(X) X

#define BOOST_ASSERT(C) (C?(void)0:std::terminate())
#define BOOST_ASSERT_MSG(C,M) (C?(void)0:std::terminate())
#define BOOST_FILESYSTEM_I18N  // aid users wishing to compile several versions

#ifdef _WIN32
#define BOOST_WINDOWS_API
#else
#define BOOST_POSIX_API
#endif

#define BOOST_FILESYSTEM_THROW(EX) throw EX

namespace boost { namespace filesystem {}}

#endif // BOOST_FILESYSTEM3_CONFIG_HPP
