# this file is adapted from https://github.com/cschreib/phypp/blob/master/cmake/FindCFITSIO.cmake

find_path (CFITSIO_INCLUDES fitsio.h fitsio2.h
	HINTS ${CFITSIO_ROOT_DIR}
	PATHS /sw /usr /usr/local /opt/local
	PATH_SUFFIXES "include" "include/fitsio" "include/cfitsio"
)

find_library (CFITSIO_LIBRARY cfitsio
    HINTS ${CFITSIO_ROOT_DIR}
    PATHS /sw /usr /usr/local /opt/local
    PATH_SUFFIXES "lib"
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (cfitsio
    DEFAULT_MESG
    CFITSIO_INCLUDES
    CFITSIO_LIBRARY
)
