find_path (RJOBJECT_INCLUDES
    NAMES "Distribution.h"
    PATH_SUFFIXES "/include" "include/rjobject"
    HINTS ${RJOBJECT_ROOT_DIR}
)

find_library (RJOBJECT_LIBRARY
    NAMES rjobject
    PATH_SUFFIXES "/lib"
    HINTS ${RJOBJECT_ROOT_DIR}
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (RJObject
    DEFAULT_MESG
    RJOBJECT_INCLUDES
    RJOBJECT_LIBRARY
)
