//  filesystem path.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2002-2005, 2009
//  Copyright Vladimir Prus 2002

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//  path::stem(), extension(), and replace_extension() are based on
//  basename(), extension(), and change_extension() from the original
//  filesystem/convenience.hpp header by Vladimir Prus.

#ifndef BOOST_FILESYSTEM_PATH_HPP
#define BOOST_FILESYSTEM_PATH_HPP

#include <boost/config.hpp>

# if defined( BOOST_NO_STD_WSTRING )
#   error Configuration not supported: Boost.Filesystem V3 and later requires std::wstring support
# endif

#include <boost/filesystem/config.hpp>
#include <system_error>
#include <memory>
#include <iomanip>
#include <type_traits>
#include <string>
#include <iterator>
#include <cstring>
#include <iosfwd>
#include <stdexcept>
#include <cassert>
#include <locale>
#include <algorithm>

namespace boost
{
namespace filesystem
{

  //------------------------------------------------------------------------------------//
  //                                                                                    //
  //                                    class path                                      //
  //                                                                                    //
  //------------------------------------------------------------------------------------//

  class BOOST_FILESYSTEM_DECL path
  {
  public:

    //  value_type is the character type used by the operating system API to
    //  represent paths.

# ifdef BOOST_WINDOWS_API
    typedef char                           value_type;
    BOOST_STATIC_CONSTEXPR value_type      preferred_separator = '\\';
# else 
    typedef char                           value_type;
    BOOST_STATIC_CONSTEXPR value_type      preferred_separator = '/';
# endif
    typedef std::basic_string<value_type>  string_type;  


    //  ----- character encoding conversions -----

    //  Following the principle of least astonishment, path input arguments
    //  passed to or obtained from the operating system via objects of
    //  class path behave as if they were directly passed to or
    //  obtained from the O/S API, unless conversion is explicitly requested.
    //
    //  POSIX specfies that path strings are passed unchanged to and from the
    //  API. Note that this is different from the POSIX command line utilities,
    //  which convert according to a locale.
    //
    //  Thus for POSIX, char strings do not undergo conversion.  wchar_t strings
    //  are converted to/from char using the path locale or, if a conversion
    //  argument is given, using a conversion object modeled on
    //  std::wstring_convert.
    //
    //  The path locale, which is global to the thread, can be changed by the
    //  imbue() function. It is initialized to an implementation defined locale.
    //  
    //  For Windows, wchar_t strings do not undergo conversion. char strings
    //  are converted using the "ANSI" or "OEM" code pages, as determined by
    //  the AreFileApisANSI() function, or, if a conversion argument is given,
    //  using a conversion object modeled on std::wstring_convert.
    //
    //  See m_pathname comments for further important rationale.

    //  TODO: rules needed for operating systems that use / or .
    //  differently, or format directory paths differently from file paths. 
    //
    //  **********************************************************************************
    //
    //  More work needed: How to handle an operating system that may have
    //  slash characters or dot characters in valid filenames, either because
    //  it doesn't follow the POSIX standard, or because it allows MBCS
    //  filename encodings that may contain slash or dot characters. For
    //  example, ISO/IEC 2022 (JIS) encoding which allows switching to
    //  JIS x0208-1983 encoding. A valid filename in this set of encodings is
    //  0x1B 0x24 0x42 [switch to X0208-1983] 0x24 0x2F [U+304F Kiragana letter KU]
    //                                             ^^^^
    //  Note that 0x2F is the ASCII slash character
    //
    //  **********************************************************************************

    //  Supported source arguments: half-open iterator range, container, c-array,
    //  and single pointer to null terminated string.

    //  All source arguments except pointers to null terminated byte strings support
    //  multi-byte character strings which may have embedded nulls. Embedded null
    //  support is required for some Asian languages on Windows.

    //  "const codecvt_type& cvt=codecvt()" default arguments are not used because this
    //  limits the impact of locale("") initialization failures on POSIX systems to programs
    //  that actually depend on locale(""). It further ensures that exceptions thrown
    //  as a result of such failues occur after main() has started, so can be caught. 

    //  -----  constructors  -----

    path() BOOST_NOEXCEPT {}                                          
    path(const path& p) : m_pathname(p.m_pathname) {}

    path(const value_type* s) : m_pathname(s) {}
    path(const string_type& s) : m_pathname(s) {}

  //  As of October 2015 the interaction between noexcept and =default is so troublesome
  //  for VC++, GCC, and probably other compilers, that =default is not used with noexcept
  //  functions. GCC is not even consistent for the same release on different platforms.

# if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    path(path&& p) BOOST_NOEXCEPT { m_pathname = std::move(p.m_pathname); }
    path& operator=(path&& p) BOOST_NOEXCEPT
      { m_pathname = std::move(p.m_pathname); return *this; }
    path(string_type&& s) : m_pathname(std::move(s)) {}
    path& operator=(string_type&& s) {m_pathname = std::move(s); return *this;}
# endif

    //  -----  assignments  -----

    path& operator=(const path& p)
    {
      m_pathname = p.m_pathname;
      return *this;
    }

    //  value_type overloads

    path& operator=(const value_type* ptr)  // required in case ptr overlaps *this
                                          {m_pathname = ptr; return *this;}
    path& operator=(const string_type& s) {m_pathname = s; return *this;}

    //  -----  concatenation  -----

    //  value_type overloads. Same rationale as for constructors above
    path& operator+=(const path& p)         { m_pathname += p.m_pathname; return *this; }
    path& operator+=(const value_type* ptr) { m_pathname += ptr; return *this; }
    path& operator+=(const string_type& s)  { m_pathname += s; return *this; }
    path& operator+=(value_type c)          { m_pathname += c; return *this; }

    //  -----  appends  -----

    //  if a separator is added, it is the preferred separator for the platform;
    //  slash for POSIX, backslash for Windows

    path& operator/=(const path& p);

    path& operator/=(const value_type* ptr);
    path& operator/=(const string_type& s) { return this->operator/=(path(s)); }

    path& append(const value_type* ptr)  // required in case ptr overlaps *this
    {
      this->operator/=(ptr);
      return *this;
    }

    //  -----  modifiers  -----

    void   clear() BOOST_NOEXCEPT             { m_pathname.clear(); }
    path&  make_preferred()
#   ifdef BOOST_POSIX_API
      { return *this; }  // POSIX no effect
#   else // BOOST_WINDOWS_API
      ;  // change slashes to backslashes
#   endif
    path&  remove_filename();
    path&  remove_trailing_separator();
    path&  replace_extension(const path& new_extension = path());
    void   swap(path& rhs) BOOST_NOEXCEPT     { m_pathname.swap(rhs.m_pathname); }

    //  -----  observers  -----
  
    //  For operating systems that format file paths differently than directory
    //  paths, return values from observers are formatted as file names unless there
    //  is a trailing separator, in which case returns are formatted as directory
    //  paths. POSIX and Windows make no such distinction.

    //  Implementations are permitted to return const values or const references.

    //  The string or path returned by an observer are specified as being formatted
    //  as "native" or "generic".
    //
    //  For POSIX, these are all the same format; slashes and backslashes are as input and
    //  are not modified.
    //
    //  For Windows,   native:    as input; slashes and backslashes are not modified;
    //                            this is the format of the internally stored string.
    //                 generic:   backslashes are converted to slashes

    //  -----  native format observers  -----

    const string_type&  native() const BOOST_NOEXCEPT  { return m_pathname; }
    const value_type*   c_str() const BOOST_NOEXCEPT   { return m_pathname.c_str(); }
    string_type::size_type size() const BOOST_NOEXCEPT { return m_pathname.size(); }

    //  string_type is std::string, so there is no conversion
    const std::string&  string() const { return m_pathname; }

    //  -----  generic format observers  -----

    //  Experimental generic function returning generic formatted path (i.e. separators
    //  are forward slashes). Motivation: simpler than a family of generic_*string
    //  functions.
#   ifdef BOOST_WINDOWS_API
    const std::string   generic_string() const; 

#   else // BOOST_POSIX_API
    //  On POSIX-like systems, the generic format is the same as the native format
    const std::string&  generic_string() const  { return m_pathname; }

#   endif

    //  -----  compare  -----

    int compare(const path& p) const BOOST_NOEXCEPT;  // generic, lexicographical
    int compare(const std::string& s) const { return compare(path(s)); }
    int compare(const value_type* s) const  { return compare(path(s)); }

    //  -----  decomposition  -----

    path  root_path() const; 
    path  root_name() const;         // returns 0 or 1 element path
                                     // even on POSIX, root_name() is non-empty() for network paths
    path  root_directory() const;    // returns 0 or 1 element path
    path  relative_path() const;
    path  parent_path() const;
    path  filename() const;          // returns 0 or 1 element path
    path  stem() const;              // returns 0 or 1 element path
    path  extension() const;         // returns 0 or 1 element path

    //  -----  query  -----

    bool empty() const BOOST_NOEXCEPT{ return m_pathname.empty(); }
    bool has_root_path() const       { return has_root_directory() || has_root_name(); }
    bool has_root_name() const       { return !root_name().empty(); }
    bool has_root_directory() const  { return !root_directory().empty(); }
    bool has_relative_path() const   { return !relative_path().empty(); }
    bool has_parent_path() const     { return !parent_path().empty(); }
    bool has_filename() const        { return !m_pathname.empty(); }
    bool has_stem() const            { return !stem().empty(); }
    bool has_extension() const       { return !extension().empty(); }
    bool is_relative() const         { return !is_absolute(); } 
    bool is_absolute() const
    {
#     ifdef BOOST_WINDOWS_API
      return has_root_name() && has_root_directory();
#     else
      return has_root_directory();
#     endif
    }

    //  -----  lexical operations  -----

    path  lexically_normal() const;
    path  lexically_relative(const path& base) const;
    path  lexically_proximate(const path& base) const
    {
      path tmp(lexically_relative(base));
      return tmp.empty() ? *this : tmp;
    }

    //  -----  iterators  -----

    class iterator;
    typedef iterator const_iterator;
    class reverse_iterator;
    typedef reverse_iterator const_reverse_iterator;

    iterator begin() const;
    iterator end() const;
    reverse_iterator rbegin() const;
    reverse_iterator rend() const;

//--------------------------------------------------------------------------------------//
//                            class path private members                                //
//--------------------------------------------------------------------------------------//

  private:

#   if defined(_MSC_VER)
#     pragma warning(push) // Save warning settings
#     pragma warning(disable : 4251) // disable warning: class 'std::basic_string<_Elem,_Traits,_Ax>'
#   endif                            // needs to have dll-interface...
/*
      m_pathname has the type, encoding, and format required by the native
      operating system. Thus for POSIX and Windows there is no conversion for
      passing m_pathname.c_str() to the O/S API or when obtaining a path from the
      O/S API. POSIX encoding is unspecified other than for dot and slash
      characters; POSIX just treats paths as a sequence of bytes. Windows
      encoding is UCS-2 or UTF-16 depending on the version.
*/
    string_type  m_pathname;  // Windows: as input; backslashes NOT converted to slashes,
                              // slashes NOT converted to backslashes
#   if defined(_MSC_VER)
#     pragma warning(pop) // restore warning settings.
#   endif 

    string_type::size_type m_append_separator_if_needed();
    //  Returns: If separator is to be appended, m_pathname.size() before append. Otherwise 0.
    //  Note: An append is never performed if size()==0, so a returned 0 is unambiguous.

    void m_erase_redundant_separator(string_type::size_type sep_pos);
    string_type::size_type m_parent_path_end() const;

    path& m_normalize();

    // Was qualified; como433beta8 reports:
    //    warning #427-D: qualified name is not allowed in member declaration 
    friend class iterator;
    friend bool operator<(const path& lhs, const path& rhs);

    // see path::iterator::increment/decrement comment below
    static void m_path_iterator_increment(path::iterator & it);
    static void m_path_iterator_decrement(path::iterator & it);

  };  // class path

  namespace detail
  {
    BOOST_FILESYSTEM_DECL
      int lex_compare(path::iterator first1, path::iterator last1,
        path::iterator first2, path::iterator last2);
    BOOST_FILESYSTEM_DECL
      const path&  dot_path();
    BOOST_FILESYSTEM_DECL
      const path&  dot_dot_path();
  }

# ifndef BOOST_FILESYSTEM_NO_DEPRECATED
  typedef path wpath;
# endif

  //------------------------------------------------------------------------------------//
  //                             class path::iterator                                   //
  //------------------------------------------------------------------------------------//
 
#ifdef FILESYSTEM_TODO

  class path::iterator
    : public boost::iterator_facade<
      path::iterator,
      path const,
      boost::bidirectional_traversal_tag >
  {
  private:
    friend class boost::iterator_core_access;
    friend class boost::filesystem::path;
    friend class boost::filesystem::path::reverse_iterator;
    friend void m_path_iterator_increment(path::iterator & it);
    friend void m_path_iterator_decrement(path::iterator & it);

    const path& dereference() const { return m_element; }

    bool equal(const iterator & rhs) const
    {
      return m_path_ptr == rhs.m_path_ptr && m_pos == rhs.m_pos;
    }

    // iterator_facade derived classes don't seem to like implementations in
    // separate translation unit dll's, so forward to class path static members
    void increment() { m_path_iterator_increment(*this); }
    void decrement() { m_path_iterator_decrement(*this); }

    path                    m_element;   // current element
    const path*             m_path_ptr;  // path being iterated over
    string_type::size_type  m_pos;       // position of m_element in
                                         // m_path_ptr->m_pathname.
                                         // if m_element is implicit dot, m_pos is the
                                         // position of the last separator in the path.
                                         // end() iterator is indicated by 
                                         // m_pos == m_path_ptr->m_pathname.size()
  }; // path::iterator

  //------------------------------------------------------------------------------------//
  //                         class path::reverse_iterator                               //
  //------------------------------------------------------------------------------------//
 
  class path::reverse_iterator
    : public boost::iterator_facade<
      path::reverse_iterator,
      path const,
      boost::bidirectional_traversal_tag >
  {
  public:

    explicit reverse_iterator(iterator itr) : m_itr(itr)
    {
      if (itr != itr.m_path_ptr->begin())
        m_element = *--itr;
    }
  private:
    friend class boost::iterator_core_access;
    friend class boost::filesystem::path;

    const path& dereference() const { return m_element; }
    bool equal(const reverse_iterator& rhs) const { return m_itr == rhs.m_itr; }
    void increment()
    { 
      --m_itr;
      if (m_itr != m_itr.m_path_ptr->begin())
      {
        iterator tmp = m_itr;
        m_element = *--tmp;
      }
    }
    void decrement()
    {
      m_element = *m_itr;
      ++m_itr;
    }

    iterator m_itr;
    path     m_element;

  }; // path::reverse_iterator

  inline path::reverse_iterator path::rbegin() const { return reverse_iterator(end()); }
  inline path::reverse_iterator path::rend() const   { return reverse_iterator(begin()); }


  //------------------------------------------------------------------------------------//
  //                                                                                    //
  //                              non-member functions                                  //
  //                                                                                    //
  //------------------------------------------------------------------------------------//

  //  std::lexicographical_compare would infinately recurse because path iterators
  //  yield paths, so provide a path aware version
  inline bool lexicographical_compare(path::iterator first1, path::iterator last1,
    path::iterator first2, path::iterator last2)
    { return detail::lex_compare(first1, last1, first2, last2) < 0; }
#endif
  inline bool operator==(const path& lhs, const path& rhs)              {return lhs.compare(rhs) == 0;}
  inline bool operator==(const path& lhs, const path::string_type& rhs) {return lhs.compare(rhs) == 0;} 
  inline bool operator==(const path::string_type& lhs, const path& rhs) {return rhs.compare(lhs) == 0;}
  inline bool operator==(const path& lhs, const path::value_type* rhs)  {return lhs.compare(rhs) == 0;}
  inline bool operator==(const path::value_type* lhs, const path& rhs)  {return rhs.compare(lhs) == 0;}
  
  inline bool operator!=(const path& lhs, const path& rhs)              {return lhs.compare(rhs) != 0;}
  inline bool operator!=(const path& lhs, const path::string_type& rhs) {return lhs.compare(rhs) != 0;} 
  inline bool operator!=(const path::string_type& lhs, const path& rhs) {return rhs.compare(lhs) != 0;}
  inline bool operator!=(const path& lhs, const path::value_type* rhs)  {return lhs.compare(rhs) != 0;}
  inline bool operator!=(const path::value_type* lhs, const path& rhs)  {return rhs.compare(lhs) != 0;}

  // TODO: why do == and != have additional overloads, but the others don't?

  inline bool operator<(const path& lhs, const path& rhs)  {return lhs.compare(rhs) < 0;}
  inline bool operator<=(const path& lhs, const path& rhs) {return !(rhs < lhs);}
  inline bool operator> (const path& lhs, const path& rhs) {return rhs < lhs;}
  inline bool operator>=(const path& lhs, const path& rhs) {return !(lhs < rhs);}

#ifdef FILESYTEM_TODO
  inline std::size_t hash_value(const path& x)
  {
# ifdef BOOST_WINDOWS_API
    std::size_t seed = 0;
    for(const path::value_type* it = x.c_str(); *it; ++it)
      hash_combine(seed, *it == '/' ? L'\\' : *it);
    return seed;
# else   // BOOST_POSIX_API
    return hash_range(x.native().begin(), x.native().end());
# endif
  }
#endif
  inline void swap(path& lhs, path& rhs)                   { lhs.swap(rhs); }

  inline path operator/(const path& lhs, const path& rhs)  { return path(lhs) /= rhs; }

  //  inserters and extractors
  //    use boost::io::quoted() to handle spaces in paths
  //    use '&' as escape character to ease use for Windows paths

#if __cplusplus > 201400 // std::quoted
  inline std::ostream&
  operator<<(std::ostream& os, const path& p)
  {
    return os
      << std::quoted(p.string(), '&');
  }

  inline std::istream&
  operator>>(std::istream& is, path& p)
  {
    std::string str;
    is >> std::quoted(str, '&');
    p = str;
    return is;
  }
#endif

//--------------------------------------------------------------------------------------//
//                     class path member template specializations                       //
//--------------------------------------------------------------------------------------//

}  // namespace filesystem
}  // namespace boost

//----------------------------------------------------------------------------//

#endif  // BOOST_FILESYSTEM_PATH_HPP
