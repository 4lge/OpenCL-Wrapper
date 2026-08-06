/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */

/**
 * @file cl_error_lookup.hpp
 * @author Otto Link (otto.link.bv@gmail.com)
 * @brief
 *
 * @copyright Copyright (c) 2024
 */
#pragma once
#include <string>

namespace clerror
{

struct ErrorInfo
{
  int         code = 1;
  std::string flag = "UNKNOWN CODE";
  std::string description = "UNKNOWN CODE";
  std::string type = "UNKNOWN CODE";
};

ErrorInfo get_error_info(int code);

std::string get_error_str(int code);

std::string get_error_full(int code);

void throw_opencl_error(int code);

} // namespace clerror