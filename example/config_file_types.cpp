// Copyright Thomas Kent 2016
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This example shows a config file (in ini format) being parsed by the
// argsy library. It includes a numebr of different value types.

#include "argsy.h"

#include <cassert>
#include <iostream>
#include <sstream>

const double FLOAT_SEPERATION = 0.00000000001;
auto         check_float(double test, double expected) -> bool
{
   double seperation = expected * (1 + FLOAT_SEPERATION) / expected;
   if((test < expected + seperation) && (test > expected - seperation))
   {
      return true;
   }
   return false;
}

auto make_file() -> std::stringstream
{
   std::stringstream ss;
   ss << "# This file checks parsing of various types of config values\n";
   // FAILS: ss << "; a windows style comment\n";

   ss << "global_string = global value\n";
   ss << "unregistered_entry = unregistered value\n";

   ss << "\n[strings]\n";
   ss << "word = word\n";
   ss << "phrase = this is a phrase\n";
   ss << "quoted = \"quotes are in result\"\n";

   ss << "\n[ints]\n";
   ss << "positive = 41\n";
   ss << "negative = -42\n";
   // FAILS: Lexical cast doesn't support hex, oct, or bin
   // ss << "hex = 0x43\n";
   // ss << "oct = 044\n";
   // ss << "bin = 0b101010\n";

   ss << "\n[floats]\n";
   ss << "positive = 51.1\n";
   ss << "negative = -52.1\n";
   ss << "double = 53.1234567890\n";
   ss << "int = 54\n";
   ss << "int_dot = 55.\n";
   ss << "dot = .56\n";
   ss << "exp_lower = 57.1e5\n";
   ss << "exp_upper = 58.1E5\n";
   ss << "exp_decimal = .591e5\n";
   ss << "exp_negative = 60.1e-5\n";
   ss << "exp_negative_val = -61.1e5\n";
   ss << "exp_negative_negative_val = -62.1e-5\n";

   ss << "\n[booleans]\n";
   ss << "number_true = 1\n";
   ss << "number_false = 0\n";
   ss << "yn_true = yes\n";
   ss << "yn_false = no\n";
   ss << "tf_true = true\n";
   ss << "tf_false = false\n";
   ss << "onoff_true = on\n";
   ss << "onoff_false = off\n";
   ss << "present_equal_true = \n";
   // FAILS: Must be an =
   // ss << "present_no_equal_true\n";

   ss.seekp(std::ios_base::beg);
   return ss;
}

auto set_options() -> argsy::options_description
{
   argsy::options_description opts;
   opts.add_options()("global_string", argsy::value<std::string>())

      ("strings.word", argsy::value<std::string>())("strings.phrase", argsy::value<std::string>())("strings.quoted",
                                                                                                   argsy::value<std::string>())

         ("ints.positive", argsy::value<int>())("ints.negative", argsy::value<int>())("ints.hex", argsy::value<int>())(
            "ints.oct", argsy::value<int>())("ints.bin", argsy::value<int>())

            ("floats.positive", argsy::value<float>())("floats.negative", argsy::value<float>())("floats.double", argsy::value<double>())(
               "floats.int", argsy::value<float>())("floats.int_dot", argsy::value<float>())("floats.dot", argsy::value<float>())(
               "floats.exp_lower", argsy::value<float>())("floats.exp_upper", argsy::value<float>())(
               "floats.exp_decimal", argsy::value<float>())("floats.exp_negative", argsy::value<float>())(
               "floats.exp_negative_val", argsy::value<float>())("floats.exp_negative_negative_val", argsy::value<float>())

      // Load booleans as value<bool>, so they will require a --option=value on the command line
      //("booleans.number_true", argsy::value<bool>())
      //("booleans.number_false", argsy::value<bool>())
      //("booleans.yn_true", argsy::value<bool>())
      //("booleans.yn_false", argsy::value<bool>())
      //("booleans.tf_true", argsy::value<bool>())
      //("booleans.tf_false", argsy::value<bool>())
      //("booleans.onoff_true", argsy::value<bool>())
      //("booleans.onoff_false", argsy::value<bool>())
      //("booleans.present_equal_true", argsy::value<bool>())
      //("booleans.present_no_equal_true", argsy::value<bool>())

      // Load booleans as bool_switch, so that a --option will set it true on the command line
      // The difference between these two types does not show up when parsing a file
      ("booleans.number_true", argsy::bool_switch())("booleans.number_false", argsy::bool_switch())(
         "booleans.yn_true", argsy::bool_switch())("booleans.yn_false", argsy::bool_switch())("booleans.tf_true", argsy::bool_switch())(
         "booleans.tf_false", argsy::bool_switch())("booleans.onoff_true", argsy::bool_switch())(
         "booleans.onoff_false", argsy::bool_switch())("booleans.present_equal_true",
                                                       argsy::bool_switch())("booleans.present_no_equal_true", argsy::bool_switch());
   return opts;
}

auto parse_file(std::stringstream &file, argsy::options_description &opts, argsy::variables_map &vm) -> std::vector<std::string>
{
   const bool ALLOW_UNREGISTERED = true;
   std::cout << file.str() << std::endl;

   argsy::parsed_options parsed = parse_config_file(file, opts, ALLOW_UNREGISTERED);
   store(parsed, vm);
   std::vector<std::string> unregistered = argsy::collect_unrecognized(parsed.options, argsy::exclude_positional);
   notify(vm);

   return unregistered;
}

void check_results(argsy::variables_map &vm, std::vector<std::string> unregistered)
{
   // Check that we got the correct values back
   std::string expected_global_string = "global value";

   std::string expected_unreg_option = "unregistered_entry";
   std::string expected_unreg_value  = "unregistered value";

   std::string expected_strings_word   = "word";
   std::string expected_strings_phrase = "this is a phrase";
   std::string expected_strings_quoted = "\"quotes are in result\"";

   int expected_int_postitive = 41;
   int expected_int_negative  = -42;
   int expected_int_hex       = 0x43;
   int expected_int_oct       = 044;
   int expected_int_bin       = 0b101010;

   float  expected_float_positive                  = 51.1f;
   float  expected_float_negative                  = -52.1f;
   double expected_float_double                    = 53.1234567890;
   float  expected_float_int                       = 54.0f;
   float  expected_float_int_dot                   = 55.0f;
   float  expected_float_dot                       = .56f;
   float  expected_float_exp_lower                 = 57.1e5f;
   float  expected_float_exp_upper                 = 58.1E5f;
   float  expected_float_exp_decimal               = .591e5f;
   float  expected_float_exp_negative              = 60.1e-5f;
   float  expected_float_exp_negative_val          = -61.1e5f;
   float  expected_float_exp_negative_negative_val = -62.1e-5f;

   bool expected_number_true           = true;
   bool expected_number_false          = false;
   bool expected_yn_true               = true;
   bool expected_yn_false              = false;
   bool expected_tf_true               = true;
   bool expected_tf_false              = false;
   bool expected_onoff_true            = true;
   bool expected_onoff_false           = false;
   bool expected_present_equal_true    = true;
   bool expected_present_no_equal_true = true;

   assert(vm["global_string"].as<std::string>() == expected_global_string);

   assert(unregistered[0] == expected_unreg_option);
   assert(unregistered[1] == expected_unreg_value);

   assert(vm["strings.word"].as<std::string>() == expected_strings_word);
   assert(vm["strings.phrase"].as<std::string>() == expected_strings_phrase);
   assert(vm["strings.quoted"].as<std::string>() == expected_strings_quoted);

   assert(vm["ints.positive"].as<int>() == expected_int_postitive);
   assert(vm["ints.negative"].as<int>() == expected_int_negative);
   // assert(vm["ints.hex"].as<int>() == expected_int_hex);
   // assert(vm["ints.oct"].as<int>() == expected_int_oct);
   // assert(vm["ints.bin"].as<int>() == expected_int_bin);

   assert(check_float(vm["floats.positive"].as<float>(), expected_float_positive));
   assert(check_float(vm["floats.negative"].as<float>(), expected_float_negative));
   assert(check_float(vm["floats.double"].as<double>(), expected_float_double));
   assert(check_float(vm["floats.int"].as<float>(), expected_float_int));
   assert(check_float(vm["floats.int_dot"].as<float>(), expected_float_int_dot));
   assert(check_float(vm["floats.dot"].as<float>(), expected_float_dot));
   assert(check_float(vm["floats.exp_lower"].as<float>(), expected_float_exp_lower));
   assert(check_float(vm["floats.exp_upper"].as<float>(), expected_float_exp_upper));
   assert(check_float(vm["floats.exp_decimal"].as<float>(), expected_float_exp_decimal));
   assert(check_float(vm["floats.exp_negative"].as<float>(), expected_float_exp_negative));
   assert(check_float(vm["floats.exp_negative_val"].as<float>(), expected_float_exp_negative_val));
   assert(check_float(vm["floats.exp_negative_negative_val"].as<float>(), expected_float_exp_negative_negative_val));

   assert(vm["booleans.number_true"].as<bool>() == expected_number_true);
   assert(vm["booleans.number_false"].as<bool>() == expected_number_false);
   assert(vm["booleans.yn_true"].as<bool>() == expected_yn_true);
   assert(vm["booleans.yn_false"].as<bool>() == expected_yn_false);
   assert(vm["booleans.tf_true"].as<bool>() == expected_tf_true);
   assert(vm["booleans.tf_false"].as<bool>() == expected_tf_false);
   assert(vm["booleans.onoff_true"].as<bool>() == expected_onoff_true);
   assert(vm["booleans.onoff_false"].as<bool>() == expected_onoff_false);
   assert(vm["booleans.present_equal_true"].as<bool>() == expected_present_equal_true);
   // assert(vm["booleans.present_no_equal_true"].as<bool>() == expected_present_no_equal_true);
}

auto main(int ac, char *av[]) -> int
{
   auto                 file = make_file();
   auto                 opts = set_options();
   argsy::variables_map vars;
   auto                 unregistered = parse_file(file, opts, vars);
   check_results(vars, unregistered);

   return 0;
}
