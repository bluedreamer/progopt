// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* The simplest usage of the library.
 */

#include "argsy.h"

#include <iostream>
#include <iterator>

auto main(int ac, char *av[]) -> int
{
   try
   {
      argsy::options_description desc("Allowed options");
      desc.add_options()("help", "produce help message")("compression", argsy::value<double>(), "set compression level");

      argsy::variables_map vm;
      argsy::store(argsy::parse_command_line(ac, av, desc), vm);
      argsy::notify(vm);

      if(vm.count("help"))
      {
         std::cout << desc << "\n";
         return 0;
      }

      if(vm.count("compression"))
      {
         std::cout << "Compression level was set to " << vm["compression"].as<double>() << ".\n";
      }
      else
      {
         std::cout << "Compression level was not set.\n";
      }
   }
   catch(std::exception &e)
   {
      std::cerr << "error: " << e.what() << "\n";
      return 1;
   }
   catch(...)
   {
      std::cerr << "Exception of unknown type!\n";
   }

   return 0;
}
