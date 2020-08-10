// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_PROGRAM_OPTIONS_SOURCE
#include "argsy/config.h"

#define BOOST_UTF8_BEGIN_NAMESPACE                                                                                                         \
   namespace argsy                                                                                                                         \
   {                                                                                                                                       \
   namespace detail                                                                                                                        \
   {
#define BOOST_UTF8_END_NAMESPACE                                                                                                           \
   }                                                                                                                                       \
   }

#include <boost/detail/utf8_codecvt_facet.ipp>

#undef BOOST_UTF8_BEGIN_NAMESPACE
#undef BOOST_UTF8_END_NAMESPACE
#undef BOOST_UTF8_DECL
