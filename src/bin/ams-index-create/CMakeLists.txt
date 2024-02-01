# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexCreateFiles.cpp
        RepoIndexCreateFilesOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-index-create ${CMD_SRC})
ADD_DEPENDENCIES(ams-index-create ams_shared)

TARGET_LINK_LIBRARIES(ams-index-create ${AMS_LIBS})

INSTALL(TARGETS
            ams-index-create
        DESTINATION
            bin
        )

