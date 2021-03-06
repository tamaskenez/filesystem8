//  boost/filesystem/operations.hpp  ---------------------------------------------------//

//  Copyright Beman Dawes 2002-2009
//  Copyright Jan Langer 2002
//  Copyright Dietmar Kuehl 2001                                        
//  Copyright Vladimir Prus 2002
   
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------//

#ifndef FILESYSTEM8_OPERATIONS_HPP
#define FILESYSTEM8_OPERATIONS_HPP

#include <filesystem8/config.hpp>
#include <filesystem8/path.hpp>

#include <filesystem8/detail/bitmask.hpp>
#include <system_error>
#include <memory>
#include <type_traits>
#include <cstdint>
#include <string>
#include <utility> // for pair
#include <ctime>
#include <vector>
#include <stack>

#ifdef FILESYSTEM8_WINDOWS_API
#  include <fstream>
#endif


//--------------------------------------------------------------------------------------//

  namespace filesystem8
  {

    //--------------------------------------------------------------------------------------//
    //                                                                                      //
    //                            class filesystem_error                                    //
    //                                                                                      //
    //--------------------------------------------------------------------------------------//

    class FILESYSTEM8_EXPORT filesystem_error : public std::system_error
    {
      // see http://www.boost.org/more/error_handling.html for design rationale

      // all functions are inline to avoid issues with crossing dll boundaries

      // functions previously throw() are now FILESYSTEM8_NOEXCEPT_OR_NOTHROW
      // functions previously without throw() are now FILESYSTEM8_NOEXCEPT

    public:
      // compiler generates copy constructor and copy assignment

      filesystem_error(
        const std::string & what_arg, std::error_code ec) FILESYSTEM8_NOEXCEPT
        : std::system_error(ec, what_arg)
      {
        try
        {
          m_imp_ptr.reset(new m_imp);
        }
        catch (...) { m_imp_ptr.reset(); }
      }

      filesystem_error(
        const std::string & what_arg, const path& path1_arg,
        std::error_code ec) FILESYSTEM8_NOEXCEPT
        : std::system_error(ec, what_arg)
      {
        try
        {
          m_imp_ptr.reset(new m_imp);
          m_imp_ptr->m_path1 = path1_arg;
        }
        catch (...) { m_imp_ptr.reset(); }
      }

      filesystem_error(
        const std::string & what_arg, const path& path1_arg,
        const path& path2_arg, std::error_code ec) FILESYSTEM8_NOEXCEPT
        : std::system_error(ec, what_arg)
      {
        try
        {
          m_imp_ptr.reset(new m_imp);
          m_imp_ptr->m_path1 = path1_arg;
          m_imp_ptr->m_path2 = path2_arg;
        }
        catch (...) { m_imp_ptr.reset(); }
      }

      ~filesystem_error() FILESYSTEM8_NOEXCEPT_OR_NOTHROW{}

        const path& path1() const FILESYSTEM8_NOEXCEPT
      {
        static const path empty_path;
        return m_imp_ptr.get() ? m_imp_ptr->m_path1 : empty_path;
      }
        const path& path2() const  FILESYSTEM8_NOEXCEPT
      {
        static const path empty_path;
        return m_imp_ptr.get() ? m_imp_ptr->m_path2 : empty_path;
      }

        const char* what() const FILESYSTEM8_NOEXCEPT_OR_NOTHROW
      {
        if (!m_imp_ptr.get())
        return std::system_error::what();

        try
        {
          if (m_imp_ptr->m_what.empty())
          {
            m_imp_ptr->m_what = std::system_error::what();
            if (!m_imp_ptr->m_path1.empty())
            {
              m_imp_ptr->m_what += ": \"";
              m_imp_ptr->m_what += m_imp_ptr->m_path1.string();
              m_imp_ptr->m_what += "\"";
            }
            if (!m_imp_ptr->m_path2.empty())
            {
              m_imp_ptr->m_what += ", \"";
              m_imp_ptr->m_what += m_imp_ptr->m_path2.string();
              m_imp_ptr->m_what += "\"";
            }
          }
          return m_imp_ptr->m_what.c_str();
        }
        catch (...)
        {
          return std::system_error::what();
        }
      }

    private:
      struct m_imp
      {
        path         m_path1; // may be empty()
        path         m_path2; // may be empty()
        std::string  m_what;  // not built until needed
      };
      std::shared_ptr<m_imp> m_imp_ptr;
    };

//--------------------------------------------------------------------------------------//
//                                     file_type                                        //
//--------------------------------------------------------------------------------------//

  enum class file_type
  { 
    none = 0,
    not_found = -1,
    regular = 1,
    directory = 2,
    symlink = 3,
    block = 4,
    character = 5,
    fifo = 6,
    socket = 7,
    unknown = 8

    _detail_directory_symlink  // internal use only; never exposed to users
  };

//--------------------------------------------------------------------------------------//
//                                       perms                                          //
//--------------------------------------------------------------------------------------//

  enum class perms
  {
    none = 0,       // file_not_found is perms::none rather than perms::unknown

    // POSIX equivalent macros given in comments.
    // Values are from POSIX and are given in octal per the POSIX standard.

    // permission bits
    
    owner_read = 0400,  // S_IRUSR, Read permission, owner
    owner_write = 0200, // S_IWUSR, Write permission, owner
    owner_exec = 0100,   // S_IXUSR, Execute/search permission, owner
    owner_all = 0700,   // S_IRWXU, Read, write, execute/search by owner

    group_read = 040,   // S_IRGRP, Read permission, group
    group_write = 020,  // S_IWGRP, Write permission, group
    group_exec = 010,    // S_IXGRP, Execute/search permission, group
    group_all = 070,    // S_IRWXG, Read, write, execute/search by group

    others_read = 04,   // S_IROTH, Read permission, others
    others_write = 02,  // S_IWOTH, Write permission, others
    others_exec = 01,    // S_IXOTH, Execute/search permission, others
    others_all = 07,    // S_IRWXO, Read, write, execute/search by others

    all = 0777,     // owner_all|group_all|others_all

    // other POSIX bits

    set_uid = 04000, // S_ISUID, Set-user-ID on execution
    set_gid = 02000, // S_ISGID, Set-group-ID on execution
    sticky_bit     = 01000, // S_ISVTX,
                            // (POSIX XSI) On directories, restricted deletion flag 
                            // (V7) 'sticky bit': save swapped text even after use 
                            // (SunOS) On non-directories: don't cache this file
                            // (SVID-v4.2) On directories: restricted deletion flag
                            // Also see http://en.wikipedia.org/wiki/Sticky_bit

    mask = 07777,     // perms::all|perms::set_uid|perms::set_gid|sticky_bit

    unknown = 0xFFFF, // present when directory_entry cache not loaded

    // options for permissions() function

    add_perms = 0x1000,     // adds the given permission bits to the current bits
    remove_perms = 0x2000,  // removes the given permission bits from the current bits;
                            // choose add_perms or remove_perms, not both; if neither add_perms
                            // nor remove_perms is given, replace the current bits with
                            // the given bits.

    resolve_symlinks = 0x4000  // on POSIX, don't resolve symlinks; implied on Windows
  };

FILESYSTEM8_BITMASK(perms)

//--------------------------------------------------------------------------------------//
//                                    file_status                                       //
//--------------------------------------------------------------------------------------//

  class FILESYSTEM8_EXPORT file_status
  {
  public:
             file_status() FILESYSTEM8_NOEXCEPT
               : m_value(status_error), m_perms(perms::unknown) {}
    explicit file_status(file_type v) FILESYSTEM8_NOEXCEPT
               : m_value(v), m_perms(perms::unknown)  {}
             file_status(file_type v, perms prms) FILESYSTEM8_NOEXCEPT
               : m_value(v), m_perms(prms) {}

  //  As of October 2015 the interaction between noexcept and =default is so troublesome
  //  for VC++, GCC, and probably other compilers, that =default is not used with noexcept
  //  functions. GCC is not even consistent for the same release on different platforms.

    file_status(const file_status& rhs) FILESYSTEM8_NOEXCEPT
      : m_value(rhs.m_value), m_perms(rhs.m_perms) {}
    file_status& operator=(const file_status& rhs) FILESYSTEM8_NOEXCEPT
    {
      m_value = rhs.m_value;
      m_perms = rhs.m_perms;
      return *this;
    }

# if !defined(FILESYSTEM8_NO_CXX11_RVALUE_REFERENCES)
    file_status(file_status&& rhs) FILESYSTEM8_NOEXCEPT
    {
      m_value = std::move(rhs.m_value);
      m_perms = std::move(rhs.m_perms);
    }
    file_status& operator=(file_status&& rhs) FILESYSTEM8_NOEXCEPT
    { 
      m_value = std::move(rhs.m_value);
      m_perms = std::move(rhs.m_perms);
      return *this;
    }
# endif


    // observers
    file_type  type() const FILESYSTEM8_NOEXCEPT            { return m_value; }
    perms      permissions() const FILESYSTEM8_NOEXCEPT     { return m_perms; } 

    // modifiers
    void       type(file_type v) FILESYSTEM8_NOEXCEPT       { m_value = v; }
    void       permissions(perms prms) FILESYSTEM8_NOEXCEPT { m_perms = prms; }

    bool operator==(const file_status& rhs) const FILESYSTEM8_NOEXCEPT
      { return type() == rhs.type() && 
        permissions() == rhs.permissions(); }
    bool operator!=(const file_status& rhs) const FILESYSTEM8_NOEXCEPT
      { return !(*this == rhs); }

  private:
    file_type   m_value;
    enum perms  m_perms;
  };

  inline bool type_present(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return f.type() != status_error; }
  inline bool permissions_present(file_status f) FILESYSTEM8_NOEXCEPT
                                          {return f.permissions() != perms::unknown;}
  inline bool status_known(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return type_present(f) && permissions_present(f); }
  inline bool exists(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return f.type() != status_error
                                                && f.type() != file_not_found; }
  inline bool is_regular_file(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return f.type() == regular_file; }
  inline bool is_directory(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return f.type() == directory_file; }
  inline bool is_symlink(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return f.type() == symlink_file; }
  inline bool is_other(file_status f) FILESYSTEM8_NOEXCEPT
                                          { return exists(f) && !is_regular_file(f)
                                                && !is_directory(f) && !is_symlink(f); }

  struct space_info
  {
    // all values are byte counts
    std::uintmax_t capacity;
    std::uintmax_t free;      // <= capacity
    std::uintmax_t available; // <= free
  };

  enum class copy_options
    {
    none = 0,
    skip_existing = 1,
    overwrite_existing = 2,
    update_existing = 4,
    recursive = 8,
    copy_symlinks = 16,
    skip_symlinks = 32,
    directories_only = 64,
    create_symlinks = 128,
    create_hard_links = 256
};

//--------------------------------------------------------------------------------------//
//                             implementation details                                   //
//--------------------------------------------------------------------------------------//

  namespace detail
  {
    //  We cannot pass a BOOST_SCOPED_ENUM to a compled function because it will result
    //  in an undefined reference if the library is compled with -std=c++0x but the use
    //  is compiled in C++03 mode, or visa versa. See tickets 6124, 6779, 10038.
    enum copy_option {none=0, fail_if_exists = none, overwrite_if_exists};

    FILESYSTEM8_EXPORT
    file_status status(const path&p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    file_status symlink_status(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    bool is_empty(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    path initial_path(std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    path canonical(const path& p, const path& base, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void copy(const path& from, const path& to, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void copy_directory(const path& from, const path& to, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void copy_file(const path& from, const path& to,  // See ticket #2925
                    detail::copy_option option, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void copy_symlink(const path& existing_symlink, const path& new_symlink, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    bool create_directories(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    bool create_directory(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void create_directory_symlink(const path& to, const path& from,
                                  std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void create_hard_link(const path& to, const path& from, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void create_symlink(const path& to, const path& from, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    path current_path(std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void current_path(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    bool equivalent(const path& p1, const path& p2, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    std::uintmax_t file_size(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    std::uintmax_t hard_link_count(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    std::time_t last_write_time(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void last_write_time(const path& p, const std::time_t new_time,
                         std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void permissions(const path& p, perms prms, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    path read_symlink(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    path relative(const path& p, const path& base, std::error_code* ec = 0);
    FILESYSTEM8_EXPORT
    bool remove(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    std::uintmax_t remove_all(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void rename(const path& old_p, const path& new_p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    void resize_file(const path& p, uintmax_t size, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    space_info space(const path& p, std::error_code* ec=0); 
    FILESYSTEM8_EXPORT
    path system_complete(const path& p, std::error_code* ec=0);
    FILESYSTEM8_EXPORT
    path temp_directory_path(std::error_code* ec=0);
#if 0 //no unique_path in std
    FILESYSTEM8_EXPORT
    path unique_path(const path& p, std::error_code* ec=0);
#endif
    FILESYSTEM8_EXPORT
    path weakly_canonical(const path& p, std::error_code* ec = 0);
  }  // namespace detail             

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                             status query functions                                   //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  inline
  file_status status(const path& p)    {return detail::status(p);}
  inline 
  file_status status(const path& p, std::error_code& ec)
                                       {return detail::status(p, &ec);}
  inline 
  file_status symlink_status(const path& p) {return detail::symlink_status(p);}
  inline
  file_status symlink_status(const path& p, std::error_code& ec)
                                       {return detail::symlink_status(p, &ec);}
  inline 
  bool exists(const path& p)           {return exists(detail::status(p));}
  inline 
  bool exists(const path& p, std::error_code& ec)
                                       {return exists(detail::status(p, &ec));}
  inline 
  bool is_directory(const path& p)     {return is_directory(detail::status(p));}
  inline 
  bool is_directory(const path& p, std::error_code& ec)
                                       {return is_directory(detail::status(p, &ec));}
  inline 
  bool is_regular_file(const path& p)  {return is_regular_file(detail::status(p));}
  inline 
  bool is_regular_file(const path& p, std::error_code& ec)
                                       {return is_regular_file(detail::status(p, &ec));}
  inline 
  bool is_other(const path& p)         {return is_other(detail::status(p));}
  inline 
  bool is_other(const path& p, std::error_code& ec)
                                       {return is_other(detail::status(p, &ec));}
  inline
  bool is_symlink(const path& p)       {return is_symlink(detail::symlink_status(p));}
  inline 
  bool is_symlink(const path& p, std::error_code& ec)
                                       {return is_symlink(detail::symlink_status(p, &ec));}
  inline
  bool is_empty(const path& p)         {return detail::is_empty(p);}
  inline
  bool is_empty(const path& p, std::error_code& ec)
                                       {return detail::is_empty(p, &ec);}

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                             operational functions                                    //
//                  in alphabetical order, unless otherwise noted                       //
//                                                                                      //
//--------------------------------------------------------------------------------------//
 
  //  forward declarations
  path current_path();  // fwd declaration
  path initial_path();

  FILESYSTEM8_EXPORT
  path absolute(const path& p, const path& base=current_path());
  //  If base.is_absolute(), throws nothing. Thus no need for ec argument

  inline
  path canonical(const path& p, const path& base=current_path())
                                       {return detail::canonical(p, base);}
  inline
  path canonical(const path& p, std::error_code& ec)
                                       {return detail::canonical(p, current_path(), &ec);}
  inline
  path canonical(const path& p, const path& base, std::error_code& ec)
                                       {return detail::canonical(p, base, &ec);}

  inline
  void copy(const path& from, const path& to) {detail::copy(from, to);}

  inline
  void copy(const path& from, const path& to, std::error_code& ec) FILESYSTEM8_NOEXCEPT 
                                       {detail::copy(from, to, &ec);}
  inline
  void copy_directory(const path& from, const path& to)
                                       {detail::copy_directory(from, to);}
  inline
  void copy_directory(const path& from, const path& to, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::copy_directory(from, to, &ec);}
  inline
  void copy_file(const path& from, const path& to,   // See ticket #2925
                 copy_option option)
  {
    detail::copy_file(from, to, static_cast<detail::copy_option>(option));
  }
  inline
  void copy_file(const path& from, const path& to)
  {
    detail::copy_file(from, to, detail::fail_if_exists);
  }
  inline
  void copy_file(const path& from, const path& to,   // See ticket #2925
                 copy_option option, std::error_code& ec) FILESYSTEM8_NOEXCEPT
  {
    detail::copy_file(from, to, static_cast<detail::copy_option>(option), &ec);
  }
  inline
  void copy_file(const path& from, const path& to, std::error_code& ec) FILESYSTEM8_NOEXCEPT
  {
    detail::copy_file(from, to, detail::fail_if_exists, &ec);
  }
  inline
  void copy_symlink(const path& existing_symlink,
                    const path& new_symlink) {detail::copy_symlink(existing_symlink, new_symlink);}

  inline
  void copy_symlink(const path& existing_symlink, const path& new_symlink,
                    std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::copy_symlink(existing_symlink, new_symlink, &ec);}
  inline
  bool create_directories(const path& p) {return detail::create_directories(p);}

  inline
  bool create_directories(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::create_directories(p, &ec);}
  inline
  bool create_directory(const path& p) {return detail::create_directory(p);}

  inline
  bool create_directory(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::create_directory(p, &ec);}
  inline
  void create_directory_symlink(const path& to, const path& from)
                                       {detail::create_directory_symlink(to, from);}
  inline
  void create_directory_symlink(const path& to, const path& from, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::create_directory_symlink(to, from, &ec);}
  inline
  void create_hard_link(const path& to, const path& new_hard_link) {detail::create_hard_link(to, new_hard_link);}

  inline
  void create_hard_link(const path& to, const path& new_hard_link, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::create_hard_link(to, new_hard_link, &ec);}
  inline
  void create_symlink(const path& to, const path& new_symlink) {detail::create_symlink(to, new_symlink);}

  inline
  void create_symlink(const path& to, const path& new_symlink, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::create_symlink(to, new_symlink, &ec);}
  inline
  path current_path()                  {return detail::current_path();}

  inline
  path current_path(std::error_code& ec) FILESYSTEM8_NOEXCEPT {return detail::current_path(&ec);}

  inline
  void current_path(const path& p)     {detail::current_path(p);}

  inline
  void current_path(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT {detail::current_path(p, &ec);}

  inline
  bool equivalent(const path& p1, const path& p2) {return detail::equivalent(p1, p2);}

  inline
  bool equivalent(const path& p1, const path& p2, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::equivalent(p1, p2, &ec);}
  inline
  std::uintmax_t file_size(const path& p) {return detail::file_size(p);}

  inline
  std::uintmax_t file_size(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::file_size(p, &ec);}
  inline
  std::uintmax_t hard_link_count(const path& p) {return detail::hard_link_count(p);}

  inline
  std::uintmax_t hard_link_count(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::hard_link_count(p, &ec);}
  inline
  path initial_path()                  {return detail::initial_path();}

  inline
  path initial_path(std::error_code& ec) {return detail::initial_path(&ec);}

  template <class Path>
  path initial_path() {return initial_path();}
  template <class Path>
  path initial_path(std::error_code& ec) {return detail::initial_path(&ec);}

  inline
  std::time_t last_write_time(const path& p) {return detail::last_write_time(p);}

  inline
  std::time_t last_write_time(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::last_write_time(p, &ec);}
  inline
  void last_write_time(const path& p, const std::time_t new_time)
                                       {detail::last_write_time(p, new_time);}
  inline
  void last_write_time(const path& p, const std::time_t new_time,
                       std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::last_write_time(p, new_time, &ec);}
  inline
  void permissions(const path& p, perms prms)
                                       {detail::permissions(p, prms);}
  inline
  void permissions(const path& p, perms prms, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::permissions(p, prms, &ec);}

  inline
  path read_symlink(const path& p)     {return detail::read_symlink(p);}

  inline
  path read_symlink(const path& p, std::error_code& ec)
                                       {return detail::read_symlink(p, &ec);}
  inline
    // For standardization, if the committee doesn't like "remove", consider "eliminate"
  bool remove(const path& p)           {return detail::remove(p);}

  inline
  bool remove(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::remove(p, &ec);}

  inline
  std::uintmax_t remove_all(const path& p) {return detail::remove_all(p);}
    
  inline
  std::uintmax_t remove_all(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::remove_all(p, &ec);}
  inline
  void rename(const path& old_p, const path& new_p) {detail::rename(old_p, new_p);}

  inline
  void rename(const path& old_p, const path& new_p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::rename(old_p, new_p, &ec);}
  inline  // name suggested by Scott McMurray
  void resize_file(const path& p, uintmax_t size) {detail::resize_file(p, size);}

  inline
  void resize_file(const path& p, uintmax_t size, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {detail::resize_file(p, size, &ec);}
  inline
  path relative(const path& p, const path& base=current_path())
                                       {return detail::relative(p, base);}
  inline
  path relative(const path& p, std::error_code& ec)
                                       {return detail::relative(p, current_path(), &ec);}
  inline
  path relative(const path& p, const path& base, std::error_code& ec)
                                       {return detail::relative(p, base, &ec);}
  inline
  space_info space(const path& p)      {return detail::space(p);} 

  inline
  space_info space(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
                                       {return detail::space(p, &ec);} 

  inline
  path system_complete(const path& p)  {return detail::system_complete(p);}

  inline
  path system_complete(const path& p, std::error_code& ec)
                                       {return detail::system_complete(p, &ec);}
  inline
  path temp_directory_path()           {return detail::temp_directory_path();}

  inline
  path temp_directory_path(std::error_code& ec) 
                                       {return detail::temp_directory_path(&ec);}
#if 0
  inline
  path unique_path(const path& p="%%%%-%%%%-%%%%-%%%%")
                                       {return detail::unique_path(p);}
  inline
  path unique_path(const path& p, std::error_code& ec)
                                       {return detail::unique_path(p, &ec);}
#endif
  inline
  path weakly_canonical(const path& p)   {return detail::weakly_canonical(p);}
 
  inline
  path weakly_canonical(const path& p, std::error_code& ec)
                                       {return detail::weakly_canonical(p, &ec);}

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 directory_entry                                      //
//                                                                                      //
//--------------------------------------------------------------------------------------//

//  GCC has a problem with a member function named path within a namespace or 
//  sub-namespace that also has a class named path. The workaround is to always
//  fully qualify the name path when it refers to the class name.

class FILESYSTEM8_EXPORT directory_entry
{
public:
  typedef filesystem8::path::value_type value_type;   // enables class path ctor taking directory_entry

  directory_entry() FILESYSTEM8_NOEXCEPT {}
  explicit directory_entry(const filesystem8::path& p)
    : m_path(p), m_status(file_status()), m_symlink_status(file_status())
    {}
  directory_entry(const filesystem8::path& p,
    file_status st, file_status symlink_st = file_status())
    : m_path(p), m_status(st), m_symlink_status(symlink_st) {}

  directory_entry(const directory_entry& rhs)
    : m_path(rhs.m_path), m_status(rhs.m_status), m_symlink_status(rhs.m_symlink_status){}

  directory_entry& operator=(const directory_entry& rhs)
  {
    m_path = rhs.m_path;
    m_status = rhs.m_status;
    m_symlink_status = rhs.m_symlink_status;
    return *this;
  }

  //  As of October 2015 the interaction between noexcept and =default is so troublesome
  //  for VC++, GCC, and probably other compilers, that =default is not used with noexcept
  //  functions. GCC is not even consistent for the same release on different platforms.

#if !defined(FILESYSTEM8_NO_CXX11_RVALUE_REFERENCES)
  directory_entry(directory_entry&& rhs) FILESYSTEM8_NOEXCEPT
  {
    m_path = std::move(rhs.m_path);
    m_status = std::move(rhs.m_status);
    m_symlink_status = std::move(rhs.m_symlink_status);
  }
  directory_entry& operator=(directory_entry&& rhs) FILESYSTEM8_NOEXCEPT
  { 
    m_path = std::move(rhs.m_path);
    m_status = std::move(rhs.m_status);
    m_symlink_status = std::move(rhs.m_symlink_status);
    return *this;
  }
#endif

  void assign(const filesystem8::path& p,
    file_status st = file_status(), file_status symlink_st = file_status())
    { m_path = p; m_status = st; m_symlink_status = symlink_st; }

  void replace_filename(const filesystem8::path& p,
    file_status st = file_status(), file_status symlink_st = file_status())
  {
    m_path.remove_filename();
    m_path /= p;
    m_status = st;
    m_symlink_status = symlink_st;
  }

  const filesystem8::path&  path() const FILESYSTEM8_NOEXCEPT {return m_path;}
  operator const filesystem8::path&() const FILESYSTEM8_NOEXCEPT
                                                              {return m_path;}
  file_status   status() const                                {return m_get_status();}
  file_status   status(std::error_code& ec) const FILESYSTEM8_NOEXCEPT
                                                              {return m_get_status(&ec); }
  file_status   symlink_status() const                        {return m_get_symlink_status();}
  file_status   symlink_status(std::error_code& ec) const FILESYSTEM8_NOEXCEPT
                                                              {return m_get_symlink_status(&ec); }

  bool operator==(const directory_entry& rhs) const FILESYSTEM8_NOEXCEPT {return m_path == rhs.m_path; }
  bool operator!=(const directory_entry& rhs) const FILESYSTEM8_NOEXCEPT {return m_path != rhs.m_path;} 
  bool operator< (const directory_entry& rhs) const FILESYSTEM8_NOEXCEPT {return m_path < rhs.m_path;} 
  bool operator<=(const directory_entry& rhs) const FILESYSTEM8_NOEXCEPT {return m_path <= rhs.m_path;} 
  bool operator> (const directory_entry& rhs) const FILESYSTEM8_NOEXCEPT {return m_path > rhs.m_path;} 
  bool operator>=(const directory_entry& rhs) const FILESYSTEM8_NOEXCEPT {return m_path >= rhs.m_path;} 

private:
  filesystem8::path   m_path;
  mutable file_status       m_status;           // stat()-like
  mutable file_status       m_symlink_status;   // lstat()-like

  file_status m_get_status(std::error_code* ec=0) const;
  file_status m_get_symlink_status(std::error_code* ec=0) const;
}; // directory_entry

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                            directory_iterator helpers                                //
//                                                                                      //
//--------------------------------------------------------------------------------------//

class directory_iterator;

namespace detail
{
  FILESYSTEM8_EXPORT
    std::error_code dir_itr_close(// never throws()
    void *& handle
#   if     defined(FILESYSTEM8_POSIX_API)
    , void *& buffer
#   endif
  ); 

  struct dir_itr_imp
  {
    directory_entry  dir_entry;
    void*            handle;

#   ifdef FILESYSTEM8_POSIX_API
    void*            buffer;  // see dir_itr_increment implementation
#   endif

    dir_itr_imp() : handle(0)
#   ifdef FILESYSTEM8_POSIX_API
      , buffer(0)
#   endif
    {}

    ~dir_itr_imp() // never throws
    {
      dir_itr_close(handle
#       if defined(FILESYSTEM8_POSIX_API)
         , buffer
#       endif
    );
    }
  };

  // see path::iterator: comment below
  FILESYSTEM8_EXPORT void directory_iterator_construct(directory_iterator& it,
    const path& p, std::error_code* ec);
  FILESYSTEM8_EXPORT void directory_iterator_increment(directory_iterator& it,
    std::error_code* ec);

}  // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                directory_iterator                                    //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  class directory_iterator
    : public iterator_facade< directory_iterator,
                                     directory_entry,
                                     std::forward_iterator_tag >
  {
  public:

    directory_iterator() FILESYSTEM8_NOEXCEPT {}  // creates the "end" iterator

    // iterator_facade derived classes don't seem to like implementations in
    // separate translation unit dll's, so forward to detail functions
    explicit directory_iterator(const path& p)
        : m_imp(new detail::dir_itr_imp)
          { detail::directory_iterator_construct(*this, p, 0); }

    directory_iterator(const path& p, std::error_code& ec) FILESYSTEM8_NOEXCEPT
        : m_imp(new detail::dir_itr_imp)
          { detail::directory_iterator_construct(*this, p, &ec); }

   ~directory_iterator() {}

    directory_iterator& increment(std::error_code& ec) FILESYSTEM8_NOEXCEPT
    { 
      detail::directory_iterator_increment(*this, &ec);
      return *this;
    }

  private:
    friend class iterator_facade< directory_iterator,
                                     directory_entry,
                                     std::forward_iterator_tag >;
    friend struct detail::dir_itr_imp;
    friend FILESYSTEM8_EXPORT void detail::directory_iterator_construct(directory_iterator& it,
      const path& p, std::error_code* ec);
    friend FILESYSTEM8_EXPORT void detail::directory_iterator_increment(directory_iterator& it,
      std::error_code* ec);

    // shared_ptr provides shallow-copy semantics required for InputIterators.
    // m_imp.get()==0 indicates the end iterator.
    std::shared_ptr< detail::dir_itr_imp >  m_imp;

    reference dereference() const
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(), "attempt to dereference end iterator");
      return m_imp->dir_entry;
    }

    void increment() { detail::directory_iterator_increment(*this, 0); }

    bool equal(const directory_iterator& rhs) const
      { return m_imp == rhs.m_imp; }

  };  // directory_iterator

  //  enable directory_iterator C++11 range-base for statement use  --------------------//

  //  begin() and end() are only used by a range-based for statement in the context of
  //  auto - thus the top-level const is stripped - so returning const is harmless and
  //  emphasizes begin() is just a pass through.
  inline
  const directory_iterator& begin(const directory_iterator& iter) FILESYSTEM8_NOEXCEPT
    {return iter;}
  inline
  directory_iterator end(const directory_iterator&) FILESYSTEM8_NOEXCEPT
    {return directory_iterator();}

  //  enable directory_iterator BOOST_FOREACH  -----------------------------------------//

  inline
  directory_iterator& range_begin(directory_iterator& iter) FILESYSTEM8_NOEXCEPT
    {return iter;}
  inline
  directory_iterator range_begin(const directory_iterator& iter) FILESYSTEM8_NOEXCEPT
    {return iter;}
  inline
  directory_iterator range_end(const directory_iterator&) FILESYSTEM8_NOEXCEPT
    {return directory_iterator();}

  }  // namespace filesystem8


namespace filesystem8
{

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                      recursive_directory_iterator helpers                            //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  enum class symlink_option
  {
    none,
    no_recurse = none,         // don't follow directory symlinks (default behavior)
    recurse,                   // follow directory symlinks
    _detail_no_push = recurse << 1  // internal use only
  };
  
  FILESYSTEM8_BITMASK(symlink_option)

  namespace detail
  {
    struct recur_dir_itr_imp
    {
      typedef directory_iterator element_type;
      std::stack< element_type, std::vector< element_type > > m_stack;
      int  m_level;
      symlink_option m_options;

      recur_dir_itr_imp() : m_level(0), m_options(symlink_option::none) {}

      void increment(std::error_code* ec);  // ec == 0 means throw on error

      bool push_directory(std::error_code& ec) FILESYSTEM8_NOEXCEPT;

      void pop();

    };

    //  Implementation is inline to avoid dynamic linking difficulties with m_stack:
    //  Microsoft warning C4251, m_stack needs to have dll-interface to be used by
    //  clients of struct 'filesystem8::detail::recur_dir_itr_imp'

    inline
    bool recur_dir_itr_imp::push_directory(std::error_code& ec) FILESYSTEM8_NOEXCEPT
    // Returns: true if push occurs, otherwise false. Always returns false on error.
    {
      ec.clear();

      //  Discover if the iterator is for a directory that needs to be recursed into,
      //  taking symlinks and options into account.

      if ((m_options & symlink_option::_detail_no_push) == symlink_option::_detail_no_push)
        m_options &= ~symlink_option::_detail_no_push;

      else
      {
        // Logic for following predicate was contributed by Daniel Aarno to handle cyclic
        // symlinks correctly and efficiently, fixing ticket #5652.
        //   if (((m_options & symlink_option::recurse) == symlink_option::recurse
        //         || !is_symlink(m_stack.top()->symlink_status()))
        //       && is_directory(m_stack.top()->status())) ...
        // The predicate code has since been rewritten to pass error_code arguments,
        // per ticket #5653.

        file_status symlink_stat;

        if ((m_options & symlink_option::recurse) != symlink_option::recurse)
        {
          symlink_stat = m_stack.top()->symlink_status(ec);
          if (ec)
            return false;
        }

        if ((m_options & symlink_option::recurse) == symlink_option::recurse
          || !is_symlink(symlink_stat))
        {
          file_status stat = m_stack.top()->status(ec);
          if (ec || !is_directory(stat))
            return false;

          directory_iterator next(m_stack.top()->path(), ec);
          if (!ec && next != directory_iterator())
          {
            m_stack.push(next);
            ++m_level;
            return true;
          }
        }
      }
      return false;
    }

    inline
    void recur_dir_itr_imp::increment(std::error_code* ec)
    // ec == 0 means throw on error
    // 
    // Invariant: On return, the top of the iterator stack is the next valid (possibly
    // end) iterator, regardless of whether or not an error is reported, and regardless of
    // whether any error is reported by exception or error code. In other words, progress
    // is always made so a loop on the iterator will always eventually terminate
    // regardless of errors.
    {
      std::error_code ec_push_directory;

      //  if various conditions are met, push a directory_iterator into the iterator stack
      if (push_directory(ec_push_directory))
      {
        if (ec)
          ec->clear();
        return;
      }

      //  Do the actual increment operation on the top iterator in the iterator
      //  stack, popping the stack if necessary, until either the stack is empty or a
      //  non-end iterator is reached.
      while (!m_stack.empty() && ++m_stack.top() == directory_iterator())
      {
        m_stack.pop();
        --m_level;
      }

      // report errors if any
      if (ec_push_directory)
      {
        if (ec)
          *ec = ec_push_directory;
        else
        {
          FILESYSTEM8_THROW(filesystem_error(
            "filesystem::recursive_directory_iterator directory error",
            ec_push_directory));
        }
      }
      else if (ec)
        ec->clear();
    }

    inline
    void recur_dir_itr_imp::pop()
    {
      FILESYSTEM8_ASSERT_MSG(m_level > 0,
        "pop() on recursive_directory_iterator with level < 1");

      do
      {
        m_stack.pop();
        --m_level;
      }
      while (!m_stack.empty() && ++m_stack.top() == directory_iterator());
    }
  } // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                           recursive_directory_iterator                               //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  class recursive_directory_iterator
    : public iterator_facade<
        recursive_directory_iterator,
        directory_entry,
        std::forward_iterator_tag >
  {
  public:

    recursive_directory_iterator() FILESYSTEM8_NOEXCEPT {}  // creates the "end" iterator

    explicit recursive_directory_iterator(const path& dir_path)  // throws if !exists()
      : m_imp(new detail::recur_dir_itr_imp)
    {
      m_imp->m_options = symlink_option::none;
      m_imp->m_stack.push(directory_iterator(dir_path));
      if (m_imp->m_stack.top() == directory_iterator())
        { m_imp.reset(); }
    }

    recursive_directory_iterator(const path& dir_path,
      symlink_option opt)  // throws if !exists()
      : m_imp(new detail::recur_dir_itr_imp)
    {
      m_imp->m_options = opt;
      m_imp->m_stack.push(directory_iterator(dir_path));
      if (m_imp->m_stack.top() == directory_iterator())
        { m_imp.reset (); }
    }

    recursive_directory_iterator(const path& dir_path,
      symlink_option opt,
      std::error_code & ec) FILESYSTEM8_NOEXCEPT
    : m_imp(new detail::recur_dir_itr_imp)
    {
      m_imp->m_options = opt;
      m_imp->m_stack.push(directory_iterator(dir_path, ec));
      if (m_imp->m_stack.top() == directory_iterator())
        { m_imp.reset (); }
    }

    recursive_directory_iterator(const path& dir_path,
      std::error_code & ec) FILESYSTEM8_NOEXCEPT
    : m_imp(new detail::recur_dir_itr_imp)
    {
      m_imp->m_options = symlink_option::none;
      m_imp->m_stack.push(directory_iterator(dir_path, ec));
      if (m_imp->m_stack.top() == directory_iterator())
        { m_imp.reset (); }
    }

    recursive_directory_iterator& increment(std::error_code& ec) FILESYSTEM8_NOEXCEPT
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "increment() on end recursive_directory_iterator");
      m_imp->increment(&ec);
      if (m_imp->m_stack.empty())
        m_imp.reset(); // done, so make end iterator
      return *this;
    }

    int depth() const FILESYSTEM8_NOEXCEPT
    { 
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "depth() on end recursive_directory_iterator");
      return m_imp->m_level;
    }
  
    int level() const FILESYSTEM8_NOEXCEPT { return depth(); }

    bool recursion_pending() const FILESYSTEM8_NOEXCEPT
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "is_no_push_requested() on end recursive_directory_iterator");
      return (m_imp->m_options & symlink_option::_detail_no_push)
        == symlink_option::_detail_no_push;
    }
  
    bool no_push_pending() const FILESYSTEM8_NOEXCEPT { return recursion_pending(); }

    void pop()
    { 
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "pop() on end recursive_directory_iterator");
      m_imp->pop();
      if (m_imp->m_stack.empty()) m_imp.reset(); // done, so make end iterator
    }

    void disable_recursion_pending(bool value=true) FILESYSTEM8_NOEXCEPT
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "no_push() on end recursive_directory_iterator");
      if (value)
        m_imp->m_options |= symlink_option::_detail_no_push;
      else
        m_imp->m_options &= ~symlink_option::_detail_no_push;
    }
  
    void no_push(bool value=true) FILESYSTEM8_NOEXCEPT { disable_recursion_pending(value); }

    file_status status() const
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "status() on end recursive_directory_iterator");
      return m_imp->m_stack.top()->status();
    }

    file_status symlink_status() const
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "symlink_status() on end recursive_directory_iterator");
      return m_imp->m_stack.top()->symlink_status();
    }

  private:

    friend class iterator_facade<
        recursive_directory_iterator,
        directory_entry,
        std::forward_iterator_tag >;

    // shared_ptr provides shallow-copy semantics required for InputIterators.
    // m_imp.get()==0 indicates the end iterator.
    std::shared_ptr< detail::recur_dir_itr_imp >  m_imp;

    reference
    dereference() const 
    {
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "dereference of end recursive_directory_iterator");
      return *m_imp->m_stack.top();
    }

    void increment()
    { 
      FILESYSTEM8_ASSERT_MSG(m_imp.get(),
        "increment of end recursive_directory_iterator");
      m_imp->increment(0);
      if (m_imp->m_stack.empty())
        m_imp.reset(); // done, so make end iterator
    }

    bool equal(const recursive_directory_iterator& rhs) const
      { return m_imp == rhs.m_imp; }

  };  // recursive directory iterator

  //  enable recursive directory iterator C++11 range-base for statement use  ----------//

  //  begin() and end() are only used by a range-based for statement in the context of
  //  auto - thus the top-level const is stripped - so returning const is harmless and
  //  emphasizes begin() is just a pass through.
  inline
  const recursive_directory_iterator&
    begin(const recursive_directory_iterator& iter) FILESYSTEM8_NOEXCEPT
                                                  {return iter;}
  inline
  recursive_directory_iterator end(const recursive_directory_iterator&) FILESYSTEM8_NOEXCEPT
                                                  {return recursive_directory_iterator();}

  //  enable recursive directory iterator BOOST_FOREACH  -------------------------------//

  inline
  recursive_directory_iterator& 
    range_begin(recursive_directory_iterator& iter) FILESYSTEM8_NOEXCEPT
                                                   {return iter;}
  inline
  recursive_directory_iterator
    range_begin(const recursive_directory_iterator& iter) FILESYSTEM8_NOEXCEPT
                                                   {return iter;}
  inline
  recursive_directory_iterator range_end(const recursive_directory_iterator&) FILESYSTEM8_NOEXCEPT
                                                  {return recursive_directory_iterator();}
  }  // namespace filesystem8

namespace filesystem8
{

//  test helper  -----------------------------------------------------------------------//

//  Not part of the documented interface since false positives are possible;
//  there is no law that says that an OS that has large stat.st_size
//  actually supports large file sizes.

  namespace detail
  {
    FILESYSTEM8_EXPORT bool possible_large_file_size_support();
  }
  } // namespace filesystem8

#endif // FILESYSTEM8_OPERATIONS_HPP
