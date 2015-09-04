project (Flake C CXX)
cmake_minimum_required (VERSION 2.8.0)

# Setting this explicitly avoids warnings from versions of CMake >= 3
# regarding policy CMP0042; it should be ignored versions of CMake which don't
# support it. (Setting the policy explicitly will break with old CMakes.)
set (CMAKE_MACOSX_RPATH TRUE)

set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set (CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set (CMAKE_CXX_FLAGS_RELEASE "-m64 -O3 -ffast-math -fno-finite-math-only -flto -march=native -funroll-loops")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "release")
endif (NOT CMAKE_BUILD_TYPE)

# Initialize the dependencies to an empty list.
set (FLAKE_DEPS)

# ==========================================================================
#                                                                     DNest3
# ==========================================================================

find_package (DNest3 REQUIRED)
list (APPEND FLAKE_DEPS ${DNEST3_LIBRARY})
include_directories (SYSTEM ${DNEST3_INCLUDES})

# ==========================================================================
#                                                                        GSL
# ==========================================================================

find_package (GSL REQUIRED)
list (APPEND FLAKE_DEPS ${GSL_LIBRARIES})
include_directories (SYSTEM ${GSL_INCLUDES})

# ==========================================================================
#                                                                   RJObject
# ==========================================================================

find_package (RJObject REQUIRED)
list (APPEND FLAKE_DEPS ${RJOBJECT_LIBRARY})
include_directories (SYSTEM ${RJOBJECT_INCLUDES})

# ==========================================================================
#                                                                      BOOST
# ==========================================================================

find_package (Boost COMPONENTS thread system REQUIRED)
list (APPEND FLAKE_DEPS ${Boost_LIBRARIES})
include_directories (SYSTEM ${Boost_INCLUDES})

# ==========================================================================
#                                                                    CFITSIO
# ==========================================================================

find_package (CFITSIO REQUIRED)
list (APPEND FLAKE_DEPS ${CFITSIO_LIBRARY})
include_directories (SYSTEM ${CFITSIO_INCLUDES})

# ==========================================================================
#                                                                     CCFITS
# ==========================================================================

find_package (CCFITS REQUIRED)
list (APPEND FLAKE_DEPS ${CCFITS_LIBRARY})
include_directories (SYSTEM ${CCFITS_INCLUDES})

# ==========================================================================
#                                                                    SRC DIR
# ==========================================================================
add_subdirectory (src)
