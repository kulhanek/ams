# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(CACHECMD_SRC
        CacheCmd.cpp
        CacheCmdOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-cache ${CACHECMD_SRC})
ADD_DEPENDENCIES(ams-cache ams_shared)

TARGET_LINK_LIBRARIES(ams-cache ${AMS_LIBS})

INSTALL(TARGETS
            ams-cache
        DESTINATION
            bin
        )

