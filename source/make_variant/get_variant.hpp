#pragma once

#include <unordered_map>

#include "../logger/logger.hpp"

#include "../types/variant.hpp"
#include "../memory/memory.hpp"

std::string VarGetString(const Variant &v)
{
    return Memory::strings.at(v.d64);
}

VarInt VarGetInt(const Variant &v)
{
    return std::bit_cast<int64_t>(v.d64);
}

VarFloat VarGetFloat(const Variant &v)
{
    return std::bit_cast<double>(v.d64);
}

VarNull VarGetNull()
{
    return 0LL;
}

std::vector<Variant> VarGetArray(const Variant &v)
{
    return Memory::arrays.at(v.d64);
}

std::unordered_map<std::string, Variant> VarGetMap()
{
    Logger::Error("VarGetMap is not defined yet.", {});
    return {};
}