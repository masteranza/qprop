#include "parameter.hh"
// #include <smallHelpers.hh>
///reads from file "parameters"
parameterList::parameterList(){

};

void parameterList::updateMergedParamField(string name, string value, string type)
{
  std::vector<parameter>::iterator it2;
  compareNameWith mycmp2(name);
  it2 = find_if(_param_merg.begin(), _param_merg.end(), mycmp2);
  if (it2 == _param_merg.end())
  {
    if (type == "")
    {
      cout << "Type of parameter " << name << " must be set before hand!";
      exit(-1);
    }
    paramNotes.push_back(new_string + name + " = " + value);
    _param_merg.push_back(parameter(name, type, value));
  }
  else if (it2->_type != type && type != "")
  {
    cout << "Type of parameter " << name << " (" << type << ") not equal to the type found in " << defu_name;
    exit(-1);
  }
  else
  {
    if (type == "")
      type = it2->_type;

    if (it2->_value != value)
    {
      paramNotes.push_back(ovr_string + name + " = " + value);
    }

    *it2 = parameter(name, type, value);
    paramNotes.push_back(ovr_string + name + " = " + value);
  }
}

///reads from file file_name
parameterList::parameterList(string file_name, string defaults_name)
{

  // paramNotes.push_back("test");
  std::ifstream inputd(defaults_name.c_str());
  if (!inputd)
  {
    paramNotes.push_back("Defaults parameters file not found in " + defaults_name + ". Proceeding without...");
    def_loaded = false;
  }
  else
    def_loaded = true;

  std::ifstream input(file_name.c_str());
  if (!input)
  {
    std::cerr << "Required parameters file "
              << file_name.c_str()
              << " not found!"
              << "\n";
    exit(-1);
  };
  //Set the .param file names for later use
  list_name = file_name;
  defu_name = defaults_name;

  char line[1024];
  if (def_loaded)
  {
    while (inputd.getline(line, 1024))
    {
      // strip comments from line
      for (size_t i(0); line[i] != '\0'; i++)
      {
        if (line[i] == '#')
        {
          line[i] = '\0';
          break;
        };
      };
      // copy the c_string to a nice string
      string strline(line);
      // check if line has at least three entries
      size_t num_entries(0);
      istringstream test_stream(strline);
      string throwaway;
      while (test_stream >> throwaway)
      {
        num_entries++;
      };
      if (num_entries >= 3)
      {
        std::istringstream in_stream(strline);
        string name, type, value;

        // For parsing vectors (after all # comments are removed)
        in_stream >> name >> type;
        getline(in_stream, value);
        in_stream.ignore();

        trim(name);
        trim(type);
        trim(value);
        // find out if name was used more than once
        compareNameWith mycmp(name);
        std::vector<parameter>::iterator it;
        it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
        if (it != _param_merg.end())
        {
          paramNotes.push_back(name + " appears more than once in parameter file: " + defu_name);
        }
        else
        {
          _param_merg.push_back(parameter(name, type, value));
        }
      }
      else if (num_entries == 0)
      {
        // do nothing for empty line
      }
      else if (num_entries == 2) //Special treatment
      {
        std::istringstream in_stream(strline);
        string name, type, value;

        in_stream >> name >> type;
        trim(name);
        trim(type);

        if (type == "string")
        {
          compareNameWith mycmp(name);
          std::vector<parameter>::iterator it;
          it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
          if (it != _param_merg.end())
          {
            paramNotes.push_back(name + " appears more than once in parameter file: " + defu_name);
          }
          else
          {
            _param_merg.push_back(parameter(name, type, ""));
          }
        }
      }
      else
      {
        paramNotes.push_back(string(strline) + " is not a well formed entry in " + defu_name + " and will be discarded");
      };
    };
    inputd.close();
  }

  while (input.getline(line, 1024))
  {
    // strip comments from line
    for (size_t i(0); line[i] != '\0'; i++)
    {
      if (line[i] == '#')
      {
        line[i] = '\0';
        break;
      };
    };
    // copy the c_string to a nice string
    string strline(line);
    // check if line has at least three entries
    size_t num_entries(0);
    istringstream test_stream(strline);
    string throwaway;
    while (test_stream >> throwaway)
    {
      num_entries++;
    };
    if (num_entries >= 3)
    {
      std::istringstream in_stream(strline);
      string name, type, value;
      // For parsing vectors too
      in_stream >> name >> type;
      getline(in_stream, value);
      in_stream.ignore();

      trim(name);
      trim(type);
      trim(value);

      // find out if name was used more than once
      compareNameWith mycmp(name);
      std::vector<parameter>::iterator it;
      it = find_if(_param_list.begin(), _param_list.end(), mycmp);
      if (it != _param_list.end())
      {
        paramNotes.push_back(name + " appears more than once in parameter file: " + list_name);
      }
      else
      {
        std::vector<parameter>::iterator it2;
        compareNameWith mycmp2(name);
        it2 = find_if(_param_merg.begin(), _param_merg.end(), mycmp2);
        if (it2 == _param_merg.end())
        {
          paramNotes.push_back(new_string + name + " = " + value);
          _param_list.push_back(parameter(name, type, value));
          _param_merg.push_back(parameter(name, type, value));
        }
        else if (it2->_type != type)
        {
          paramNotes.push_back("Type of parameter " + name + " (" + type + ") not equal to the type found in " + defu_name);
          exit(-1);
        }
        else
        {
          if (it2->_value != value)
          {
            paramNotes.push_back(ovr_string + name + " = " + value);
          }
          *it2 = parameter(name, type, value);
          _param_list.push_back(parameter(name, type, value));
          paramNotes.push_back(ovr_string + name + " = " + value);
        }
      }
    }
    else if (num_entries == 0)
    {
      // do nothing for empty line
    }
    else if (num_entries == 2) //Special treatment
    {

      std::istringstream in_stream(strline);
      string name, type, value;

      in_stream >> name >> type;
      trim(name);
      trim(type);

      if (type == "string")
      {
        compareNameWith mycmp(name);
        std::vector<parameter>::iterator it;
        it = find_if(_param_list.begin(), _param_list.end(), mycmp);
        if (it != _param_list.end())
        {
          paramNotes.push_back(name + " appears more than once in parameter file: " + list_name);
        }
        else
        {
          std::vector<parameter>::iterator it2;
          compareNameWith mycmp2(name);
          it2 = find_if(_param_merg.begin(), _param_merg.end(), mycmp2);

          if (it2 == _param_merg.end())
          {
            paramNotes.push_back(new_string + name + " = " + "");
            _param_list.push_back(parameter(name, type, ""));
            _param_merg.push_back(parameter(name, type, ""));
          }
          else if (it2->_type != type)
          {
            paramNotes.push_back("Type of parameter " + name + " (" + type + ") not equal to the type found in " + defu_name);
            exit(-1);
          }
          else
          {
            paramNotes.push_back(ovr_string + name + " = " + "");
            *it2 = parameter(name, type, "");
            _param_list.push_back(parameter(name, type, ""));
          }
        }
      }
    }
    else
    {
      paramNotes.push_back(string(strline) + " is not a well formed entry in " + list_name + " and will be discarded");
    };
  };
  input.close();
};

void parameterList::updateParamFileField(string fieldName, string value)
{
  updateMergedParamField(fieldName, value);

  string tmp("tmp");
  std::fstream fs;
  fs.open((list_name).c_str(), std::fstream::in);
  if (!fs)
  {
    cout << "Error opening " << (list_name).c_str() << endl;
    exit(-1);
  }
  std::fstream fs2;
  fs2.open((list_name + tmp).c_str(), std::fstream::out);
  if (!fs2)
  {
    cout << "Error opening " << (list_name + tmp).c_str() << endl;
    exit(-1);
  }

  string strTemp;
  int found = 0;
  char line[1024];
  while (fs.getline(line, 1024))
  {
    string strline(line);
    if (strline.find(fieldName) != std::string::npos && strline[0] != '#')
    {
      int length = strline.length();
      int element = 0;
      int prev = (int)' ';
      for (int i = 0; i < length; i++)
      {
        int c = strline[i];
        if (!isspace(c) && isspace(prev))
          element++;
        if (element == 3)
        {
          fs2 << strline.substr(0, i) << value << " # (auto)" << endl;
          // cout << strline.substr(0, i) << value << " # (auto)" << endl;

          break;
        }
        prev = c;
      }
    }
    else
    {
      fs2 << line << endl;
      // cout << line << endl;
    }
  }
  if (remove((list_name).c_str()) != 0)
  {
    cout << "Error removing the (old) config file: " << (list_name).c_str();
    exit(-1);
  }
  if (rename((list_name + tmp).c_str(), (list_name).c_str()) != 0)
  {
    cout << "Error renaming the new config file from: " << (list_name + tmp).c_str() << " to " << (list_name).c_str();
    exit(-1);
  }
}

void parameterList::copyMergedParamFileTo(string filepath)
{
  FILE *file_ptr = fopen(filepath.c_str(), "w");
  if (file_ptr == NULL)
  {
    cerr << "copyMergedParamFileTo: " << filepath << " can not be opened!" << endl;
    exit(-1);
  };
  // cout << "Opened file: " << filename << endl;

  for (std::vector<parameter>::iterator it = _param_merg.begin(); it != _param_merg.end(); ++it)
  {
    fprintf(file_ptr, "%-32s%-16s%-s\n", it->_name.c_str(), it->_type.c_str(), it->_value.c_str());
  }
}

///return the parameter name of type double if it exists and return 255 otherwise
double parameterList::getDouble(string name, bool opt /*= false*/)
{

  double blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;

  it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
  if (it == _param_merg.end())
  {
    if (!opt)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
  }
  else if (it->_type != "double")
  {
    std::cerr << "Parameter " << name << " in has an invalid type of " << it->_type << " (double expected)" << std::endl;
    exit(-1);
  }
  else
  {
    //Reading default param
    stringstream bla(it->_value);
    bla >> blub;
  };
  return blub;
};

long parameterList::getLong(string name, bool opt)
{
  long blub = 0;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;

  it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
  if (it == _param_merg.end())
  {
    if (!opt)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
  }
  else if (it->_type != "long")
  {
    std::cerr << "Parameter " << name << " in has an invalid type of " << it->_type << " (long expected)" << std::endl;
    exit(-1);
  }
  else
  {
    //Reading default param
    stringstream bla(it->_value);
    bla >> blub;
  };
  return blub;
};

string parameterList::getString(string name, bool opt)
{
  string blub = "";
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;

  it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
  if (it == _param_merg.end())
  {
    if (!opt)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
  }
  else if (it->_type != "string")
  {
    std::cerr << "Parameter " << name << " has an invalid type of " << it->_type << " (string expected)" << std::endl;
    exit(-1);
  }
  else
  {
    //Reading default param
    stringstream bla(it->_value);
    bla >> blub;
  };
  return blub;
};

bool parameterList::getBool(string name, bool opt)
{
  //Default
  bool blub = false;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;

  it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
  if (it == _param_merg.end())
  {
    if (!opt)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
  }
  else if (it->_type != "bool")
  {
    std::cerr << "Parameter " << name << " has an invalid type of " << it->_type << " (bool expected)" << std::endl;
    exit(-1);
  }
  else
  {
    //Reading default param
    stringstream bla(it->_value);
    bla >> blub;
  };
  return blub;
};

///opt indicates whether the parameter is optional
std::vector<double> parameterList::getVectorDouble(string name, bool opt)
{
  std::vector<double> blub{};
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;

  it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
  if (it == _param_merg.end())
  {
    if (!opt)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
  }
  else if (it->_type != "double")
  {
    std::cerr << "Parameter " << name << " has an invalid type of " << it->_type << " (double expected)" << std::endl;
    exit(-1);
  }
  else
  {
    //Reading default param
    double d;
    stringstream bla(it->_value);
    while (bla >> d)
    {
      cout << "double: " << d;
      blub.push_back(d);
    }
  };
  return blub;
};

///opt indicates whether the parameter is optional
std::vector<long> parameterList::getVectorLong(string name, bool opt)
{
  std::vector<long> blub{};
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;

  it = find_if(_param_merg.begin(), _param_merg.end(), mycmp);
  if (it == _param_merg.end())
  {
    if (!opt)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
  }
  else if (it->_type != "long")
  {
    std::cerr << "Parameter " << name << " has an invalid type of " << it->_type << " (long expected)" << std::endl;
    exit(-1);
  }
  else
  {
    //Reading default param
    long d;
    stringstream bla(it->_value);
    while (bla >> d)
    {
      cout << "long: " << d;
      blub.push_back(d);
    }
  };
  return blub;
};