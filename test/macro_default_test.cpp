//  macro_default_test program  --------------------------------------------------------//

//  Copyright Beman Dawes 2012

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#undef BOOST_ALL_DYN_LINK
#undef BOOST_ALL_STATIC_LINK
#undef FILESYSTEM8_DYN_LINK
#undef FILESYSTEM8_STATIC_LINK
#undef BOOST_SYSTEM_DYN_LINK
#undef BOOST_SYSTEM_STATIC_LINK

#ifndef BOOST_ALL_NO_LIB
# define BOOST_ALL_NO_LIB
#endif

#include <filesystem8/config.hpp>
#include <boost/system/config.hpp>

#ifndef FILESYSTEM8_STATIC_LINK
# error FILESYSTEM8_STATIC_LINK not set by default
#endif


#ifndef BOOST_SYSTEM_STATIC_LINK
# error BOOST_SYSTEM_STATIC_LINK not set by default
#endif

int main()
{
  return 0;
}
