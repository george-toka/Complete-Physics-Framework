#pragma once

#include <vector>  

// ------------------------------
//     Layer of SPH particles 
// ------------------------------
template<typename prevLayer>
struct SPHparticles : public prevLayer {
    // Bring out the template used in Base Particles
    using Real = typename prevLayer::Real;
    static constexpr size_t Dims = prevLayer::Dims;

    // ------------------------------
    //    SPH Particles attributes
    // ------------------------------
    std::vector<Real> rho;
    std::vector<Real> h;
    std::vector<Real> K;

    // ------------------
    //    Constructors
    // ------------------
       

    // ------------------------------
    //    SPH Particles Functions
    // ------------------------------

};