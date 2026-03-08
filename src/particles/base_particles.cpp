#include "particles.hpp"

// Center of Mass Setter for variable mass scheme
void BaseParticles::setCOM() {
    std::fill(r_com, r_com + Dims, 0);
    if constexpr(VariableMass) {
        for (size_t i = 0; i < N; i++) {
            Real m_i = mass[i];
            for (size_t d = 0; d < Dims; d++) {
                r_com[d] += m_i * pos[d][i];
            }
        }

        Real inv_mass = Real(1) / total_mass;
        for (size_t d = 0; d < Dims; d++) r_com[d] *= inv_mass;
    }

    else {
        for(size_t i = 0; i < N; i++) {
            for (size_t d = 0; d < Dims; d++) {
                r_com[d] += pos[d][i];
            }
        }

        Real inv_N = Real(1) / Real(N);
        for (size_t d = 0; d < Dims; d++) r_com[d] *= inv_N;
    }
}

