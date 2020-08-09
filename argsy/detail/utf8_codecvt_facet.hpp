// Copyright (c) 2001 Ronald Garcia, Indiana University (garcia@osl.iu.edu)
// Andrew Lumsdaine, Indiana University (lums@osl.iu.edu). Permission to copy,
// use, modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided "as is"
// without express or implied warranty, and with no claim as to its suitability
// for any purpose.
#pragma once

#include "argsy/config.hpp"

#define BOOST_UTF8_BEGIN_NAMESPACE                                                                                                         \
   namespace argsy                                                                                                                         \
   {                                                                                                                                       \
   namespace detail                                                                                                                        \
   {
#define BOOST_UTF8_END_NAMESPACE                                                                                                           \
   }                                                                                                                                       \
   }
#define BOOST_UTF8_DECL BOOST_PROGRAM_OPTIONS_DECL

#include <boost/detail/utf8_codecvt_facet.hpp>

#undef BOOST_UTF8_BEGIN_NAMESPACE
#undef BOOST_UTF8_END_NAMESPACE
#undef BOOST_UTF8_DECL
