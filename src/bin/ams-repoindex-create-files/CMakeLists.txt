# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexCreateFiles.cpp
        RepoIndexCreateFilesOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-repoindex-create-files ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-create-files ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-create-files ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-create-files
        DESTINATION
            bin
        )

