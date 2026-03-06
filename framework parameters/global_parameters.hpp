#pragma once
#include <cstdint>
#include <type_traits>

// ================================
//        GLOBAL PARAMETERS
// ================================

#ifndef PRECISION
    #define PRECISION double
#endif

#ifndef DIMENSIONS
    #define DIMENSIONS 3
#endif

#ifndef INDEX
    #define INDEX uint32_t
#endif

#ifndef VARIABLE_MASS
    #define VARIABLE_MASS 1
#endif

#ifndef ENABLE_GRAVITY
    #define ENABLE_GRAVITY 0
#endif

#ifndef ENABLE_HYDRO
    #define ENABLE_HYDRO 0
#endif


namespace Framework {
    // Choose precision type
    using Real = PRECISION;

    // Dimensions for a given simulation
    inline constexpr size_t Dims = DIMENSIONS;

    // Fixed-width integers for indexing 
    using Index = INDEX;

    // Constant vs Variable Mass Scheme 
    inline constexpr bool VariableMass = (VARIABLE_MASS==1);    

    // Flavor Flags
    inline constexpr bool hasGravity = (ENABLE_GRAVITY==1);
    inline constexpr bool hasHydro = (ENABLE_HYDRO==1);

    // Choose Framework for each flavor

    // Check user input
    static_assert(std::is_floating_point_v<Real>, "INPUT ERROR: Precision can only be of floating point type");
    static_assert(Dims >= 1 && Dims <= 3, "INPUT ERROR: Simulations can only run in 1D, 2D or 3D");
    static_assert(std::is_unsigned_v<INDEX>, "INPUT ERROR: INDEX is used for array/vector indexing, so it can only be of unsigned int type");
}