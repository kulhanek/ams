# ==============================================================================
# AMS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(lib/ams SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/base SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/host SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/host/components SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/user SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/site SYSTEM)

# include sub-directories ------------------------------------------------------
IF(AMS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
ELSE(AMS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
    ADD_SUBDIRECTORY(bin)
ENDIF(AMS_CORE_ONLY)

