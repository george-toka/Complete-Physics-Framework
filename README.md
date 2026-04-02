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

   ```markdown
4. Update `iniconds.py` to prompt the user for the initial magnetic setup and calculate the NumPy array.
5. *Now* update the C++ IO tool to read the new HDF5 chunk.

### Scenario B: Adding a New Physics Constant (e.g., Speed of Light)
You are adding a relativistic module and need the engine to know `c`.

1. Open the relevant domain schema (e.g., `iniconds_templates/core_fields.json` or a new `relativity_fields.json`).
2. Add the constant to the `"Constants"` block.

```json
"Constants": {
    "SpeedOfLight_C": { "value": "", "type": "float", "shape": "scalar", "help": "Speed of light in chosen units" }
}
```

3. Run `pre_generator.py`. The script will automatically stamp this into the `/Header` attributes of your new HDF5 file.
4. In C++, read it into your global `PhysicsContext` struct during the Zeroth Step.

### Scenario C: Adding a New Algorithmic Setting (e.g., a new Tree Sorting method)
You wrote a faster way to sort the KD-Tree in C++ and need a flag to turn it on, plus a new tolerance variable.

1. **Do NOT** put this in the initial conditions schemas (it is not a physical property of the universe).
2. Add the framework flag to `build_templates/default.json` so CMake knows to compile your new C++ headers.
3. Add the runtime dials (`TREE_SORT_METHOD`, `SORT_TOLERANCE`) to `runtime_params.json`.
4. Run the simulation. The C++ engine reads the new parameters at boot and executes.

---

## 🛑 The Golden Rules of the Codebase

To prevent architecture collapse, any pull request or commit must adhere to these three rules:

> **1. The C++ Engine is Dumb (In a Good Way)**
> The C++ codebase should do zero complex initialization math. It does not know what a "Salpeter Mass Function" or a "Plummer Sphere" is. Python (`iniconds.py`) calculates the complex starting scenarios. C++ simply loads the arrays and executes the Hot Loop.

> **2. Never Hardcode Constants in C++**
> Never write `double G = 6.674e-11;` in the C++ source code. This permanently locks the engine into a single unit system. Read all physical constants from the HDF5 `/Header` at runtime so the engine can simulate an atom or a galaxy without recompiling.

> **3. Protect the "Time Capsule"**
> Derived quantities (Density, Pressure, Gravitational Potential) are calculated by the C++ engine during the "Zeroth Step." They belong in the Snapshot output files for visualization, but they **must not** be required in the `iniconds.h5` initialization file. Keep the input files as lean as physically possible.

---

## 🚀 Pipeline Execution Order

When running a simulation from scratch, the terminal workflow is always:

1. **Compile:** `python build_orchestrator.py --config build_templates/default.json`
2. **Pre-Generate (Carve Memory):** `python pre_generator.py --particles 1000000 --out iniconds.h5`
3. **Paint the Universe:** `python iniconds.py --target iniconds.h5`
4. **Iterate (Optional):** `python tweak_solver.py --theta 0.8 --target iniconds.h5`
5. **Simulate:** `./engine --params runtime_params.json --data iniconds.h5`

