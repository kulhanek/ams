# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        BundleCmd.cpp
        BundleCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-bundle ${PROG_SRC})
ADD_DEPENDENCIES(ams-bundle ams_shared)

TARGET_LINK_LIBRARIES(ams-bundle ${AMS_LIBS})

INSTALL(TARGETS
            ams-bundle
        DESTINATION
            bin
        )

