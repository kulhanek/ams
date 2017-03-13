# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        ModuleCmd.cpp
        ModuleCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_ams-module-cmd ${PROG_SRC})
ADD_DEPENDENCIES(_ams-module-cmd ams_shared)
TARGET_LINK_LIBRARIES(_ams-module-cmd ${AMS_LIBS})

INSTALL(TARGETS
            _ams-module-cmd
        DESTINATION
            bin
        )

