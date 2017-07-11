# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexCreateFDirs.cpp
        RepoIndexCreateFDirsOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-repoindex-create-fdirs ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-create-fdirs ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-create-fdirs ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-create-fdirs
        DESTINATION
            bin
        )

