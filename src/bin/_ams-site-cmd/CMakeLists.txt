# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        SiteCmd.cpp
        SiteCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_ams-site-cmd ${PROG_SRC})
ADD_DEPENDENCIES(_ams-site-cmd ams_shared)

TARGET_LINK_LIBRARIES(_ams-site-cmd ${AMS_LIBS})

INSTALL(TARGETS
            _ams-site-cmd
        DESTINATION
            bin
        )

