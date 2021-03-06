//  operations_unit_test.cpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2008, 2009, 2015

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//  ------------------------------------------------------------------------------------//

//  This program is misnamed - it is really a smoke test rather than a unit test

//  ------------------------------------------------------------------------------------//


#include <boost/config/warning_disable.hpp>

//  See deprecated_test for tests of deprecated features
#ifndef FILESYSTEM8_NO_DEPRECATED 
#  define FILESYSTEM8_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED 
#  define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include <filesystem8/filesystem.hpp>   // make sure filesystem.hpp works

#include <boost/config.hpp>
# if defined( BOOST_NO_STD_WSTRING )
#   error Configuration not supported: Boost.Filesystem V3 and later requires std::wstring support
# endif

#include <boost/system/error_code.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>

using namespace boost::filesystem;
using namespace boost::system;
using std::cout;
using std::endl;
using std::string;

#define CHECK(x) check(x, __FILE__, __LINE__)

namespace
{
  const path temp_dir(initial_path() / unique_path("op-unit_test-%%%%-%%%%-%%%%"));

  bool cleanup = true;

  void check(bool ok, const char* file, int line)
  {
    if (ok) return;

    ++::boost::detail::test_errors();

    cout << file << '(' << line << "): test failed\n";
  }

  //  file_status_test  ----------------------------------------------------------------//

  void file_status_test()
  {
    cout << "file_status test..." << endl;

    file_status s = status(".");
    int v = s.permissions();
    cout << "  status(\".\") permissions are "
      << std::oct << (v & 0777) << std::dec << endl; 
    CHECK((v & 0400) == 0400);

    s = symlink_status(".");
    v = s.permissions();
    cout << "  symlink_status(\".\") permissions are "
      << std::oct << (v & 0777) << std::dec << endl; 
    CHECK((v & 0400) == 0400);
  }

  //  query_test  ----------------------------------------------------------------------//

  void query_test()
  {
    cout << "query test..." << endl;

    error_code ec;

    CHECK(file_size("no-such-file", ec) == static_cast<std::uintmax_t>(-1));
    CHECK(ec == errc::no_such_file_or_directory);

    CHECK(status("no-such-file") == file_status(file_not_found, perms::none));

    CHECK(exists("/"));
    CHECK(is_directory("/"));
    CHECK(!exists("no-such-file"));

    exists("/", ec);
    if (ec)
    {
      cout << "exists(\"/\", ec) resulted in non-zero ec.value()" << endl;
      cout << "ec value: " << ec.value() << ", message: "<< ec.message() << endl;
    }
    CHECK(!ec);

    CHECK(exists("/"));
    CHECK(is_directory("/"));
    CHECK(!is_regular_file("/"));
    CHECK(!filesystem8::is_empty("/"));
    CHECK(!is_other("/"));
  }

  //  directory_iterator_test  -----------------------------------------------//

  void directory_iterator_test()
  {
    cout << "directory_iterator_test..." << endl;

    directory_iterator end;

    directory_iterator it(".");

    CHECK(!it->path().empty());

    if (is_regular_file(it->status()))
    {
      CHECK(is_regular_file(it->symlink_status()));
      CHECK(!is_directory(it->status()));
      CHECK(!is_symlink(it->status()));
      CHECK(!is_directory(it->symlink_status()));
      CHECK(!is_symlink(it->symlink_status()));
    }
    else
    {
      CHECK(is_directory(it->status()));
      CHECK(is_directory(it->symlink_status()));
      CHECK(!is_regular_file(it->status()));
      CHECK(!is_regular_file(it->symlink_status()));
      CHECK(!is_symlink(it->status()));
      CHECK(!is_symlink(it->symlink_status()));
    }

    for (; it != end; ++it)
    {
      //cout << "  " << it->path() << "\n";
    }

    CHECK(directory_iterator(".") != directory_iterator());
    CHECK(directory_iterator() == end);

#ifndef BOOST_NO_CXX11_RANGE_BASED_FOR
    for (directory_entry& x : directory_iterator("."))
    {
      CHECK(!x.path().empty());
       //cout << "  " << x.path() << "\n";
    }
    const directory_iterator dir_itr(".");
    for (directory_entry& x : dir_itr)
    {
      CHECK(!x.path().empty());
      //cout << "  " << x.path() << "\n";
    }
#endif

    for (directory_iterator itr("."); itr != directory_iterator(); ++itr)
    {
      CHECK(!itr->path().empty());
      //cout << "  " << itr->path() << "\n";
    }

    cout << "directory_iterator_test complete" << endl;
  }

  //  recursive_directory_iterator_test  -----------------------------------------------//

  void recursive_directory_iterator_test()
  {
    cout << "recursive_directory_iterator_test..." << endl;

    recursive_directory_iterator end;

    recursive_directory_iterator it("..");

    CHECK(!it->path().empty());

    if (is_regular_file(it->status()))
    {
      CHECK(is_regular_file(it->symlink_status()));
      CHECK(!is_directory(it->status()));
      CHECK(!is_symlink(it->status()));
      CHECK(!is_directory(it->symlink_status()));
      CHECK(!is_symlink(it->symlink_status()));
    }
    else
    {
      CHECK(is_directory(it->status()));
      CHECK(is_directory(it->symlink_status()));
      CHECK(!is_regular_file(it->status()));
      CHECK(!is_regular_file(it->symlink_status()));
      CHECK(!is_symlink(it->status()));
      CHECK(!is_symlink(it->symlink_status()));
    }

    for (; it != end; ++it)
    {
      //cout << "  " << it->path() << "\n";
    }

    CHECK(recursive_directory_iterator("..") != recursive_directory_iterator());
    CHECK(recursive_directory_iterator() == end);

#ifndef BOOST_NO_CXX11_RANGE_BASED_FOR
    for (directory_entry& x : recursive_directory_iterator(".."))
    {
      CHECK(!x.path().empty());
      //cout << "  " << x.path() << "\n";
    }
    const recursive_directory_iterator dir_itr("..");
    for (directory_entry& x : dir_itr)
    {
      CHECK(!x.path().empty());
      //cout << "  " << x.path() << "\n";
    }
#endif

    for (recursive_directory_iterator itr("..");
      itr != recursive_directory_iterator(); ++itr)
    {
      CHECK(!itr->path().empty());
      //cout << "  " << itr->path() << "\n";
    }

    cout << "recursive_directory_iterator_test complete" << endl;
  }

  //  operations_test  -------------------------------------------------------//

  void operations_test()
  {
    cout << "operations test..." << endl;

    error_code ec;

    CHECK(!create_directory("/", ec));

    CHECK(!filesystem8::remove("no-such-file-or-directory"));
    CHECK(!remove_all("no-such-file-or-directory"));

    space_info info = space("/");

    CHECK(info.available <= info.capacity);

    CHECK(equivalent("/", "/"));
    CHECK(!equivalent("/", "."));

    std::time_t ft = last_write_time(".");
    ft = -1;
    last_write_time(".", ft, ec);
  }

  //  directory_entry_test  ------------------------------------------------------------//

  void directory_entry_test()
  {
    cout << "directory_entry test..." << endl;

    directory_entry de("foo.bar",
      file_status(regular_file, owner_all), file_status(directory_file, group_all));

    CHECK(de.path() == "foo.bar");
    CHECK(de.status() == file_status(regular_file, owner_all));
    CHECK(de.symlink_status() == file_status(directory_file, group_all));
    CHECK(de < directory_entry("goo.bar"));
    CHECK(de == directory_entry("foo.bar"));
    CHECK(de != directory_entry("goo.bar"));
    de.replace_filename("bar.foo");
    CHECK(de.path() == "bar.foo");
  }

  //  directory_entry_overload_test  ---------------------------------------------------//

  void directory_entry_overload_test()
  {
    cout << "directory_entry overload test..." << endl;

    directory_iterator it(".");
    path p(*it);
  }

  //  error_handling_test  -------------------------------------------------------------//

  void error_handling_test()
  {
    cout << "error handling test..." << endl;

    bool threw(false);
    try
    { 
      file_size("no-such-file");
    }
    catch (const filesystem8::filesystem_error & ex)
    {
      threw = true;
      cout << "\nas expected, attempt to get size of non-existent file threw a filesystem_error\n"
        "what() returns " << ex.what() << "\n";
    }
    catch (...)
    {
      cout << "\nunexpected exception type caught" << endl;
    }

    CHECK(threw);

    error_code ec;
    CHECK(!create_directory("/", ec));
  }

  //  string_file_tests  ---------------------------------------------------------------//

  void string_file_tests()
  {
    cout << "string_file_tests..." << endl;
    std::string contents("0123456789");
    path p(temp_dir / "string_file");
    save_string_file(p, contents);
    save_string_file(p, contents);
    BOOST_TEST_EQ(file_size(p), 10u);
    std::string round_trip;
    load_string_file(p, round_trip);
    BOOST_TEST_EQ(contents, round_trip);
  }

}  // unnamed namespace

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                    main                                              //
//                                                                                      //
//--------------------------------------------------------------------------------------//

int cpp_main(int, char*[])
{
// document state of critical macros
#ifdef BOOST_POSIX_API
  cout << "BOOST_POSIX_API is defined\n";
#endif
#ifdef BOOST_WINDOWS_API
  cout << "BOOST_WINDOWS_API is defined\n";
#endif
  cout << "FILESYSTEM8_EXPORT" << BOOST_STRINGIZE(=FILESYSTEM8_EXPORT) << "\n";
  cout << "FILESYSTEM8_EXPORT" << BOOST_STRINGIZE(=FILESYSTEM8_EXPORT) << "\n";
  
  cout << "current_path() is " << current_path().string() << endl;

  create_directory(temp_dir);

  file_status_test();
  query_test();
  directory_iterator_test();
  recursive_directory_iterator_test();
  operations_test();
  directory_entry_test();
  directory_entry_overload_test();
  error_handling_test();
  string_file_tests();

  cout << unique_path() << endl;
  cout << unique_path("foo-%%%%%-%%%%%-bar") << endl;
  cout << unique_path("foo-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%-bar") << endl;
  cout << unique_path("foo-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-%%%%%-bar") << endl;
  
  cout << "testing complete" << endl;

  // post-test cleanup
  if (cleanup)
  {
    cout << "post-test removal of " << temp_dir << endl;
    BOOST_TEST(remove_all(temp_dir) != 0);
    // above was added just to simplify testing, but it ended up detecting
    // a bug (failure to close an internal search handle). 
    cout << "post-test removal complete" << endl;
//    BOOST_TEST(!fs::exists(dir));  // nice test, but doesn't play well with TortoiseGit cache
  }

  return ::boost::report_errors();
}
