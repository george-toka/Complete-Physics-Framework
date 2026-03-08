#pragma once

#include "hydro_flags.hpp"
#include "gravity_flags.hpp"

namespace Framework {
    static_assert(hasGravity || hasHydro, "INPUT ERROR: No given physics framework was initiated.");
}