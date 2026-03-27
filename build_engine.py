import json
import os
import subprocess
import sys
import argparse
from pathlib import Path

# ==========================================
# 1. PRESENTATION LOGIC (The Terminal API)
# ==========================================
class CLI:
    HEADER = '\033[96m'   # Cyan
    SUCCESS = '\033[92m'  # Green
    PROMPT = '\033[93m'   # Yellow
    ERROR = '\033[91m'    # Red
    INFO = '\033[95m'     # Magenta
    BOLD = '\033[1m'
    RESET = '\033[0m'

    @staticmethod
    def print_header(text):
        print(f"\n{CLI.BOLD}{CLI.HEADER}=== {text} ==={CLI.RESET}")

    @staticmethod
    def print_success(text):
        print(f"{CLI.SUCCESS}✔ {text}{CLI.RESET}")

    @staticmethod
    def print_error(text):
        print(f"{CLI.ERROR}❌ {text}{CLI.RESET}")

    @staticmethod
    def print_step(text):
        print(f"{CLI.INFO}{text}{CLI.RESET}")

    @staticmethod
    def print_instruction(text):
        print(f"{CLI.PROMPT}{text}{CLI.RESET}\n")

    @staticmethod
    def ask(flag_name, desc, current_val, options=None):
        # Beautiful wrapper for input() that shows defaults and options
        opt_str = f" {CLI.HEADER}{options}{CLI.RESET}" if options else ""
        prompt_text = f"  ↳ {flag_name} ({desc}){opt_str} [{CLI.SUCCESS}{current_val}{CLI.RESET}]: "
        return input(prompt_text).strip()

# ==========================================
# 2. HELPER FUNCTIONS
# ==========================================

# Recursively find all parameters
def extract_parameters_recursive(data, params_list=None):
    # Initialiser for the dict that the function returns
    if params_list is None:
        params_list = {}
    
    # Recursion breaker
    if not isinstance(data, dict):
        return params_list

    # Recursion main loop
    for key, content in data.items():
        
        # Check for leaf dictionary a.k.a parameters to be set
        if isinstance(content, dict):

            # If the condition below is satisfied, we got a parameter
            if "value" in content:
                params_list[key] = content
            # else, we got a branch
            else:
                extract_parameters_recursive(content, params_list)
    
    return params_list


# Build CLI Arguments
def build_cli(params, parser):
    for key, content in params.items():
        parser.add_argument(f'--{key}', type=str, help=content.get("description",""))


# Override any values that where explicitly given in CLI before running
def apply_overrides(params, args):
    for key, content in params.items():
        preset_input = getattr(args, key, None)

        if preset_input is not None:
            content["value"] = str(preset_input)


# Helper function for interactive wizard
def get_active_value(params):
    val = params.get("value","").strip()
    if val:
        return val
    return params.get("hint", "").strip()


# User friendly terminal-API for engine configuration
def interactive_wizard(params):
    CLI.print_instruction("Review or modify parameters (Press 'Enter' to accept the green default).")
    
    for key, content in params.items():
        # Gather the text we need for the prompt
        current_val = get_active_value(content)
        desc = content.get("description", "No description provided.")
        options = content.get("options", None)
        
        # Persist until you get a valid input
        while True:
            # Ask the user
            user_input = CLI.ask(key, desc, current_val, options)
            
            # User setting the value (if they hit enter, they keep the default/hint value)
            proposed_val = user_input if user_input else current_val

            if options and proposed_val not in options:
                CLI.print_error(f'Invalid choice. Must be one of: {options}')
                continue

            # If we make it here, proposed_val is valid and we move onto the next parameter    
            content["value"] = proposed_val
            break


# ==========================================
# 3. PIPELINED ENGINE CONFIGURATION & SETUP
# ==========================================
def setup():

    # Open file    
    config_file = "build_config.json"
    
    try:
        with open(config_file, 'r') as file:
            data = json.load(file)
    except Exception as e:
        CLI.print_error(f"Failed to load '{config_file}': {e}")
        sys.exit(1)

    # Welcome the user 
    CLI.print_header("WELCOME TO THE INTERACTIVE ENGINE SETUP")

    # Argparser to access Command Line
    parser = argparse.ArgumentParser(description = "Astrophysical Simulations Engine Setup")
    # If -i flag is non-existent it defaults to false
    parser.add_argument('-i','--interactive', action='store_true', help = 'Force interactive wizard')

    # Extract all parameters
    params = extract_parameters_recursive(data)
    
    # Add all parameters as CLI Arguments
    build_cli(params, parser)

    # Read the terminal
    args = parser.parse_args()

    # Override parameters 
    apply_overrides(params, args)   
    
    # Interactive wizard vs Rapid build
    if args.interactive or len(sys.argv) == 1:
        interactive_wizard(params)
    else:
        CLI.print_success("Silent mode engaged. Proceeding with CLI overrides and defaults.")
        # Safety check to ensure correct CLI input
        for key, content in params.items():
            val = get_active_value(content)
            options = content.get("options")
            if options and val not in options:
                CLI.print_error(f"FATAL: Invalid value '{val}' for --{key}. Allowed: {options}")
                sys.exit(1)

    # ---- SET PROJECT NAME & FOLDERS ---- 
    
    # Generate signature (e.g., "2d_gravity_hydro")
    dim = get_active_value(params.get("DIMENSIONS", {}))
    signature = f"{dim}d"
    if get_active_value(params.get("ENABLE_GRAVITY", {})) == "1":
        signature += "_gravity"
    if get_active_value(params.get("ENABLE_HYDRO", {})) == "1":
        signature += "_hydro"

    # Ask the user (Only if human mode is active)
    if args.interactive or len(sys.argv) == 1:
        # Show the signature as the green default
        user_name = CLI.ask("Project Name", "Name of the build folder & executable", signature)
        final_name = user_name if user_name else signature
    else:
        # Machine Mode silently defaults to the signature
        final_name = signature

    # Inject the final name into the JSON memory so CMake's project() command sees it!
    data["PROJECT_NAME"] = final_name    

    build_dir = Path("build") / final_name
    build_dir.mkdir(parents=True, exist_ok=True)
    CLI.print_step(f"Target Directory: {build_dir}")

    # ---- RECEIPT OF THE CONFIGURATION PARAMETERS ----
    # MUST happen before CMake so the compiler can read the choices!
    receipt_path = build_dir / config_file
    
    try:
        with open(receipt_path, 'w') as f:
            # We dump 'data' (the root dict), not 'params', to keep the structure!
            json.dump(data, f, indent=4)
        CLI.print_success(f"Receipt saved to: {receipt_path}")
    except Exception as e:
        CLI.print_error(f"Failed to save receipt: {e}")
        sys.exit(1)

    # ---- CMAKE AND FINISH SETTING UP ----
    exe_name = f"{final_name}.exe" if os.name == 'nt' else final_name
    exe_path = build_dir / exe_name

    if exe_path.exists():
        CLI.print_success(f"Engine already exists at {exe_path}. Skipping compilation.")
    else:
        CLI.print_header("BUILD PHASE")
        
        # Build project (Configure)
        CLI.print_step("Configuring CMake...")
        # Point CMake exactly to our new receipt using -DCONFIG_FILE_PATH
        cmd_config = ["cmake", "-B", str(build_dir), "-S", ".", f"-DCONFIG_FILE_PATH={receipt_path.resolve()}"]
        if subprocess.run(cmd_config).returncode != 0:
            CLI.print_error("CMake configuration failed.")
            sys.exit(1)

        # Compile Project
        build_type = get_active_value(params.get("CMAKE_BUILD_TYPE", {}))
        CLI.print_step(f"Compiling Engine ({build_type})...")
        cmd_build = ["cmake", "--build", str(build_dir), "--config", build_type]
        if subprocess.run(cmd_build).returncode != 0:
            CLI.print_error("Compilation failed.")
            sys.exit(1)
            
        CLI.print_success(f"Build complete! Executable ready at: {exe_path}")

    # ---- PRE-GENERATED FIELDS FOR INITIAL CONDITIONS ----
    # Phase 2 Hook! We will eventually trigger iniconds.py here.
    # e.g., subprocess.run(["python", "iniconds.py", "--target", str(build_dir)])


if __name__ == "__main__":
    setup()