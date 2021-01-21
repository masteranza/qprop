#include "parameter.hh"
// #include <smallHelpers.hh>
///reads from file "parameters"
parameterList::parameterList(){

};

///reads from file file_name
parameterList::parameterList(string file_name, string defaults_name)
{
  // paramNotes.push_back("test");
  std::ifstream inputd(defaults_name.c_str());
  if (!inputd)
  {
    paramNotes.push_back("Defaults parameters file not found in " + defaults_name + ". Proceeding without...");
    // std::cerr << "Defaults parameters file not found in "
    // << defaults_name.c_str()
    // << ". Proceeding without..." << endl;
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

  list_name = file_name;
  defu_name = defaults_name;

  char line[1024];
  if (def_loaded)
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
      // check if line has exactly three entries
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

        // Old approad
        // in_stream >> name >> type >> value;
        // For parsing vectors
        in_stream >> name >> type;
        getline(in_stream, value);
        in_stream.ignore();
        // remove_carriage_return(value);
        // find out if name was used more than once
        compareNameWith mycmp(name);
        std::vector<parameter>::const_iterator it;
        it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
        if (it != _param_defu.end())
        {
          cerr << name << " appears more than once in parameter file: " << defu_name << endl;
        }
        else
          _param_defu.push_back(parameter(name, type, value));
#ifdef VERBOSE
        cout << name << " " << type << " " << value << endl;
#endif
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
        if (type == "string")
        {
          compareNameWith mycmp(name);
          std::vector<parameter>::const_iterator it;
          it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
          if (it != _param_defu.end())
          {
            cerr << name << " appears more than once in parameter file: " << defu_name << endl;
          }
          else
            _param_defu.push_back(parameter(name, type, ""));
        }
      }
      else
      {
        cerr << strline << " is not a well formed entry in " << defu_name << " and will be discarded, num_entries: " << num_entries << endl;
      };
    };

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
    // check if line has exactly three entries
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

      // Old approad
      // in_stream >> name >> type >> value;
      // For parsing vectors
      in_stream >> name >> type;
      getline(in_stream, value);
      in_stream.ignore();
      // remove_carriage_return(value);
      // find out if name was used more than once
      compareNameWith mycmp(name);
      std::vector<parameter>::const_iterator it;
      it = find_if(_param_list.begin(), _param_list.end(), mycmp);
      if (it != _param_list.end())
      {
        cerr << name << " appears more than once in parameter file: " << list_name << endl;
      }
      else
        _param_list.push_back(parameter(name, type, value));
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
      if (type == "string")
      {
        compareNameWith mycmp(name);
        std::vector<parameter>::const_iterator it;
        it = find_if(_param_list.begin(), _param_list.end(), mycmp);
        if (it != _param_list.end())
        {
          cerr << name << " appears more than once in parameter file: " << list_name << endl;
        }
        else
          _param_list.push_back(parameter(name, type, ""));
      }
    }
    else
    {
      cerr << strline << " is not a well formed entry in " << list_name << " and will be discarded, num_entries: " << num_entries << endl;
    };
  };
};

///return the parameter name of type double if it exists and return 255 otherwise
double parameterList::getDouble(string name)
{
  bool nd = !def_loaded;
  double blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;
  if (def_loaded)
  {
    it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
    if (it == _param_defu.end())
    {
      nd = true;
      // std::cout << "Asking for non-default parameter"
    }
    else if (it->_type != "double")
    {
      std::cerr << "Parameter " << name << " in " << defu_name << " has an invalid type of " << it->_type << " (double expected)" << std::endl;
      exit(-1);
    }
    else
    {
      //Reading default param
      stringstream bla(it->_value);
      bla >> blub;
    };
  };

  it = find_if(_param_list.begin(), _param_list.end(), mycmp);
  if (it == _param_list.end())
  {
    if (nd)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
    else
      return blub;
  }
  else if (it->_type != "double")
  {
    std::cerr << "Parameter " << name << " in " << list_name << " has an invalid type of " << it->_type << " (double expected)" << std::endl;
    exit(-1);
  }
  else
  {
    stringstream bla(it->_value);
    double blub2;
    bla >> blub2;
    if (nd)
      paramNotes.push_back(new_string + name + " = " + it->_value); // std::__cxx11::to_string(blub2)
    else if (blub2 != blub)
      paramNotes.push_back(ovr_string + name + " = " + it->_value);
    return blub2;
  };
};

long parameterList::getLong(string name)
{
  bool nd = !def_loaded;
  long blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;
  if (def_loaded)
  {
    it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
    if (it == _param_defu.end())
    {
      nd = true;
    }
    else if (it->_type != "long")
    {
      std::cerr << "Parameter " << name << " in " << defu_name << " has an invalid type of " << it->_type << " (long expected)" << std::endl;
      exit(-1);
    }
    else
    {
      //Reading default param
      stringstream bla(it->_value);
      bla >> blub;
    };
  };

  it = find_if(_param_list.begin(), _param_list.end(), mycmp);
  if (it == _param_list.end())
  {
    if (nd)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
    else
      return blub;
  }
  else if (it->_type != "long")
  {
    std::cerr << "Parameter " << name << " in " << list_name << " has an invalid type of " << it->_type << " (long expected)" << std::endl;
    exit(-1);
  }
  else
  {
    stringstream bla(it->_value);
    long blub2;
    bla >> blub2;
    if (nd)
      paramNotes.push_back(new_string + name + " = " + it->_value);
    else if (blub2 != blub)
      paramNotes.push_back(ovr_string + name + " = " + it->_value);
    return blub2;
  };
};

string parameterList::getString(string name)
{
  bool nd = !def_loaded;
  string blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;
  if (def_loaded)
  {
    it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
    if (it == _param_defu.end())
    {
      nd = true;
    }
    else if (it->_type != "string")
    {
      std::cerr << "Parameter " << name << " in " << defu_name << " has an invalid type of " << it->_type << " (string expected)" << std::endl;
      exit(-1);
    }
    else
    {
      //Reading default param
      stringstream bla(it->_value);
      bla >> blub;
    };
  };

  it = find_if(_param_list.begin(), _param_list.end(), mycmp);
  if (it == _param_list.end())
  {
    if (nd)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
    else
      return blub;
  }
  else if (it->_type != "string")
  {
    std::cerr << "Parameter " << name << " in " << list_name << " has an invalid type of " << it->_type << " (string expected)" << std::endl;
    exit(-1);
  }
  else
  {
    stringstream bla(it->_value);
    string blub2;
    bla >> blub2;
    if (nd)
      paramNotes.push_back(new_string + name + " = " + it->_value);
    else if (blub2 != blub)
      paramNotes.push_back(ovr_string + name + " = " + it->_value);
    return blub2;
  };
};

bool parameterList::getBool(string name)
{
  bool nd = !def_loaded;
  bool blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;
  if (def_loaded)
  {
    it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
    if (it == _param_defu.end())
    {
      nd = true;
    }
    else if (it->_type != "bool")
    {
      std::cerr << "Parameter " << name << " in " << defu_name << " has an invalid type of " << it->_type << " (bool expected)" << std::endl;
      exit(-1);
    }
    else
    {
      //Reading default param
      stringstream bla(it->_value);
      bla >> blub;
    };
  };

  it = find_if(_param_list.begin(), _param_list.end(), mycmp);
  if (it == _param_list.end())
  {
    if (nd)
    {
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
    else
      return blub;
  }
  else if (it->_type != "bool")
  {
    std::cerr << "Parameter " << name << " in " << list_name << " has an invalid type of " << it->_type << " (bool expected)" << std::endl;
    exit(-1);
  }
  else
  {
    stringstream bla(it->_value);
    bool blub2;
    bla >> blub2;
    if (nd)
      paramNotes.push_back(new_string + name + " = " + it->_value);
    else if (blub2 != blub)
      paramNotes.push_back(ovr_string + name + " = " + it->_value);
    return blub2;
  };
};

///opt indicates whether the parameter is optional
std::vector<double> parameterList::getVectorDouble(string name, bool opt)
{
  bool nd = !def_loaded;
  // double blub;
  std::vector<double> blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;
  if (def_loaded)
  {
    it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
    if (it == _param_defu.end())
    {
      nd = true;
    }
    else if (it->_type != "double")
    {
      std::cerr << "Parameter " << name << " in " << defu_name << " has an invalid type of " << it->_type << " (double expected)" << std::endl;
      exit(-1);
    }
    else
    {
      //Reading default param
      double d;
      stringstream bla(it->_value);
      while (bla >> d)
      {
        blub.push_back(d);
      }
    };
  };

  it = find_if(_param_list.begin(), _param_list.end(), mycmp);
  if (it == _param_list.end())
  {
    if (nd)
    {
      if (opt)
        return {};
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
    else
      return blub;
  }
  else if (it->_type != "double")
  {
    std::cerr << "Parameter " << name << " in " << list_name << " has an invalid type of " << it->_type << " (double expected)" << std::endl;
    exit(-1);
  }
  else
  {
    double d;
    stringstream bla(it->_value);
    std::vector<double> blub2;
    bool difQ = false;
    int inx = 0;

    while (bla >> d)
    {
      if (inx < blub.size())
      {
        if (d != blub[inx])
          difQ = true;
      }
      else
        difQ = true;
      blub2.push_back(d);
      inx++;
    }
    if (nd)
      paramNotes.push_back(new_string + name + " = " + it->_value);
    else if (difQ)
      paramNotes.push_back(ovr_string + name + " = " + it->_value);
    return blub2;
  };
};

///opt indicates whether the parameter is optional
std::vector<long> parameterList::getVectorLong(string name, bool opt)
{
  bool nd = !def_loaded;
  std::vector<long> blub;
  compareNameWith mycmp(name);
  std::vector<parameter>::const_iterator it;
  if (def_loaded)
  {
    it = find_if(_param_defu.begin(), _param_defu.end(), mycmp);
    if (it == _param_defu.end())
    {
      nd = true;
    }
    else if (it->_type != "long")
    {
      std::cerr << "Parameter " << name << " in " << defu_name << " has an invalid type of " << it->_type << " (long expected)" << std::endl;
      exit(-1);
    }
    else
    {
      //Reading default param
      long d;
      stringstream bla(it->_value);
      while (bla >> d)
      {
        blub.push_back(d);
      }
    };
  };

  it = find_if(_param_list.begin(), _param_list.end(), mycmp);
  if (it == _param_list.end())
  {
    if (nd)
    {
      if (opt)
        return {};
      std::cerr << " Parameter " << name << " not found in the " << list_name << " nor " << defu_name << " files" << std::endl;
      exit(-1);
    }
    else
      return blub;
  }
  else if (it->_type != "long")
  {
    std::cerr << "Parameter " << name << " in " << list_name << " has an invalid type of " << it->_type << " (long expected)" << std::endl;
    exit(-1);
  }
  else
  {
    long d;
    stringstream bla(it->_value);
    std::vector<long> blub2;
    bool difQ = false;
    int inx = 0;

    while (bla >> d)
    {
      if (inx < blub.size())
      {
        if (d != blub[inx])
          difQ = true;
      }
      else
        difQ = true;
      blub2.push_back(d);
      inx++;
    }
    if (nd)
      paramNotes.push_back(new_string + name + " = " + it->_value);
    else if (difQ)
      paramNotes.push_back(ovr_string + name + " = " + it->_value);
    return blub2;
  };
};