# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(STAT_SRC
        StatClient.cpp
        StatClientOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(module-add-stat ${STAT_SRC})
ADD_DEPENDENCIES(module-add-stat ams_shared)

TARGET_LINK_LIBRARIES(module-add-stat ${AMS_LIBS})

INSTALL(TARGETS
            module-add-stat
        DESTINATION
            bin/actions
        )

