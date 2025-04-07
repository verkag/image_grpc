include(FetchContent)
find_package(OpenCV REQUIRED) 

# Remove FetchContent for pqxx and use find_package instead
find_package(pqxx REQUIRED)

find_package(absl REQUIRED) 

set(SOURCES main.cpp impl.cpp)

add_executable(client ${SOURCES})

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${OpenCV_INCLUDE_DIRS})

# Remove pqxx-specific compile options since we're not building it
target_link_libraries(client PRIVATE pqxx::pqxx)
target_link_libraries(client PRIVATE protos) 
target_link_libraries(client PRIVATE ${OpenCV_LIBS})
target_link_libraries(client PRIVATE 
    absl::flags
    absl::flags_parse
    absl::strings
    absl::strings_internal
    absl::base
    absl::config
    absl::core_headers
    absl::log_severity
    absl::raw_logging_internal
    absl::spinlock_wait
)
