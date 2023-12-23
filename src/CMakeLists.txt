# ==============================================================================
# AMS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(lib/ams SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/base SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/host SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/host/components SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/user SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/site SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/mods SYSTEM)

# include sub-directories ------------------------------------------------------
IF(AMS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
ELSE(AMS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
    ADD_SUBDIRECTORY(bin)
    ADD_SUBDIRECTORY(sbin)
ENDIF(AMS_CORE_ONLY)

