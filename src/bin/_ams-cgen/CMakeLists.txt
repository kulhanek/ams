# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        Cgen.cpp
        CgenOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_ams-cgen ${PROG_SRC})
ADD_DEPENDENCIES(_ams-cgen ams_shared)
TARGET_LINK_LIBRARIES(_ams-cgen ${AMS_LIBS})

INSTALL(TARGETS
            _ams-cgen
        DESTINATION
            bin
        )

