cmake_minimum_required(VERSION 3.23)

project(MyProjectName)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/save-file.cpp
    src/util.cpp
)

# Find Boost packages (header-only multiprecision, miller_rabin, etc.)
find_package(Boost REQUIRED CONFIG)

# Find OpenSSL (required for RSA key generation/saving)
find_package(OpenSSL REQUIRED CONFIG)

add_executable(my_app ${SOURCES})
target_link_libraries(my_app PRIVATE Boost::boost OpenSSL::Crypto)