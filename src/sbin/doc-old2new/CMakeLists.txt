# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        Old2NewCmd.cpp
        Old2NewCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(doc-old2new ${PROG_SRC})
ADD_DEPENDENCIES(doc-old2new ams_shared)

TARGET_LINK_LIBRARIES(doc-old2new ${AMS_LIBS})

INSTALL(TARGETS
            doc-old2new
        DESTINATION
            bin
        )

