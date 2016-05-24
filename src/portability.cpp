//  portability.cpp  -------------------------------------------------------------------//

//  Copyright 2002-2005 Beman Dawes
//  Use, modification, and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
//  at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------// 

// define FILESYSTEM8_SOURCE so that <filesystem8/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define FILESYSTEM8_SOURCE 

#include <filesystem8/config.hpp>
#include <filesystem8/path.hpp>

namespace fs = filesystem8;

#include <cstring> // SGI MIPSpro compilers need this

//--------------------------------------------------------------------------------------//

namespace
{
  const char invalid_chars[] =
    "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    "<>:\"/\\|";
  // note that the terminating '\0' is part of the string - thus the size below
  // is sizeof(invalid_chars) rather than sizeof(invalid_chars)-1.  I 
  const std::string windows_invalid_chars(invalid_chars, sizeof(invalid_chars));

  const std::string valid_posix(
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-");

} // unnamed namespace

  namespace filesystem8
  {

    //  name_check functions  ----------------------------------------------//

#   ifdef _WIN32
    FILESYSTEM8_EXPORT bool native(const std::string & name)
    {
      return windows_name(name);
    }
#   else
    FILESYSTEM8_EXPORT bool native(const std::string & name)
    {
      return  name.size() != 0
        && name[0] != ' '
        && name.find('/') == std::string::npos;
    }
#   endif

    FILESYSTEM8_EXPORT bool portable_posix_name(const std::string & name)
    {
      return name.size() != 0
        && name.find_first_not_of(valid_posix) == std::string::npos;     
    }

    FILESYSTEM8_EXPORT bool windows_name(const std::string & name)
    {
      return name.size() != 0
        && name[0] != ' '
        && name.find_first_of(windows_invalid_chars) == std::string::npos
        && *(name.end()-1) != ' '
        && (*(name.end()-1) != '.'
          || name.length() == 1 || name == "..");
    }

    FILESYSTEM8_EXPORT bool portable_name(const std::string & name)
    {
      return
        name.size() != 0
        && (name == "."
          || name == ".."
          || (windows_name(name)
            && portable_posix_name(name)
            && name[0] != '.' && name[0] != '-'));
    }

    FILESYSTEM8_EXPORT bool portable_directory_name(const std::string & name)
    {
      return
        name == "."
        || name == ".."
        || (portable_name(name)
          && name.find('.') == std::string::npos);
    }

    FILESYSTEM8_EXPORT bool portable_file_name(const std::string & name)
    {
      std::string::size_type pos;
      return
         portable_name(name)
         && name != "."
         && name != ".."
         && ((pos = name.find('.')) == std::string::npos
             || (name.find('.', pos+1) == std::string::npos
               && (pos + 5) > name.length()))
        ;
    }

  } // namespace filesystem8
