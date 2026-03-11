#include "network.hpp"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void async_recognize(std::vector<Stroke> trace_copy, std::atomic<bool>* flag, std::string language, std::vector<std::string>* out_results, std::mutex* out_mutex) {
    CURL* curl = curl_easy_init();
    if (!curl) { *flag = false; return; }

    json ink = json::array();
    for (const auto& s : trace_copy) {
        if (s.x.size() > 1) ink.push_back({s.x, s.y, json::array()});
    }

    if (ink.empty()) { curl_easy_cleanup(curl); *flag = false; return; }

    json body = {{"requests", {{
        {"writing_guide", {{"writing_area_width", WIDTH}, {"writing_area_height", HEIGHT}}},
        {"ink", ink}, {"language", language}
    }}}};

    std::string req = body.dump();
    std::string res;
    struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com.tw/inputtools/request?ime=handwriting&app=mobilesearch&cs=1&oe=UTF-8");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);

    if (curl_easy_perform(curl) == CURLE_OK) {
        try {
            auto j = json::parse(res);
            if (j.size() > 1 && !j[1][0][1].empty()) {
                std::vector<std::string> new_results;
                for (auto& item : j[1][0][1]) {
                    new_results.push_back(item.get<std::string>());
                }
                
                if (out_mutex && out_results) {
                    std::lock_guard<std::mutex> lock(*out_mutex);
                    *out_results = new_results;
                }
                std::cout << "\r " << new_results[0] << "    " << std::flush;
            }
        } catch (...) {}
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    *flag = false;
}
