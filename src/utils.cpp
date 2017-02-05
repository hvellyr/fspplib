// Copyright (c) 2016 Gregor Klinke

#include "fspp/utils.hpp"

#include "fspp/estd/memory.hpp"
#include "fspp/filesystem.hpp"

#if defined(FSPP_IS_WIN)
#include <Windows.h>
#elif defined(FSPP_IS_MAC) || defined(FSPP_IS_UNIX)
#include <unistd.h>
#endif

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>


namespace eyestep {
namespace fs = filesystem;

namespace filesystem {

namespace {
int
current_process_id()
{
#if defined(FSPP_IS_WIN)
  static int pid = static_cast<int>(::GetCurrentProcessId());
  return pid;
#elif defined(FSPP_IS_MAC) || defined(FSPP_IS_UNIX)
  return getpid();
#else
#error unknown os
#endif
}


std::string
current_process_id_as_string()
{
  return std::to_string(current_process_id());
}


std::string
random_string(size_t n)
{
  using std::begin;

  static const auto letters = std::string("abcdefghijklmnopqrstuvwxzy0123456789");

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, static_cast<int>(letters.size()) - 1);

  std::string result(n, 0);
  std::generate_n(
    begin(result), n, [&]() { return letters[static_cast<size_t>(dis(gen))]; });

  return result;
}


std::string
now()
{
  std::time_t now =
    std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream buf;
  char tmp[100];
  std::strftime(tmp, sizeof(tmp), "%Y%m%dT%H%M%S", std::localtime(&now));
  return tmp;
}

}  // namespace


fs::path
create_temp_dir(const path& temp_p, const std::string& prefix)
{
  fs::create_directories(temp_p);

  fs::path tmp_dir;
  do {
    std::stringstream random_name;
    random_name << prefix
                << "-" << current_process_id_as_string() << "-" << now() << "-"
                << random_string(10);

    tmp_dir = temp_p / random_name.str();
  } while (!fs::create_directory(tmp_dir));

  return tmp_dir;
}


void
write_to_file(const path& p, const std::string& data)
{
  with_stream_for_writing(
    p, [&](std::ostream& os) { os << data; }, std::ios::trunc | std::ios::binary);
}

}  // namespace filesystem
}  // namespace eyestep
