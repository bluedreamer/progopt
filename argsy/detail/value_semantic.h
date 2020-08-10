// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// This file defines template functions that are declared in
// ../value_semantic.hpp.

#pragma once

namespace argsy
{
extern std::string arg;

template<class T, class charT>
std::string typed_value<T, charT>::name() const
{
   std::string const &var = (m_value_name.empty() ? arg : m_value_name);
   if(m_implicit_value.has_value() && !m_implicit_value_as_text.empty())
   {
      std::string msg = "[=" + var + "(=" + m_implicit_value_as_text + ")]";
      if(m_default_value.has_value() && !m_default_value_as_text.empty())
         msg += " (=" + m_default_value_as_text + ")";
      return msg;
   }
   else if(m_default_value.has_value() && !m_default_value_as_text.empty())
   {
      return var + " (=" + m_default_value_as_text + ")";
   }
   else
   {
      return var;
   }
}

template<class T, class charT>
void typed_value<T, charT>::notify(const std::any &value_store) const
{
   const T *value = std::any_cast<T>(&value_store);
   if(m_store_to)
   {
      *m_store_to = *value;
   }
   if(m_notifier)
   {
      m_notifier(*value);
   }
}

namespace validators
{
/* If v.size() > 1, throw validation_error.
   If v.size() == 1, return v.front()
   Otherwise, returns a reference to a statically allocated
   empty string if 'allow_empty' and throws validation_error
   otherwise. */
template<class charT>
const std::basic_string<charT> &get_single_string(const std::vector<std::basic_string<charT>> &v, bool allow_empty = false)
{
   static std::basic_string<charT> empty;
   if(v.size() > 1)
      boost::throw_exception(validation_error(validation_error::multiple_values_not_allowed));
   else if(v.size() == 1)
      return v.front();
   else if(!allow_empty)
      boost::throw_exception(validation_error(validation_error::at_least_one_value_required));
   return empty;
}

/* Throws multiple_occurrences if 'value' is not empty. */
void check_first_occurrence(const std::any &value);
} // namespace validators

/** Validates 's' and updates 'v'.
    @pre 'v' is either empty or in the state assigned by the previous
    invocation of 'validate'.
    The target type is specified via a parameter which has the type of
    pointer to the desired type. This is workaround for compilers without
    partial template ordering, just like the last 'long/int' parameter.
*/
template<class T, class charT>
void validate(std::any &v, const std::vector<std::basic_string<charT>> &xs, T *, long /*unused*/)
{
   validators::check_first_occurrence(v);
   std::basic_string<charT> s(validators::get_single_string(xs));
   try
   {
      v = std::any(boost::lexical_cast<T>(s));
   }
   catch(const boost::bad_lexical_cast &)
   {
      boost::throw_exception(invalid_option_value(s));
   }
}

void validate(std::any &v, const std::vector<std::string> &xs, bool *, int);
void validate(std::any &v, const std::vector<std::wstring> &xs, bool *, int);
void validate(std::any &v, const std::vector<std::string> &xs, std::string *, int);
void validate(std::any &v, const std::vector<std::wstring> &xs, std::string *, int);

/** Validates sequences. Allows multiple values per option occurrence
   and multiple occurrences. */
template<class T, class charT>
void validate(std::any &v, const std::vector<std::basic_string<charT>> &s, std::vector<T> * /*unused*/, int /*unused*/)
{
   if(!v.has_value())
   {
      v = std::any(std::vector<T>());
   }
   std::vector<T> *tv = std::any_cast<std::vector<T>>(&v);
   assert(NULL != tv);
   for(unsigned i = 0; i < s.size(); ++i)
   {
      try
      {
         /* We call validate so that if user provided
            a validator for class T, we use it even
            when parsing vector<T>.  */
         std::any                              a;
         std::vector<std::basic_string<charT>> cv;
         cv.push_back(s[i]);
         validate(a, cv, (T *)0, 0);
         tv->push_back(std::any_cast<T>(a));
      }
      catch(const boost::bad_lexical_cast & /*e*/)
      {
         boost::throw_exception(invalid_option_value(s[i]));
      }
   }
}

/** Validates optional arguments. */
template<class T, class charT>
void validate(std::any &v, const std::vector<std::basic_string<charT>> &s, std::optional<T> * /*unused*/, int /*unused*/)
{
   validators::check_first_occurrence(v);
   validators::get_single_string(s);
   std::any a;
   validate(a, s, (T *)0, 0);
   v = std::any(std::optional<T>(std::any_cast<T>(a)));
}

template<class T, class charT>
void typed_value<T, charT>::xparse(std::any &value_store, const std::vector<std::basic_string<charT>> &new_tokens) const
{
   // If no tokens were given, and the option accepts an implicit
   // value, then assign the implicit value as the stored value;
   // otherwise, validate the user-provided token(s).
   if(new_tokens.empty() && m_implicit_value.has_value())
      value_store = m_implicit_value;
   else
      validate(value_store, new_tokens, (T *)0, 0);
}

template<class T>
typed_value<T> *value()
{
   // Explicit qualification is vc6 workaround.
   return argsy::value<T>(0);
}

template<class T>
typed_value<T> *value(T *v)
{
   typed_value<T> *r = new typed_value<T>(v);

   return r;
}

template<class T>
typed_value<T, wchar_t> *wvalue()
{
   return wvalue<T>(0);
}

template<class T>
typed_value<T, wchar_t> *wvalue(T *v)
{
   typed_value<T, wchar_t> *r = new typed_value<T, wchar_t>(v);

   return r;
}

} // namespace argsy
