find_path (DNEST4_INCLUDES
    NAMES "Sampler.h"
    PATHS "/home/matthew/repositories/DNest4/code"
    PATH_SUFFIXES "/include" "include/dnest4"
    HINTS ${DNEST4_ROOT_DIR}
)

find_library (DNEST4_LIBRARY
    NAMES dnest4
    PATHS "/home/matthew/repositories/DNest4/code"
    PATH_SUFFIXES "/lib"
    HINTS ${DNEST4_ROOT_DIR}
)

find_path (RJOBJECT_INCLUDES
    NAMES "Distributions.h"
    PATHS "/home/matthew/repositories/DNest4/code"
    PATH_SUFFIXES "/include" "include/rjobject"
    HINTS ${DNEST4_ROOT_DIR}
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (DNest4
    DEFAULT_MESG
    DNEST4_INCLUDES
    DNEST4_LIBRARY
    RJOBJECT_INCLUDES
)
