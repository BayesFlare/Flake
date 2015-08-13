# this file is adapted from that in https://github.com/cschreib/phypp/blob/master/cmake/FindCCFITS.cmake

find_path(CCFITS_INCLUDES CCfits.h
	HINTS ${CCFITS_ROOT_DIR}
        PATHS /sw /usr /usr/local /opt/local
	PATH_SUFFIXES "/include" "include/CCfits"
        HINTS ${CCFITS_ROOT_DIR}
)

find_library(CCFITS_LIBRARY CCfits
	HINTS ${CCFITS_ROOT_DIR}
 	PATHS /sw /usr /usr/local /opt/local
	PATH_SUFFIXES "lib" "lib/x86_64-linux-gnu"
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CCFITS
	DEFAULT_MSG
	CCFITS_LIBRARY
	CCFITS_INCLUDES
)
