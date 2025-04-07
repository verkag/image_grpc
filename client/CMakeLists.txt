include(FetchContent)
find_package(OpenCV REQUIRED) 

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PQ REQUIRED libpq)

find_package(absl REQUIRED) 

set(SOURCES main.cpp impl.cpp)

add_executable(client ${SOURCES})

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(client PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(client PRIVATE ${PQXX_INCLUDE_DIRS})

target_link_libraries(client PRIVATE ${PQXX_LIBRARIES})
target_link_libraries(client PRIVATE ${PQ_LIBRARIES})
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
