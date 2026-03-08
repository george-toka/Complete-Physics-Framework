#include <type_traits>
#include "../framework parameters/compile time flags/flags.hpp"
#include "base_particles.hpp"
#include "hydro_particles.hpp"
#include "gravity_particles.hpp"

// ================================
//         PARTICLE WRAPPER
// ================================

// ----------------------------------------------------------------------------------
// THE CODE BELOW IS EQUIVALENT TO THIS:
//    using Base = BaseParticles;
//    using Layer1 = std::conditional_t<hasGravity, GRAVITYparticles<Base>, Base>;
//    using Layer2 = std::conditional_t<hasHydro, SPHparticles<Layer1>, Layer1>;
//    using Particles = Layer2;
//    ...
// BUT IMPLEMENTED USING RECURSIVE TEMPLATES
// -----------------------------------------------------------------------------------

namespace ParticleWrapper {
    // ============================================================
    // 1. THE LAYER POLICY
    // Bundles a condition (hasGravity) with a layer (GravityLayer)
    // ============================================================
    template <bool Condition, template <typename> typename Layer>
    struct OptionalLayer {
        template <typename Base>
        using Apply = std::conditional_t<Condition, Layer<Base>, Base>;
    };

    // ============================================================
    // 2. THE ASSEMBLER ENGINE (Primary Template)
    // Expects a Base and a variadic list of OptionalLayers
    // ============================================================
    template <typename Base, typename... Layers>
    struct Assembler;

    // BASE CASE: The pack of Layers is empty. We are done.
    template<typename Base>
    struct Assembler<Base> {
        using Type = Base;
    };

    // RECURSIVE CASE: We have at least one FirstLayer, and maybe RestLayers
    template<typename Base, typename FirstLayer, typename... RestLayers>
    struct Assembler<Base, FirstLayer, RestLayers...> {
        // STEP A: Apply the first layer
        using NewBase = typename FirstLayer::template Apply<Base>;
        // STEP B: Pass the result and the remaining layers back into the loop
        using Type = Assembler<NewBase, RestLayers...>::Type;
    };

    // ============================================================
    // 3. FINAL REGISTRY
    // ============================================================
    using Particles = Assembler<BaseParticles,
                                OptionalLayer<hasGravity, GRAVITYparticles>,
                                OptionalLayer<hasHydro, SPHparticles>
                            >;
}

