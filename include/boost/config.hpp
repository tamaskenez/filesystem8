#ifndef CONFIG_309470394
#define CONFIG_309470394

#include <boost/filesystem/export.h>
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
#endif
