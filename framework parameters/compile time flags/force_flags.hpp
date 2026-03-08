#pragma once

#include "hydro_flags.hpp"
#include "gravity_flags.hpp"

namespace Framework {
    static_assert(hasGravity || hasHydro, "Error: No given physics framework was initiated.");
}