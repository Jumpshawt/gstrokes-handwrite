#pragma once

#include <vector>
#include <atomic>
#include <string>
#include <mutex>
#include "types.hpp"

void async_recognize(std::vector<Stroke> trace_copy, std::atomic<bool>* flag, std::string language, std::vector<std::string>* out_results, std::mutex* out_mutex);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
