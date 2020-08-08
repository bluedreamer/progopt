// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <any>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

template<class charT>
class basic_parsed_options;

class value_semantic;
class variables_map;

// forward declaration

/** Stores in 'm' all options that are defined in 'options'.
    If 'm' already has a non-defaulted value of an option, that value
    is not changed, even if 'options' specify some value.
*/
void store(const basic_parsed_options<char> &options, variables_map &m, bool utf8 = false);

/** Stores in 'm' all options that are defined in 'options'.
    If 'm' already has a non-defaulted value of an option, that value
    is not changed, even if 'options' specify some value.
    This is wide character variant.
*/
void store(const basic_parsed_options<wchar_t> &options, variables_map &m);

/** Runs all 'notify' function for options in 'm'. */
void notify(variables_map &m);

/** Class holding value of option. Contains details about how the
    value is set and allows to conveniently obtain the value.
*/
class variable_value
{
public:
   variable_value()

      = default;
   variable_value(std::any xv, bool xdefaulted)
      : v(std::move(xv))
      , m_defaulted(xdefaulted)
   {
   }

   /** If stored value if of type T, returns that value. Otherwise,
       throws boost::bad_any_cast exception. */
   template<class T>
   auto as() const -> const T &
   {
      return std::any_cast<const T &>(v);
   }
   /** @overload */
   template<class T>
   auto as() -> T &
   {
      return std::any_cast<T &>(v);
   }

   /// Returns true if no value is stored.
   [[nodiscard]] auto empty() const -> bool;
   /** Returns true if the value was not explicitly
       given, but has default value. */
   [[nodiscard]] auto defaulted() const -> bool;
   /** Returns the contained value. */
   [[nodiscard]] auto value() const -> const std::any &;

   /** Returns the contained value. */
   auto value() -> std::any &;

private:
   std::any v;
   bool     m_defaulted{false};
   // Internal reference to value semantic. We need to run
   // notifications when *final* values of options are known, and
   // they are known only after all sources are stored. By that
   // time options_description for the first source might not
   // be easily accessible, so we need to store semantic here.
   std::shared_ptr<const value_semantic> m_value_semantic;

   friend void store(const basic_parsed_options<char> &options, variables_map &m, bool);

   friend class variables_map;
};

/** Implements string->string mapping with convenient value casting
    facilities. */
class abstract_variables_map
{
public:
   abstract_variables_map();
   abstract_variables_map(const abstract_variables_map *next);

   virtual ~abstract_variables_map() = default;

   /** Obtains the value of variable 'name', from *this and
       possibly from the chain of variable maps.

       - if there's no value in *this.
           - if there's next variable map, returns value from it
           - otherwise, returns empty value

       - if there's defaulted value
           - if there's next variable map, which has a non-defaulted
             value, return that
           - otherwise, return value from *this

       - if there's a non-defaulted value, returns it.
   */
   auto operator[](const std::string &name) const -> const variable_value &;

   /** Sets next variable map, which will be used to find
      variables not found in *this. */
   void next(abstract_variables_map *next);

private:
   /** Returns value of variable 'name' stored in *this, or
       empty value otherwise. */
   [[nodiscard]] virtual auto get(const std::string &name) const -> const variable_value & = 0;

   const abstract_variables_map *m_next;
};

/** Concrete variables map which store variables in real map.

    This class is derived from std::map<std::string, variable_value>,
    so you can use all map operators to examine its content.
*/
class variables_map : public abstract_variables_map, public std::map<std::string, variable_value>
{
public:
   variables_map();
   variables_map(const abstract_variables_map *next);

   // Resolve conflict between inherited operators.
   auto operator[](const std::string &name) const -> const variable_value & { return abstract_variables_map::operator[](name); }

   // Override to clear some extra fields.
   void clear();

   void notify();

private:
   /** Implementation of abstract_variables_map::get
       which does 'find' in *this. */
   [[nodiscard]] auto get(const std::string &name) const -> const variable_value &;

   /** Names of option with 'final' values \-- which should not
       be changed by subsequence assignments. */
   std::set<std::string> m_final;

   friend void store(const basic_parsed_options<char> &options, variables_map &xm, bool utf8);

   /** Names of required options, filled by parser which has
       access to options_description.
       The map values are the "canonical" names for each corresponding option.
       This is useful in creating diagnostic messages when the option is absent. */
   std::map<std::string, std::string> m_required;
};

/*
 * Templates/inlines
 */

inline auto variable_value::empty() const -> bool
{
   return !v.has_value();
}

inline auto variable_value::defaulted() const -> bool
{
   return m_defaulted;
}

inline auto variable_value::value() const -> const std::any &
{
   return v;
}

inline auto variable_value::value() -> std::any &
{
   return v;
}
