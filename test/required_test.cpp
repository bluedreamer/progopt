// Copyright Sascha Ochsenknecht 2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "program_options.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "minitest.hpp"

void required_throw_test()
{
   argsy::options_description opts;
   opts.add_options()("cfgfile,c", argsy::value<std::string>()->required(),
                      "the configfile")("fritz,f", argsy::value<std::string>()->required(), "the output file");

   argsy::variables_map vm;
   bool                 thrown = false;
   {
      // This test must throw exception
      std::string              cmdline = "prg -f file.txt";
      std::vector<std::string> tokens  = argsy::split_unix(cmdline);
      thrown                           = false;
      try
      {
         store(argsy::command_line_parser(tokens).options(opts).run(), vm);
         notify(vm);
      }
      catch(argsy::required_option &e)
      {
         BOOST_CHECK_EQUAL(e.what(), std::string("the option '--cfgfile' is required but missing"));
         thrown = true;
      }
      BOOST_CHECK(thrown);
   }

   {
      // This test mustn't throw exception
      std::string              cmdline = "prg -c config.txt";
      std::vector<std::string> tokens  = argsy::split_unix(cmdline);
      thrown                           = false;
      try
      {
         store(argsy::command_line_parser(tokens).options(opts).run(), vm);
         notify(vm);
      }
      catch(argsy::required_option &e)
      {
         thrown = true;
      }
      BOOST_CHECK(!thrown);
   }
}

void simple_required_test(const char *config_file)
{
   argsy::options_description opts;
   opts.add_options()("cfgfile,c", argsy::value<std::string>()->required(),
                      "the configfile")("fritz,f", argsy::value<std::string>()->required(), "the output file");

   argsy::variables_map vm;
   bool                 thrown = false;
   {
      // This test must throw exception
      std::string              cmdline = "prg -f file.txt";
      std::vector<std::string> tokens  = argsy::split_unix(cmdline);
      thrown                           = false;
      try
      {
         // options coming from different sources
         store(argsy::command_line_parser(tokens).options(opts).run(), vm);
         store(parse_config_file<char>(config_file, opts), vm);
         notify(vm);
      }
      catch(argsy::required_option &e)
      {
         thrown = true;
      }
      BOOST_CHECK(!thrown);
   }
}

void multiname_required_test()
{
   argsy::options_description opts;
   opts.add_options()("foo,bar", argsy::value<std::string>()->required(), "the foo");

   argsy::variables_map vm;
   bool                 thrown = false;
   {
      // This test must throw exception
      std::string              cmdline = "prg --bar file.txt";
      std::vector<std::string> tokens  = argsy::split_unix(cmdline);
      thrown                           = false;
      try
      {
         // options coming from different sources
         store(argsy::command_line_parser(tokens).options(opts).run(), vm);
         notify(vm);
      }
      catch(argsy::required_option &e)
      {
         thrown = true;
      }
      BOOST_CHECK(!thrown);
   }
}

auto main(int /*argc*/, char *av[]) -> int
{
   required_throw_test();
   simple_required_test(av[1]);
   multiname_required_test();

   return 0;
}
