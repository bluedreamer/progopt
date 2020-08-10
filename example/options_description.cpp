// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "program_options.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

// A helper function to simplify the main part.
template<class T>
auto operator<<(std::ostream &os, const std::vector<T> &v) -> std::ostream &
{
   copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
   return os;
}

auto main(int ac, char *av[]) -> int
{
   try
   {
      int                        opt;
      int                        portnum;
      argsy::options_description desc("Allowed options");
      desc.add_options()("help", "produce help message")("optimization", argsy::value<int>(&opt)->default_value(10), "optimization level")(
         "verbose,v", argsy::value<int>()->implicit_value(1), "enable verbosity (optionally specify level)")(
         "listen,l", argsy::value<int>(&portnum)->implicit_value(1001)->default_value(0, "no"),
         "listen on a port.")("include-path,I", argsy::value<std::vector<std::string>>(),
                              "include path")("input-file", argsy::value<std::vector<std::string>>(), "input file");

      argsy::positional_options_description p;
      p.add("input-file", -1);

      argsy::variables_map vm;
      argsy::store(argsy::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
      argsy::notify(vm);

      if(vm.count("help"))
      {
         std::cout << "Usage: options_description [options]\n";
         std::cout << desc;
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

      if(vm.count("verbose"))
      {
         std::cout << "Verbosity enabled.  Level is " << vm["verbose"].as<int>() << "\n";
      }

      std::cout << "Optimization level is " << opt << "\n";

      std::cout << "Listen port is " << portnum << "\n";
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << "\n";
      return 1;
   }
   return 0;
}
