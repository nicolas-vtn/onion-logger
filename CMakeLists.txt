cmake_minimum_required(VERSION 3.20)

project(onion_logger LANGUAGES CXX)


# ---- Dependency : onion_datetime ----
if (NOT TARGET onion::datetime)
    message(FATAL_ERROR "onion::datetime is required for onion::logger")
endif()

# ---- Library ----
add_library(onion_logger
    "onion/Logger.cpp"
)
add_library(onion::logger ALIAS onion_logger)

target_include_directories(onion_logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(onion_logger PUBLIC cxx_std_20)

set_target_properties(onion_logger PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# ---- Link dependency ----
target_link_libraries(onion_logger
    PUBLIC
        onion::datetime
)

# ---- Demo ----
option(ONION_BUILD_DEMO "Build Logger demo" OFF)

if (ONION_BUILD_DEMO)
    add_subdirectory(demo)
endif()
