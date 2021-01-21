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

///Class for reading parameters from a file.
class parameterList
{
  std::string ovr_string = "Default parameter override with ";
  std::string new_string = "New parameter initialized with ";
  std::vector<parameter> _param_list;
  std::vector<parameter> _param_defu;
  std::string list_name;
  std::string defu_name;
  bool def_loaded = false;

public:
  std::vector<string> paramNotes;
  parameterList();
  parameterList(string file_name, string defaults_name);
  // void setDouble(string file_name, string name, double value);
  double getDouble(string name);
  string getString(string name);
  long getLong(string name);
  bool getBool(string name);
  std::vector<double> getVectorDouble(string name, bool opt);
  std::vector<long> getVectorLong(string name, bool opt);
};

#endif
