﻿#pragma once
#include <wil/cppwinrt.h>
#include <wil/com.h>
#include <wil/result.h>

#define WINRT_NO_MAKE_DETECTION
#include <winrt/base.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>

#define NOMINMAX
#include <Windows.h>
#undef min
#undef max

#include <span>
#include <ranges>
#include <optional>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <utility>
#include <memory>
#include <functional>
#include <iostream>
#include <thread>
#include <future>
#include <atomic>
#include <bit>
#include <mutex>
#include <fstream>
#include <charconv>
#include <algorithm>
#include <concepts>
#include <vector>
#include <bitset>
#include <chrono>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <nlohmann/json.hpp>
