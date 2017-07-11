# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexCreate.cpp
        RepoIndexCreateOptions.cpp
        sha1.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-repoindex-files ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-files ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-files ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-files
        DESTINATION
            bin
        )

