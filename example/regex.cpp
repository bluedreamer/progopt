// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This example shows how a user-defined class can be parsed using
// specific mechanism -- not the iostream operations used by default.
//
// A new class 'magic_number' is defined and the 'validate' method is overloaded
// to validate the values of that class using Boost.Regex.
// To test, run
//
//    regex -m 123-456
//    regex -m 123-4567
//
// The first invocation should output:
//
//   The magic is "456"
//
// and the second invocation should issue an error message.

#include "program_options.hpp"

#include <regex>
#include <iostream>

/* Define a completely non-sensical class. */
struct magic_number
{
public:
   magic_number(int n)
      : n(n)
   {
   }
   int n;
};

/* Overload the 'validate' function for the user-defined class.
   It makes sure that value is of form XXX-XXX
   where X are digits and converts the second group to an integer.
   This has no practical meaning, meant only to show how
   regex can be used to validate values.
*/
void validate(std::any &v, const std::vector<std::string> &values, magic_number * /*unused*/, int /*unused*/)
{
   static std::regex r(R"(\d\d\d-(\d\d\d))");

   // Make sure no previous assignment to 'a' was made.
   argsy::validators::check_first_occurrence(v);
   // Extract the first string from 'values'. If there is more than
   // one string, it's an error, and exception will be thrown.
   const std::string &s = argsy::validators::get_single_string(values);

   // Do regex match and convert the interesting part to
   // int.
   std::smatch match;
   if(regex_match(s, match, r))
   {
      v = std::any(magic_number(boost::lexical_cast<int>(match[1])));
   }
   else
   {
      throw argsy::validation_error(argsy::validation_error::invalid_option_value);
   }
}

auto main(int ac, char *av[]) -> int
{
   try
   {
      argsy::options_description desc("Allowed options");
      desc.add_options()("help", "produce a help screen")("version,v", "print the version number")("magic,m", argsy::value<magic_number>(),
                                                                                                   "magic value (in NNN-NNN format)");

      argsy::variables_map vm;
      store(parse_command_line(ac, av, desc), vm);

      if(vm.count("help"))
      {
         std::cout << "Usage: regex [options]\n";
         std::cout << desc;
         return 0;
      }
      if(vm.count("version"))
      {
         std::cout << "Version 1.\n";
         return 0;
      }
      if(vm.count("magic"))
      {
         std::cout << "The magic is \"" << vm["magic"].as<magic_number>().n << "\"\n";
      }
   }
   catch(std::exception &e)
   {
      std::cout << e.what() << "\n";
   }
}
