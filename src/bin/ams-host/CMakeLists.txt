# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        HostCmd.cpp
        HostCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-host ${PROG_SRC})
ADD_DEPENDENCIES(ams-host ams_shared)

TARGET_LINK_LIBRARIES(ams-host ${AMS_LIBS})

INSTALL(TARGETS
            ams-host
        DESTINATION
            bin
        )

