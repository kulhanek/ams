# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CMD_SRC
        RepoIndexCreateAMS.cpp
        RepoIndexCreateAMSOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-repoindex-create-ams ${CMD_SRC})
ADD_DEPENDENCIES(ams-repoindex-create-ams ams_shared)

TARGET_LINK_LIBRARIES(ams-repoindex-create-ams ${AMS_LIBS})

INSTALL(TARGETS
            ams-repoindex-create-ams
        DESTINATION
            bin
        )

