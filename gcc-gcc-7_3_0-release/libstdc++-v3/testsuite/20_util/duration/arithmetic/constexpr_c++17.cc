// { dg-options "-std=gnu++17" }
// { dg-do compile { target c++1z } }

// Copyright (C) 2011-2017 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include <chrono>
#include <testsuite_common_types.h>

#ifndef __cpp_lib_chrono
# error "Feature-test macro for constexpr <chrono> missing"
#elif __cpp_lib_chrono != 201611
# error "Feature-test macro for constexpr <chrono> has wrong value"
#endif

constexpr auto test_operators()
{
  std::chrono::nanoseconds d1 { 1 };
  d1++;
  ++d1;
  d1--;
  --d1;

  auto d2(d1);

  d1+=d2;
  d1-=d2;

  d1*=1;
  d1/=1;
  d1%=1;
  d1%=d2;

  return d1;
}

constexpr auto d4 = test_operators();

