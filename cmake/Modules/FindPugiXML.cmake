# Find the pugixml XML parsing library.
#
# Sets the usual variables expected for find_package scripts:
#
# PUGIXML_INCLUDE_DIRS - header location
# PUGIXML_LIBRARIES - library to link against
# PUGIXML_FOUND - true if pugixml was found.

find_path (PUGIXML_INCLUDE_DIRS
           NAMES pugixml.hpp
           PATHS ${PUGIXML_HOME}/include)
	   find_library (PUGIXML_LIBRARIES
              NAMES pugixml
              PATHS ${PUGIXML_HOME}/lib)

# Support the REQUIRED and QUIET arguments, and set PUGIXML_FOUND if found.
include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (PugiXML DEFAULT_MSG PUGIXML_LIBRARIES
	PUGIXML_INCLUDE_DIRS)

if (PUGIXML_FOUND)
    set (PUGIXML_LIBRARIES ${PUGIXML_LIBRARY})
	message (STATUS "PugiXML include = ${PUGIXML_INCLUDE_DIRS}")
    message (STATUS "PugiXML library = ${PUGIXML_LIBRARIES}")
else ()
    message (STATUS "No PugiXML found")
endif()

mark_as_advanced (PUGIXML_LIBRARIES PUGIXML_INCLUDE_DIRS)
