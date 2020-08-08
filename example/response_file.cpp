// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/** This example shows how to handle response file.

    For a test, build and run:
       response_file -I foo @response_file.rsp

    The expected output is:
      Include paths: foo bar biz

    Thanks to Hartmut Kaiser who raised the issue of response files
    and discussed the possible approach.
*/

#include <options_description.h>
#include <parsers.h>
#include <variables_map.h>

#include <fstream>
#include <iostream>

// Additional command line parser which interprets '@something' as a
// option "config-file" with the value "something"

auto at_option_parser(std::string const &s) -> std::pair<std::string, std::string>
{
   if('@' == s[0]) {
      return std::make_pair(std::string("response-file"), s.substr(1));
   }  {
      return std::pair<std::string, std::string>();
}
}

auto main(int ac, char *av[]) -> int
{
   try
   {
      options_description desc("Allowed options");
      desc.add_options()("help", "produce a help message")("include-path,I", value<std::vector<std::string>>()->composing(), "include path")(
         "magic", value<int>(), "magic value")("response-file", value<std::string>(), "can be specified with '@name', too");

      variables_map vm;
      store(command_line_parser(ac, av).options(desc).extra_parser(at_option_parser).run(), vm);

      if(vm.count("help") != 0u)
      {
         std::cout << desc;
      }
      if(vm.count("response-file") != 0u)
      {
         // Load the file and tokenize it
         std::ifstream ifs(vm["response-file"].as<std::string>().c_str());
         if(!ifs)
         {
            std::cout << "Could not open the response file\n";
            return 1;
         }
         // Read the whole file into a string
         std::stringstream ss;
         ss << ifs.rdbuf();
         // Split the file content
//         char_separator<char>            sep(" \n\r");
//         std::string                     sstr = ss.str();
//         tokenizer<char_separator<char>> tok(sstr, sep);
//         std::vector<std::string>        args;
//         std::copy(tok.begin(), tok.end(), back_inserter(args));
//         // Parse the file and store the options
//         store(command_line_parser(args).options(desc).run(), vm);
      }

      if(vm.count("include-path") != 0u)
      {
         const std::vector<std::string> &s = vm["include-path"].as<std::vector<std::string>>();
         std::cout << "Include paths: ";
         std::copy(s.begin(), s.end(), std::ostream_iterator<std::string>(std::cout, " "));
         std::cout << "\n";
      }
      if(vm.count("magic") != 0u)
      {
         std::cout << "Magic value: " << vm["magic"].as<int>() << "\n";
      }
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << "\n";
   }
}
