cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(mp_os LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message([[VCPKG toolchain setup example. It is needed for some libraries. Highly recommended to pass it by command line arguments for cmake configuration (IDE can help with this).
        set(VCPKG_TARGET_TRIPLET x64-windows)
        set(CMAKE_TOOLCHAIN_FILE C:/Programming/vcpkg/scripts/buildsystems/vcpkg.cmake)]])

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    cpp-httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(cpp-httplib)

# FetchContent_Declare(
#     crow
#     GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
#     GIT_TAG v0.3+4
#     SOURCE_SUBDIR include
# )
# FetchContent_MakeAvailable(crow)

find_package(Boost COMPONENTS system container date_time REQUIRED) # Ставить через vcpkg

# include_directories(
#     ${crow_SOURCE_DIR}/include
# )

add_subdirectory(allocator)
add_subdirectory(arithmetic)
add_subdirectory(associative_container)
add_subdirectory(common)
add_subdirectory(logger)

# cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# project(mp_os LANGUAGES CXX)

# # 1. Активируем vcpkg
# set(CMAKE_TOOLCHAIN_FILE "/home/nikita/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
# set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")

# set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # 2. Настройка Boost
# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)

# include(FetchContent)

# # 3. Зависимости
# FetchContent_Declare(
#     googletest
#     URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
# )
# FetchContent_MakeAvailable(googletest)

# FetchContent_Declare(
#     nlohmann_json
#     GIT_REPOSITORY https://github.com/nlohmann/json.git
#     GIT_TAG v3.11.3
# )
# FetchContent_MakeAvailable(nlohmann_json)

# FetchContent_Declare(
#     cpp-httplib
#     GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
#     GIT_TAG v0.15.3
# )
# FetchContent_MakeAvailable(cpp-httplib)

# # 4. Поиск Boost через vcpkg
# find_package(Boost REQUIRED COMPONENTS system container date_time)

# message(STATUS "Boost found: ${Boost_FOUND}")
# message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
# message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# # 5. Подключение зависимостей
# add_subdirectory(allocator)
# add_subdirectory(arithmetic)
# add_subdirectory(associative_container)
# add_subdirectory(common)
# add_subdirectory(logger)

# # 6. Для всех подпроектов
# function(configure_target target)
#     target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIRS})
#     target_link_libraries(${target} PRIVATE 
#         Boost::boost
#         Boost::system
#         Boost::container
#         Boost::date_time
#     )
# endfunction()