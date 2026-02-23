add_executable(onion_logger_demo
    "logger_demo.cpp"
)

target_link_libraries(onion_logger_demo
    PRIVATE
        onion::logger
)

target_compile_features(onion_logger_demo PRIVATE cxx_std_20)

set_target_properties(onion_logger_demo PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
