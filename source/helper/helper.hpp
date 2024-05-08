#pragma once

#include <iostream>
#include <map>
#include <unordered_map>

namespace Helper
{
    template <typename K, typename V>
    bool MapHasKey(const std::map<K, V> &map, const K &key)
    {
        return map.find(key) != map.end();
    }

    template <typename K, typename V>
    bool UnorderedMapHasKey(const std::unordered_map<K, V> &map, const K &key)
    {
        return map.find(key) != map.end();
    }

    template <typename K, typename V>
    bool PairVectorHasKey(const std::vector<std::pair<K, V>> &vec, const K &key)
    {
        for (const auto &pair : vec)
        {
            if (pair.first == key)
                return true;
        }
        return false;
    }

    template <typename K, typename V>
    void PairVectorGet(std::vector<std::pair<K, V>> &vec, const K &key, V **out_value)
    {
        for (std::pair<K, V> &pair : vec)
        {
            if (pair.first == key)
            {
                *out_value = &pair.second;
                return;
            }
        }
    }

    std::vector<std::string> SplitString(std::string str, char separator)
    {
        std::vector<std::string> result = {};
        std::vector<char> buff = {};

        for (char c : str)
        {
            if (c == separator)
            {
                result.push_back(std::string(buff.begin(), buff.end()));
                buff.clear();
                continue;
            }
            buff.push_back(c);
        }

        result.push_back(std::string(buff.begin(), buff.end()));
        return result;
    }

    bool StringContains(std::string str, char character)
    {
        for (char c : str)
        {
            if (c == character)
                return true;
        }
        return false;
    }
}