# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_OBJ
        config-cli.cpp
        config-cli-opts.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-config ${PROG_OBJ})

TARGET_LINK_LIBRARIES(ams-config ${AMS_LIBS})

INSTALL(TARGETS
            ams-config
        DESTINATION
            bin
        )

