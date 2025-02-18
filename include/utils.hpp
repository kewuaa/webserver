#pragma once
#include <string>


static std::string url_decode(std::string_view in_url) noexcept {
    int len = in_url.size();
    std::string out_url;
    out_url.reserve(len);

    const char* p = in_url.data();
    for (int i = 0; i < len; ++i) {
        unsigned int tmp = 0;
        char c = in_url[i];
        if (
            (c == '%')
            &&
            (i + 2 < len)
            && (sscanf(p + i + 1, "%2x", &tmp) == 1)
        ) {
            out_url += (char)tmp;
            i += 2;
        } else {
            out_url += c;
        }
    }
    return out_url;
}
