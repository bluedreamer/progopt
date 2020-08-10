// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "program_options.hpp"

#include <iostream>

/* Auxiliary functions for checking input for validity. */

/* Function used to check that 'opt1' and 'opt2' are not specified
   at the same time. */
void conflicting_options(const argsy::variables_map &vm, const char *opt1, const char *opt2)
{
   if(vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) && !vm[opt2].defaulted())
   {
      throw std::logic_error(std::string("Conflicting options '") + opt1 + "' and '" + opt2 + "'.");
   }
}

/* Function used to check that of 'for_what' is specified, then
   'required_option' is specified too. */
void option_dependency(const argsy::variables_map &vm, const char *for_what, const char *required_option)
{
   if(vm.count(for_what) && !vm[for_what].defaulted())
   {
      if(vm.count(required_option) == 0 || vm[required_option].defaulted())
      {
         throw std::logic_error(std::string("Option '") + for_what + "' requires option '" + required_option + "'.");
      }
   }
}

auto main(int argc, char *argv[]) -> int
{
   try
   {
      std::string ofile;
      std::string macrofile, libmakfile;
      bool   t_given = false;
      bool   b_given = false;
      std::string mainpackage;
      std::string depends = "deps_file";
      std::string sources = "src_file";
      std::string root    = ".";

      argsy::options_description desc("Allowed options");
      desc.add_options()
         // First parameter describes option name/short name
         // The second is parameter to option
         // The third is description
         ("help,h", "print usage message")("output,o", argsy::value(&ofile), "pathname for output")(
            "macrofile,m", argsy::value(&macrofile), "full pathname of macro.h")("two,t", argsy::bool_switch(&t_given),
                                                                          "preprocess both header and body")(
            "body,b", argsy::bool_switch(&b_given), "preprocess body in the header context")("libmakfile,l", argsy::value(&libmakfile),
                                                                                      "write include makefile for library")(
            "mainpackage,p", argsy::value(&mainpackage), "output dependency information")("depends,d", argsy::value(&depends),
                                                                                   "write dependencies to <pathname>")(
            "sources,s", argsy::value(&sources), "write source package list to <pathname>")("root,r", argsy::value(&root),
                                                                                     "treat <dirname> as project root directory");

      argsy::variables_map vm;
      store(parse_command_line(argc, argv, desc), vm);

      if(vm.count("help"))
      {
         std::cout << desc << "\n";
         return 0;
      }

      conflicting_options(vm, "output", "two");
      conflicting_options(vm, "output", "body");
      conflicting_options(vm, "output", "mainpackage");
      conflicting_options(vm, "two", "mainpackage");
      conflicting_options(vm, "body", "mainpackage");

      conflicting_options(vm, "two", "body");
      conflicting_options(vm, "libmakfile", "mainpackage");
      conflicting_options(vm, "libmakfile", "mainpackage");

      option_dependency(vm, "depends", "mainpackage");
      option_dependency(vm, "sources", "mainpackage");
      option_dependency(vm, "root", "mainpackage");

      std::cout << "two = " << vm["two"].as<bool>() << "\n";
   }
   catch(std::exception &e)
   {
      std::cerr << e.what() << "\n";
   }
}
