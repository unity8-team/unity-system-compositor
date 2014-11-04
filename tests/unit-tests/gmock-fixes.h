/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USC_GMOCK_FIXES_H_
#define USC_GMOCK_FIXES_H_
#include <gmock/gmock.h>

// copied from mir code base
namespace testing
{
namespace internal
{

template<typename T>
class ActionResultHolder<std::unique_ptr<T>>
: public UntypedActionResultHolderBase {
 public:
  explicit ActionResultHolder(std::unique_ptr<T>&& a_value) :
  value_(std::move(a_value)) {}

  // The compiler-generated copy constructor and assignment operator
  // are exactly what we need, so we don't need to define them.

  // Returns the held value and deletes this object.
  std::unique_ptr<T> GetValueAndDelete() const {
      std::unique_ptr<T> retval(std::move(value_));
    delete this;
    return std::move(retval);
  }

  // Prints the held value as an action's result to os.
  virtual void PrintAsActionResult(::std::ostream* os) const {
    *os << "\n          Returns: ";
    // T may be a reference type, so we don't use UniversalPrint().
    UniversalPrinter<std::unique_ptr<T>>::Print(value_, os);
  }

  // Performs the given mock function's default action and returns the
  // result in a new-ed ActionResultHolder.
  template <typename F>
  static ActionResultHolder* PerformDefaultAction(
      const FunctionMockerBase<F>* func_mocker,
      const typename Function<F>::ArgumentTuple& args,
      const string& call_description) {
    return new ActionResultHolder(
        func_mocker->PerformDefaultAction(args, call_description));
  }

  // Performs the given action and returns the result in a new-ed
  // ActionResultHolder.
  template <typename F>
  static ActionResultHolder*
  PerformAction(const Action<F>& action,
                const typename Function<F>::ArgumentTuple& args) {
    return new ActionResultHolder(action.Perform(args));
  }

 private:
  std::unique_ptr<T> mutable value_;

  // T could be a reference type, so = isn't supported.
  GTEST_DISALLOW_ASSIGN_(ActionResultHolder);
};

}

template<typename T>
class DefaultValue<std::unique_ptr<T>> {
 public:
  // Unsets the default value for type T.
  static void Clear() {}

  // Returns true iff the user has set the default value for type T.
  static bool IsSet() { return false; }

  // Returns true if T has a default return value set by the user or there
  // exists a built-in default value.
  static bool Exists() {
    return true;
  }

  // Returns the default value for type T if the user has set one;
  // otherwise returns the built-in default value if there is one;
  // otherwise aborts the process.
  static std::unique_ptr<T> Get() {
    return std::unique_ptr<T>();
  }
};

}

#endif

