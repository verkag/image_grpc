find_package(OpenCV REQUIRED) 

set(SOURCES main.cpp image_process_impl.cpp processor.cpp)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(server PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(server PRIVATE protos) 
target_link_libraries(server PRIVATE ${OpenCV_LIBS})

