#pragma once

#include <vector>
#include "../framework parameters/compile time flags/base_flags.hpp"

// ================================
//     BASIC PARTICLES TEMPLATE
// ================================


// ---------------------------------------------------------
//     Other Frameworks' particles work like layers.
//     They extend the functionalities and attributes
//     of this template struct.
// ---------------------------------------------------------
struct BaseParticles {
    using Real = Framework::Real;
    static constexpr size_t Dims = Framework::Dims;
    static constexpr bool VariableMass = Framework::VariableMass;

    // ------------------------------
    //    Base Particles attributes
    // ------------------------------
    size_t N;
    std::vector<Real> pos[Dims];
    std::vector<Real> vel[Dims];
    std::vector<Real> acc[Dims];
    std::vector<Real> mass;
    Real total_mass;
    Real r_com[Dims];

    // ------------------
    //    Constructors
    // ------------------
       

    // ------------------------------
    //    Base Particles Functions
    // ------------------------------

    // Inline Mass Getter, to avoid branch logic for variable vs constant mass
    Real getMass(size_t i) const {
        if constexpr(VariableMass) return mass[i];
        else return total_mass/Real(N);
    }

    // Center of Mass Setter for variable mass scheme
    void setCOM();
};
