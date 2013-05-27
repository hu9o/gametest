#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <SFML/Graphics.hpp>
#include "rapidjson/document.h"
#include "rapidjson/filestream.h"

#define DEBUG_OUT (std::cout)

namespace js = rapidjson;

typedef unsigned int uint;
typedef sf::Vector2i vec2i;
typedef sf::Vector2f vec2f;
typedef std::string  str;

#endif // COMMON_H_INCLUDED
