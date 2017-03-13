# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexDiff.cpp
        RepoIndexDiffOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-repoindex-diff ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-diff ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-diff ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-diff
        DESTINATION
            bin
        )

