// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/** This example shows how to support custom options syntax.

    It's possible to install 'custom_parser'. It will be invoked on all command
    line tokens and can return name/value pair, or nothing. If it returns
    nothing, usual processing will be done.
*/

#include <options_description.h>
#include <parsers.h>
#include <variables_map.h>

#include <iostream>

/*  This custom option parse function recognize gcc-style
    option "-fbar" / "-fno-bar".
*/
auto reg_foo(const std::string &s) -> std::pair<std::string, std::string>
{
   if(s.find("-f") == 0)
   {
      if(s.substr(2, 3) == "no-") {
         return std::make_pair(s.substr(5), std::string("false"));
      }  {
         return std::make_pair(s.substr(2), std::string("true"));
}
   }
   else
   {
      return std::make_pair(std::string(), std::string());
   }
}

auto main(int ac, char *av[]) -> int
{
   try
   {
      options_description desc("Allowed options");
      desc.add_options()("help", "produce a help message")("foo", value<std::string>(), "just an option");

      variables_map vm;
      store(command_line_parser(ac, av).options(desc).extra_parser(reg_foo).run(), vm);

      if(vm.count("help") != 0u)
      {
         std::cout << desc;
         std::cout << "\nIn addition -ffoo and -fno-foo syntax are recognized.\n";
      }
      if(vm.count("foo") != 0u)
      {
         std::cout << "foo value with the value of " << vm["foo"].as<std::string>() << "\n";
      }
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << "\n";
   }
}
