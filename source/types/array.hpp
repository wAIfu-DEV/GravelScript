#pragma once

#include <iostream>
#include <cstdint>

#include <stdexcept>

template <typename T>
struct Array
{
public:
    size_t size = 0;

private:
    T *data = nullptr;

public:
    constexpr Array()
    {
    }

    Array(const std::initializer_list<T> &list)
    {
        size = list.size();
        if (!size)
            return;
        data = new T[size];
        std::copy(list.begin(), list.end(), data);
    }

    ~Array()
    {
        if (size && data)
            delete[] data;
    }

    T &operator[](const size_t index) const
    {
        return data[index];
    }

    T &at(const size_t index) const
    {
        if (index >= size)
        {
            throw std::runtime_error("Out of bounds access.");
        }
        return data[index];
    }
};
