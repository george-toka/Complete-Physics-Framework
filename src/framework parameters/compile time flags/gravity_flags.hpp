#pragma once

#ifndef ENABLE_GRAVITY
    #define ENABLE_GRAVITY 1
#endif

namespace Framework {
    // Inclusion or not for gravity
    inline constexpr bool hasGravity = (ENABLE_GRAVITY == 1);
    
    // Check for specific gravity solver
    
}