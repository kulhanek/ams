# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        AutoCmd.cpp
        AutoCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-autosite ${PROG_SRC})

TARGET_LINK_LIBRARIES(ams-autosite ${AMS_LIBS})

INSTALL(TARGETS
            ams-autosite
        DESTINATION
            bin
        )

