// Copyright Vladimir Prus 2002-2004.
// Copyright Bertolt Mildner 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include "argsy/config.hpp"
#include "argsy/errors.hpp"
#include "argsy/value_semantic.hpp"

#include <any>
//#include <boost/detail/workaround.hpp>
#include <functional>

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <iosfwd>

namespace argsy
{
/** Describes one possible command line/config file option. There are two
    kinds of properties of an option. First describe it syntactically and
    are used only to validate input. Second affect interpretation of the
    option, for example default value for it or function that should be
    called  when the value is finally known. Routines which perform parsing
    never use second kind of properties \-- they are side effect free.
    @sa options_description
*/
class option_description
{
public:
   option_description();

   /** Initializes the object with the passed data.

       Note: it would be nice to make the second parameter auto_ptr,
       to explicitly pass ownership. Unfortunately, it's often needed to
       create objects of types derived from 'value_semantic':
          options_description d;
          d.add_options()("a", parameter<int>("n")->default_value(1));
       Here, the static type returned by 'parameter' should be derived
       from value_semantic.

       Alas, derived->base conversion for auto_ptr does not really work,
       see
       http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2000/n1232.pdf
       http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#84

       So, we have to use plain old pointers. Besides, users are not
       expected to use the constructor directly.


       The 'name' parameter is interpreted by the following rules:
       - if there's no "," character in 'name', it specifies long name
       - otherwise, the part before "," specifies long name and the part
       after \-- short name.
   */
   option_description(const char *name, const value_semantic *s);

   /** Initializes the class with the passed data.
    */
   option_description(const char *name, const value_semantic *s, const char *description);

   virtual ~option_description();

   enum match_result
   {
      no_match,
      full_match,
      approximate_match
   };

   /** Given 'option', specified in the input source,
       returns 'true' if 'option' specifies *this.
   */
   [[nodiscard]] auto match(const std::string &option, bool approx, bool long_ignore_case, bool short_ignore_case) const -> match_result;

   /** Returns the key that should identify the option, in
       particular in the variables_map class.
       The 'option' parameter is the option spelling from the
       input source.
       If option name contains '*', returns 'option'.
       If long name was specified, it's the long name, otherwise
       it's a short name with prepended '-'.
   */
   [[nodiscard]] auto key(const std::string &option) const -> const std::string &;

   /** Returns the canonical name for the option description to enable the user to
       recognised a matching option.
       1) For short options ('-', '/'), returns the short name prefixed.
       2) For long options ('--' / '-') returns the first long name prefixed
       3) All other cases, returns the first long name (if present) or the short
          name, unprefixed.
   */
   [[nodiscard]] auto canonical_display_name(int canonical_option_style = 0) const -> std::string;

   [[nodiscard]] auto long_name() const -> const std::string &;

   [[nodiscard]] auto long_names() const -> const std::pair<const std::string *, std::size_t>;

   /// Explanation of this option
   [[nodiscard]] auto description() const -> const std::string &;

   /// Semantic of option's value
   [[nodiscard]] auto semantic() const -> std::shared_ptr<const value_semantic>;

   /// Returns the option name, formatted suitably for usage message.
   [[nodiscard]] auto format_name() const -> std::string;

   /** Returns the parameter name and properties, formatted suitably for
       usage message. */
   [[nodiscard]] auto format_parameter() const -> std::string;

private:
   auto set_names(const char *name) -> option_description &;

   /**
    * a one-character "switch" name - with its prefix,
    * so that this is either empty or has length 2 (e.g. "-c"
    */
   std::string m_short_name;

   /**
    *  one or more names by which this option may be specified
    *  on a command-line or in a config file, which are not
    *  a single-letter switch. The names here are _without_
    * any prefix.
    */
   std::vector<std::string> m_long_names;

   std::string m_description;

   // shared_ptr is needed to simplify memory management in
   // copy ctor and destructor.
   std::shared_ptr<const value_semantic> m_value_semantic;
};

class options_description;

/** Class which provides convenient creation syntax to option_description.
 */
class options_description_easy_init
{
public:
   options_description_easy_init(options_description *owner);

   auto operator()(const char *name, const char *description) -> options_description_easy_init &;

   auto operator()(const char *name, const value_semantic *s) -> options_description_easy_init &;

   auto operator()(const char *name, const value_semantic *s, const char *description) -> options_description_easy_init &;

private:
   options_description *owner;
};

/** A set of option descriptions. This provides convenient interface for
    adding new option (the add_options) method, and facilities to search
    for options by name.

    See @ref a_adding_options "here" for option adding interface discussion.
    @sa option_description
*/
class options_description
{
public:
   static const unsigned m_default_line_length;

   /** Creates the instance. */
   options_description(unsigned line_length = m_default_line_length, unsigned min_description_length = m_default_line_length / 2);
   /** Creates the instance. The 'caption' parameter gives the name of
       this 'options_description' instance. Primarily useful for output.
       The 'description_length' specifies the number of columns that
       should be reserved for the description text; if the option text
       encroaches into this, then the description will start on the next
       line.
   */
   options_description(std::string caption, unsigned line_length = m_default_line_length,
                       unsigned min_description_length = m_default_line_length / 2);
   /** Adds new variable description. Throws duplicate_variable_error if
       either short or long name matches that of already present one.
   */
   void add(std::shared_ptr<option_description> desc);
   /** Adds a group of option description. This has the same
       effect as adding all option_descriptions in 'desc'
       individually, except that output operator will show
       a separate group.
       Returns *this.
   */
   auto add(const options_description &desc) -> options_description &;

   /** Find the maximum width of the option column, including options
       in groups. */
   [[nodiscard]] auto get_option_column_width() const -> unsigned;

public:
   /** Returns an object of implementation-defined type suitable for adding
       options to options_description. The returned object will
       have overloaded operator() with parameter type matching
       'option_description' constructors. Calling the operator will create
       new option_description instance and add it.
   */
   auto add_options() -> options_description_easy_init;

   [[nodiscard]] auto find(const std::string &name, bool approx, bool long_ignore_case = false, bool short_ignore_case = false) const
      -> const option_description &;

   [[nodiscard]] auto find_nothrow(const std::string &name, bool approx, bool long_ignore_case = false,
                                   bool short_ignore_case = false) const -> const option_description *;

   [[nodiscard]] auto options() const -> const std::vector<std::shared_ptr<option_description>> &;

   /** Produces a human readable output of 'desc', listing options,
       their descriptions and allowed parameters. Other options_description
       instances previously passed to add will be output separately. */
   friend auto operator<<(std::ostream &os, const options_description &desc) -> std::ostream &;

   /** Outputs 'desc' to the specified stream, calling 'f' to output each
       option_description element. */
   void print(std::ostream &os, unsigned width = 0) const;

private:
#if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1800))
   // prevent warning C4512: assignment operator could not be generated
   options_description &operator=(const options_description &);
#endif

   typedef std::map<std::string, int>::const_iterator          name2index_iterator;
   typedef std::pair<name2index_iterator, name2index_iterator> approximation_range;

   // approximation_range find_approximation(const std::string& prefix) const;

   std::string    m_caption;
   const unsigned m_line_length;
   const unsigned m_min_description_length;

   // Data organization is chosen because:
   // - there could be two names for one option
   // - option_add_proxy needs to know the last added option
   std::vector<std::shared_ptr<option_description>> m_options;

   // Whether the option comes from one of declared groups.
#if BOOST_WORKAROUND(BOOST_DINKUMWARE_STDLIB, BOOST_TESTED_AT(313))
   // vector<bool> is buggy there, see
   // http://support.microsoft.com/default.aspx?scid=kb;en-us;837698
   std::vector<char> belong_to_group;
#else
   std::vector<bool> belong_to_group;
#endif

   std::vector<std::shared_ptr<options_description>> groups;
};

/** Class thrown when duplicate option description is found. */
class duplicate_option_error : public error
{
public:
   duplicate_option_error(const std::string &xwhat)
      : error(xwhat)
   {
   }
};
} // namespace argsy
