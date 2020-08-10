// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy.h"
namespace po = argsy;

#include <boost/optional.hpp>

#include <string>

#include "minitest.hpp"

auto sv(const char *array[], unsigned size) -> std::vector<std::string>
{
   std::vector<std::string> r;
   for(unsigned i = 0; i < size; ++i)
   {
      r.emplace_back(array[i]);
   }
   return r;
}

void test_optional()
{
   std::optional<int> foo, bar, baz;

   argsy::options_description desc;
   desc.add_options()("foo,f", argsy::value(&foo), "")("bar,b", argsy::value(&bar), "")("baz,z", argsy::value(&baz), "");

   const char *             cmdline1_[] = {"--foo=12", "--bar", "1"};
   std::vector<std::string> cmdline1    = sv(cmdline1_, sizeof(cmdline1_) / sizeof(const char *));
   argsy::variables_map     vm;
   argsy::store(argsy::command_line_parser(cmdline1).options(desc).run(), vm);
   argsy::notify(vm);

   BOOST_REQUIRE(!!foo);
   BOOST_CHECK(*foo == 12);

   BOOST_REQUIRE(!!bar);
   BOOST_CHECK(*bar == 1);

   BOOST_CHECK(!baz);
}

auto main(int, char *[]) -> int
{
   test_optional();
   return 0;
}
