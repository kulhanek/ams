# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        ModuleVar.cpp
        ModuleVarOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_ams-module-var ${PROG_SRC})
ADD_DEPENDENCIES(_ams-module-var ams_shared)

TARGET_LINK_LIBRARIES(_ams-module-var ${AMS_LIBS})

INSTALL(TARGETS
            _ams-module-var
        DESTINATION
            bin
        )

