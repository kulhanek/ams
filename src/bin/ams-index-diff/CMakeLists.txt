# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexDiff.cpp
        RepoIndexDiffOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-index-diff ${CMD_SRC})
ADD_DEPENDENCIES(ams-index-diff ams_shared)

TARGET_LINK_LIBRARIES(ams-index-diff ${AMS_LIBS})

INSTALL(TARGETS
            ams-index-diff
        DESTINATION
            bin
        )

