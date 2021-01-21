#ifndef SMALL_HELPERS_HH
#define SMALL_HELPERS_HH

#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

using std::cerr;
using std::endl;

template <class a_type>
std::string to_string(a_type zahl)
{
  std::stringstream fluss;
  fluss << zahl;
  return fluss.str();
}

FILE *fopen_with_check(std::string ffname, const char *attr, bool verbose = false)
{
  FILE *file_ptr = fopen(ffname.c_str(), attr);
  if (file_ptr == NULL)
  {
    cerr << "fopen_with_check: " << ffname << " can not be opened!" << endl;
    exit(-1);
  };
  if (verbose)
    cout << "Opened file: " << ffname << endl;
  return file_ptr;
}

std::string create_dir_in_home(std::string ffname)
{
  struct passwd *pw = getpwuid(getuid());
  std::string homedir = pw->pw_dir;
  system(("mkdir -p " + homedir + ffname).c_str());
  return homedir + ffname;
}

std::string create_dir_here(std::string ffname)
{
  system(("mkdir -p ." + ffname).c_str());
  return "./" + ffname;
}

void copy_file(std::string ffname)
{
  system(("mkdir -p ." + ffname).c_str());
}

#endif
