/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <stdexcept>
#include "get_error_info.hpp"

namespace clerror
{

inline std::string get_error_str(int code)
{
  ErrorInfo error_info = get_error_info(code);

  return error_info.flag + " [" + std::to_string(code) + "]";
}

inline std::string get_error_full(int code)
{
  ErrorInfo error_info = get_error_info(code);
  return error_info.flag + " [" + std::to_string(code) +
         "]: " + error_info.description + " " + error_info.type + ".";
}

inline void throw_opencl_error(int code)
{
  if (code != 0)
  {
    ErrorInfo error_info = get_error_info(code);
    throw std::runtime_error("OpenCL Error " + get_error_full(code));
  }
}

} // namespace clerror
