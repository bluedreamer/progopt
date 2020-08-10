// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/** This example shows how to handle options groups.

    For a test, run:

    option_groups --help
    option_groups --num-threads 10
    option_groups --help-module backend

    The first invocation would show to option groups, and will not show the
    '--num-threads' options. The second invocation will still get the value of
    the hidden '--num-threads' option. Finally, the third invocation will show
    the options for the 'backend' module, including the '--num-threads' option.

*/

#include "argsy/options_description.hpp"
#include "argsy/parsers.hpp"
#include "argsy/variables_map.hpp"
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>

#include <exception>
#include <fstream>
#include <iostream>

auto main(int ac, char *av[]) -> int
{
   try
   {
      // Declare three groups of options.
      argsy::options_description general("General options");
      general.add_options()("help", "produce a help message")("help-module", argsy::value<std::string>(),
                                                              "produce a help for a given module")("version", "output the version number");

      argsy::options_description gui("GUI options");
      gui.add_options()("display", argsy::value<std::string>(), "display to use");

      argsy::options_description backend("Backend options");
      backend.add_options()("num-threads", argsy::value<int>(), "the initial number of threads");

      // Declare an options description instance which will include
      // all the options
      argsy::options_description all("Allowed options");
      all.add(general).add(gui).add(backend);

      // Declare an options description instance which will be shown
      // to the user
      argsy::options_description visible("Allowed options");
      visible.add(general).add(gui);

      argsy::variables_map vm;
      store(parse_command_line(ac, av, all), vm);

      if(vm.count("help"))
      {
         std::cout << visible;
         return 0;
      }
      if(vm.count("help-module"))
      {
         const auto &s = vm["help-module"].as<std::string>();
         if(s == "gui")
         {
            std::cout << gui;
         }
         else if(s == "backend")
         {
            std::cout << backend;
         }
         else
         {
            std::cout << "Unknown module '" << s << "' in the --help-module option\n";
            return 1;
         }
         return 0;
      }
      if(vm.count("num-threads"))
      {
         std::cout << "The 'num-threads' options was set to " << vm["num-threads"].as<int>() << "\n";
      }
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << "\n";
   }
}
