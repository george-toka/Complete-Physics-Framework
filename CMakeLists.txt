cmake_minimum_required(VERSION 3.20)

# -------- INITIAL STEPS --------

# Allow an external tool (like Python) to specify the config path
if(NOT CONFIG_FILE_PATH)
    # If no path is provided, default to the one in the root folder
    set(CONFIG_FILE_PATH "${CMAKE_SOURCE_DIR}/build_config.json")
endif()

message(STATUS "Reading configuration from: ${CONFIG_FILE_PATH}")

# Read the file dynamically
file(READ "${CONFIG_FILE_PATH}" CONFIG_DATA)

# Set the project name 
string(JSON PARSED_NAME GET ${CONFIG_DATA} "PROJECT_NAME")
project(${PARSED_NAME} CXX)

# -------- SET TARGET & COMPILER OPTIONS --------

# Grab the human's value first
string(JSON PARSED_BUILD_TYPE ERROR_VARIABLE BT_ERR GET ${CONFIG_DATA} "CMAKE_SETUP" "CMAKE_BUILD_TYPE" "value")

# If the human didn't set a value, fall back to the hint
if("${PARSED_BUILD_TYPE}" STREQUAL "" OR BT_ERR)
    string(JSON PARSED_BUILD_TYPE GET ${CONFIG_DATA} "CMAKE_SETUP" "CMAKE_BUILD_TYPE" "hint")
endif()

# Set it globally
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${PARSED_BUILD_TYPE} CACHE STRING "Build type from JSON" FORCE)
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Enforce ISO C++20 globally for pure modern C++ expressions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Recursive search for source files - CONFIGURE_DEPENDS tracks them for changes
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS src/*.cpp)

# Create the engine
add_executable(${PROJECT_NAME} ${SRCS})

# Include directory for headers
target_include_directories(${PROJECT_NAME} PRIVATE src/) 

# Set compiler options based on configuration file

# Standard compilation Flags - target_compile_options keeps every flag's scope to its target
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wpedantic
    -Wextra
    -march=native
)
# Enforce -03 if build type is "Release"
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

# Check for any other specific flags pulled by the configuration file
string(JSON USER_CXX_FLAGS ERROR_VARIABLE NO_CXX_FLAGS_ERR GET ${CONFIG_DATA} "CMAKE_SETUP" "EXTRA_CXX_FLAGS" "value")
string(STRIP "${USER_CXX_FLAGS}" USER_CXX_FLAGS)

if(NOT ("${USER_CXX_FLAGS}" STREQUAL "" OR NO_CXX_FLAGS_ERR))
    separate_arguments(PARSED_FLAGS NATIVE_COMMAND "${USER_CXX_FLAGS}")
    target_compile_options(${PROJECT_NAME} PRIVATE ${PARSED_FLAGS})
    message(STATUS "Injected Raw Compiler Flags: ${USER_CXX_FLAGS}")
endif()

# -------- PARSE ENGINE-SPECIFIC FLAGS --------

string(JSON PARAM_COUNT LENGTH ${CONFIG_DATA} "COMPILE_PARAMETERS")
math(EXPR LAST_PARAM_INDEX "${PARAM_COUNT} - 1")

# Inject all parameters from the JSON file as -D flags (C++ macros)
foreach(PARAM_INDEX RANGE ${LAST_PARAM_INDEX})
    # Get the Category Name (e.g., CORE_FLAGS, GRAVITY_FLAGS)
    string(JSON CATEGORY_NAME MEMBER ${CONFIG_DATA} "COMPILE_PARAMETERS" ${PARAM_INDEX})

    # Count how many flags are inside this specific category
    string(JSON PER_CATEGORY_COUNT LENGTH ${CONFIG_DATA} "COMPILE_PARAMETERS" "${CATEGORY_NAME}")
    math(EXPR LAST_CATEGORY_INDEX "${PER_CATEGORY_COUNT} - 1")

    # Inner Loop: Find all parameters per category
    foreach(FLAG_INDEX RANGE ${LAST_CATEGORY_INDEX})
        # Get the Flag Name (e.g., DIMENSIONS, ENABLE_GRAVITY)
        string(JSON FLAG_NAME MEMBER ${CONFIG_DATA} "COMPILE_PARAMETERS" "${CATEGORY_NAME}" ${FLAG_INDEX})
        
        # Safely pull the value field 
        string(JSON FLAG_VALUE ERROR_VARIABLE VALUE_FIELD_UNSET_ERR GET ${CONFIG_DATA} "COMPILE_PARAMETERS" "${CATEGORY_NAME}" "${FLAG_NAME}" "value")
        
        # Strip leading or trailing spaces (CORRECT ORDER: "Input" Output)
        string(STRIP "${FLAG_VALUE}" FLAG_VALUE) 

        # Check if value field is empty OR if it doesn't exist
        if("${FLAG_VALUE}" STREQUAL "" OR VALUE_FIELD_UNSET_ERR)      
            # Inject default value (fallback to hint)
            string(JSON FLAG_VALUE GET ${CONFIG_DATA} "COMPILE_PARAMETERS" "${CATEGORY_NAME}" "${FLAG_NAME}" "hint")
            string(STRIP "${FLAG_VALUE}" FLAG_VALUE) 
            message(STATUS "Macro was not set. Falling back to default: -D${FLAG_NAME}=${FLAG_VALUE}")
        else()
            # Print to terminal
            message(STATUS "Loaded Macro: -D${FLAG_NAME}=${FLAG_VALUE}")
        endif()

        # Inject macro into the C++ compiler 
        target_compile_definitions(${PROJECT_NAME} PRIVATE ${FLAG_NAME}=${FLAG_VALUE})
        
        # Unset variables for the next iteration
        unset(FLAG_VALUE)
        unset(VALUE_FIELD_UNSET_ERR)
    endforeach()
endforeach()
