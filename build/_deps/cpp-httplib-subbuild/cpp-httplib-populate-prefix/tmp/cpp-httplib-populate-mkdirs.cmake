# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-src"
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-build"
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix"
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix/tmp"
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix/src/cpp-httplib-populate-stamp"
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix/src"
  "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix/src/cpp-httplib-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix/src/cpp-httplib-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/nikita/university/fiit/FIIT_FA_SP/build/_deps/cpp-httplib-subbuild/cpp-httplib-populate-prefix/src/cpp-httplib-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
