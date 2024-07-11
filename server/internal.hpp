#pragma once

#include <cstdint>
#include <string_view>

#include <yyjson.h>

#include <kanban_markdown/kanban.hpp>

namespace server
{
    static constexpr inline uint32_t hash(const std::string_view s) noexcept
    {
        uint32_t hash = 5381;

        for (const char *c = s.data(); c < s.data() + s.size(); ++c)
            hash = ((hash << 5) + hash) + (unsigned char)*c;

        return hash;
    }
    
    static std::vector<std::string> split(std::string s, std::string delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

    static inline std::string yyjson_get_string_object(yyjson_val *val)
    {
        const char *val_data = yyjson_get_str(val);
        int val_size = yyjson_get_len(val);
        return std::string(val_data, val_size);
    }

    struct KanbanTuple
    {
        std::string file_path;
        kanban_markdown::KanbanBoard kanban_board;
    };
}
