// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* Shows how to use both command line and config file. */

#include <fstream>
#include <iostream>
#include <iterator>
#include <program_options.h>

// A helper function to simplify the main part.
template<class T>
auto operator<<(std::ostream &os, const std::vector<T> &v) -> std::ostream &
{
   std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
   return os;
}

auto main(int ac, char *av[]) -> int
{
   try
   {
      int         opt;
      std::string config_file;

      // Declare a group of options that will be
      // allowed only on command line
      options_description generic("Generic options");
      generic.add_options()("version,v", "print version string")("help", "produce help message")(
         "config,c", value<std::string>(&config_file)->default_value("multiple_sources.cfg"), "name of a file of a configuration.");

      // Declare a group of options that will be
      // allowed both on command line and in
      // config file
      options_description config("Configuration");
      config.add_options()("optimization", value<int>(&opt)->default_value(10),
                           "optimization level")("include-path,I", value<std::vector<std::string>>()->composing(), "include path");

      // Hidden options, will be allowed both on command line and
      // in config file, but will not be shown to the user.
      options_description hidden("Hidden options");
      hidden.add_options()("input-file", value<std::vector<std::string>>(), "input file");

      options_description cmdline_options;
      cmdline_options.add(generic).add(config).add(hidden);

      options_description config_file_options;
      config_file_options.add(config).add(hidden);

      options_description visible("Allowed options");
      visible.add(generic).add(config);

      positional_options_description p;
      p.add("input-file", -1);

      variables_map vm;
      store(command_line_parser(ac, av).options(cmdline_options).positional(p).run(), vm);
      notify(vm);

      std::ifstream ifs(config_file.c_str());
      if(!ifs)
      {
         std::cout << "can not open config file: " << config_file << "\n";
         return 0;
      }
      else
      {
         store(parse_config_file(ifs, config_file_options), vm);
         notify(vm);
      }

      if(vm.count("help"))
      {
         std::cout << visible << "\n";
         return 0;
      }

      if(vm.count("version"))
      {
         std::cout << "Multiple sources example, version 1.0\n";
         return 0;
      }

      if(vm.count("include-path"))
      {
         std::cout << "Include paths are: " << vm["include-path"].as<std::vector<std::string>>() << "\n";
      }

      if(vm.count("input-file"))
      {
         std::cout << "Input files are: " << vm["input-file"].as<std::vector<std::string>>() << "\n";
      }

      std::cout << "Optimization level is " << opt << "\n";
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << "\n";
      return 1;
   }
   return 0;
}
