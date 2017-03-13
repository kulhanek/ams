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

TARGET_LINK_LIBRARIES(_ams-cgen ${AMS_LIBS})

INSTALL(TARGETS
            _ams-cgen
        DESTINATION
            bin
        )

