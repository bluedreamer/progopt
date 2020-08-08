// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* The simplest usage of the library.
 */

#include <program_options.h>

#include <iostream>
#include <iterator>

auto main(int ac, char *av[]) -> int
{
   try
   {
      options_description desc("Allowed options");
      desc.add_options()("help", "produce help message")("compression", value<double>(), "set compression level");

      variables_map vm;
      store(parse_command_line(ac, av, desc), vm);
      notify(vm);

      if(vm.count("help") != 0u)
      {
         std::cout << desc << "\n";
         return 0;
      }

      if(vm.count("compression") != 0u)
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
