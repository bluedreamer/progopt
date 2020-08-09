// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
#include "argsy/config.hpp"
#include "argsy/positional_options.hpp"

#include <cassert>

namespace argsy
{
positional_options_description::positional_options_description() = default;

auto positional_options_description::add(const char *name, int max_count) -> positional_options_description &
{
   assert(max_count != -1 || m_trailing.empty());

   if(max_count == -1)
   {
      m_trailing = name;
   }
   else
   {
      m_names.resize(m_names.size() + max_count, name);
   }
   return *this;
}

auto positional_options_description::max_total_count() const -> unsigned
{
   return m_trailing.empty() ? static_cast<unsigned>(m_names.size()) : (std::numeric_limits<unsigned>::max)();
}

auto positional_options_description::name_for_position(unsigned position) const -> const std::string &
{
   assert(position < max_total_count());

   if(position < m_names.size())
   {
      return m_names[position];
   }
   else
   {
      return m_trailing;
   }
}

} // namespace argsy
