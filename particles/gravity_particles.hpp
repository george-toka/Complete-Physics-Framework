#pragma once

#include <vector>
// bale edw ta upoloipa headers


// =========================================
//     GRAVITY FRAMEWORK USING MULTIGRID
// =========================================


// ----------------------------------
//     Layer of gravity particles 
// ----------------------------------
template<typename prevLayer>
struct GRAVITYparticles : public prevLayer {
    using Real = typename prevLayer::Real;
    static constexpr size_t Dims = prevLayer::Dims;

    // Gravity attributes
    std::vector<Real> phi_potential;

    // TODO STUFF
};