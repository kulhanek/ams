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
ADD_EXECUTABLE(ams-repoindex-create ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-create ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-create ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-create
        DESTINATION
            bin
        )

