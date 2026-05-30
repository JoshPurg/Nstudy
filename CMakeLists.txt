cmake_minimum_required(VERSION 3.16)
project(NJUPTLibraryHub VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Drogon must be installed: https://github.com/drogonframework/drogon
find_package(Drogon CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)

set(SOURCES
    main.cpp
    src/database/Database.cpp
    src/controllers/AuthController.cpp
    src/controllers/RoomController.cpp
    src/controllers/PostController.cpp
    src/utils/TokenUtils.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Drogon::Drogon
    SQLite::SQLite3
)

# Copy config to build dir
configure_file(config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)
