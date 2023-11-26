# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        SetEnv.cpp
        SetEnvOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_ams-setenv ${PROG_SRC})
ADD_DEPENDENCIES(_ams-setenv ams_shared)

TARGET_LINK_LIBRARIES(_ams-setenv ${AMS_LIBS})

INSTALL(TARGETS
            _ams-setenv
        DESTINATION
            bin
        )

