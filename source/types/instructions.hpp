#pragma once

#include <iostream>

#include "token.hpp"

struct Instruction
{
    Token::TYPE type;
    std::vector<Token::Token> args;
};