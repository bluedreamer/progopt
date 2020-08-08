// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "eof_iterator.h"

#include <cassert>
#include <string>
#include <utility>

class environment_iterator : public eof_iterator<environment_iterator, std::pair<std::string, std::string>>
{
public:
   environment_iterator(char **environment)
      : m_environment(environment)
   {
      get();
   }

   environment_iterator() { found_eof(); }

   void get()
   {
      if(*m_environment == nullptr)
      {
         found_eof();
      }
      else
      {
         std::string            s(*m_environment);
         std::string::size_type n = s.find('=');
         assert(n != s.npos);
         value().first  = s.substr(0, n);
         value().second = s.substr(n + 1);

         ++m_environment;
      }
   }

private:
   char **m_environment;
};
