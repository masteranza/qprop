#ifndef PARAMETER_H
#define PARAMETER_H

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::istringstream;
using std::string;
using std::stringstream;

///Simple Object representing a  parameter.
struct parameter
{
  parameter(string n, string t, string v)
      : _name(n), _type(t), _value(v){};
  string _name, _type, _value;
};

///STL type functor for compariosn of parmeter name
struct compareNameWith : public std::unary_function<parameter, bool>
{
  compareNameWith(string n)
      : _name(n){};
  string _name;
  bool operator()(parameter cmp) { return (cmp._name == _name); };
};
static inline string trim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}



///Class for reading parameters from a file.
class parameterList
{
  std::string ovr_string = "Default parameter override with ";
  std::string new_string = "New parameter initialized with ";
  std::vector<parameter> _param_list;
  // std::vector<parameter> _param_defu;
  std::vector<parameter> _param_merg;
  std::string list_name;
  std::string defu_name;

public:
  bool def_loaded = false;
  std::vector<string> paramNotes;
  parameterList();
  parameterList(string file_name, string defaults_name);
  // void setDouble(string file_name, string name, double value);
  double getDouble(string name, bool opt = false);
  string getString(string name, bool opt = false);
  long getLong(string name, bool opt= false);
  bool getBool(string name, bool opt =false);
  std::vector<double> getVectorDouble(string name, bool opt= false);
  std::vector<long> getVectorLong(string name, bool opt =false);
  void copyMergedParamFileTo(string filepath);
};

#endif
