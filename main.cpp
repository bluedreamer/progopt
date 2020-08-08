#include "program_options.h"
#include <iostream>

auto main(int argc, char *argv[]) -> int
{
   try
   {
      options_description desc("Allowed options");
      desc.add_options()("help", "produce help message")("compression", value<double>(), "set compression level");

      variables_map vm;
      store(parse_command_line(argc, argv, desc), vm);
      notify(vm);

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
