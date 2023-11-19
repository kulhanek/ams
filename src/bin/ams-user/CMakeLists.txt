# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        UserCmd.cpp
        UserCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-user ${PROG_SRC})
ADD_DEPENDENCIES(ams-user ams_shared)

TARGET_LINK_LIBRARIES(ams-user ${AMS_LIBS})

INSTALL(TARGETS
            ams-user
        DESTINATION
            bin
        )

