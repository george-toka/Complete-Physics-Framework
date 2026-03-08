#pragma once

#ifndef ENABLE_HYDRO
    #define ENABLE_HYDRO 1
#endif

namespace Framework {
    // Inclusion or not for hydrodynamics
    inline constexpr bool hasHydro = (ENABLE_HYDRO == 1);
    
    // Check for specific hydrodynamics solver

}