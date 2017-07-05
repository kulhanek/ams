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
ADD_EXECUTABLE(ams-repoindex-fdirs ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-fdirs ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-fdirs ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-fdirs
        DESTINATION
            bin
        )

