# 🌌 High-Performance Astrophysics Engine Pipeline

Welcome to the core repository for this highly scalable, modular N-Body and Smoothed Particle Hydrodynamics (SPH) engine. 

This engine is built on a strict **Top-Down Data Architecture**. It is designed to completely eliminate the "Re-Run Tax" by decoupling the physical matter (HDF5) from the algorithmic solvers (C++ Runtime), giving you maximum performance and an infinitely reusable data sandbox.

---

## 🏗️ The Three-Pillar Architecture

To maintain absolute modularity, the engine's data pipeline is strictly divided into three pillars. **Before modifying any code, you must understand where your change belongs.**

### Pillar 1: The Forge (Compile-Time)
* **File:** `build_templates/default.json`
* **Purpose:** Hardware setup and framework choices burned into the C++ binary.
* **When to touch it:** If you are changing spatial dimensions (2D vs 3D), enabling GPU/CUDA, or adding a completely new C++ solver framework (e.g., compiling the Multigrid solver).
* *Note: Changing this requires recompiling the engine via CMake.*

### Pillar 2: The Blueprint & The Matter (Initialization)
* **Files:** `iniconds_templates/*.json`, `pre_generator.py`, `iniconds.py`
* **Purpose:** Defines the pure, algorithmic-agnostic state of the universe. Contains initial particle arrays (Masses, Positions) and fundamental physical constants (`G`, `k_B`, `m_H`).
* **When to touch it:** If you are adding a new physical property to the universe (e.g., `MagneticField` arrays, or changing the simulation from G=1.0 to SI units).
* *Note: Changing this requires regenerating your `iniconds.h5` file.*

### Pillar 3: The Sandbox (Runtime Parameters)
* **Files:** `runtime_params.json` & `tweak_solver.py`
* **Purpose:** The dials and knobs for the numerical solver. Dictates *how* the C++ engine runs the math on the initialized matter.
* **When to touch it:** To change the timestep (`dt`), swap integrators (Leapfrog to RK4), tweak Barnes-Hut accuracy (`theta`), or adjust artificial viscosity (`alpha`).
* *Note: Zero recompilation and zero data regeneration required. Tweak and run instantly.*

---

## 🛠️ Developer Guide: How to Extend the Engine

If you need to add or change something in the engine, follow these strict architectural workflows to ensure the pipeline remains unbroken.

### Scenario A: Adding a New Particle Field (e.g., Magnetic Fields)
You want to simulate Magnetohydrodynamics (MHD) and need every particle to have a Magnetic Field vector.

1. **Do NOT** touch the C++ engine first.
2. Create `iniconds_templates/mhd_fields.json`.
3. Add the array definition strictly as a schema:
   ```json
   {
       "Constants": {},
       "Arrays": {
           "MagneticField": { "type": "float", "shape": "vector", "help": "Initial B-field" }
       }
   }
