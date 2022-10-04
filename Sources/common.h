#pragma once

// Croissant namesapce
#define _CD_NAMESPACE_BEGIN     namespace cd {
#define _CD_NAMESPACE_END       }

// headers
#include "glm/glm.hpp"
#include <iostream>
#include <memory>   // shared_ptr
#include <vector>

// constants
const float     _INFINITY = std::numeric_limits<float>::infinity();
const float     _EPSILON = 1e-8;
