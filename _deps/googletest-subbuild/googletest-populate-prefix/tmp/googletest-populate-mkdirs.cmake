# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-src"
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-build"
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix"
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/nikita/university/fiit/FIIT_FA_SP/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
