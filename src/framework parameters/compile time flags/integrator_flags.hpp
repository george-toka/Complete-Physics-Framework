#pragma once

#define LEAPFROG    1
#define RK2         2
#define RK4         3

#ifndef INTEGRATOR
    #define INTEGRATOR RK2
#endif

#ifndef ADAPTIVE_TIMESTEP
    #define ADAPTIVE_TIMESTEP 0
#endif

#ifndef INDIVIDUAL_TIMESTEP
    #define INDIVIDUAL_TIMESTEP 0
#endif

namespace Framework {
    // Choose which integrator shall be used in the simulation
    inline constexpr unsigned int Integrator = INTEGRATOR;

    // Determine whether adaptive timestep is implemented or not
    inline constexpr bool itsAdaptive = (ADAPTIVE_TIMESTEP == 1);
    
    // Determine whether individual timestepping is implemented or not
    inline constexpr bool itsIPTS = (INDIVIDUAL_TIMESTEP == 1);

    // Check user input
    static_assert(!(itsIPTS && !itsAdaptive), 
        "INPUT ERROR: Individual timestepping requires adaptive timestepping to be enabled");
    static_assert(Integrator == LEAPFROG || Integrator == RK2 || Integrator == RK4, 
        "INPUT ERROR: Invalid Integrator choice. Available options are: LEAPFROG, RK2, RK4");
}
