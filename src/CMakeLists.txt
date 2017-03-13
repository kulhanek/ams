# ==============================================================================
# AMS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(lib/plusula SYSTEM)
INCLUDE_DIRECTORIES(lib/ams SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/system SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/soft SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/torque SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/cuda SYSTEM)
INCLUDE_DIRECTORIES(lib/ams/srv SYSTEM)
INCLUDE_DIRECTORIES(lib/torque SYSTEM)

# include subdirectories -------------------------------------------------------
IF(AMS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
ELSE(AMS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
    ADD_SUBDIRECTORY(bin)
ENDIF(AMS_CORE_ONLY)

