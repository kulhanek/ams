# ==============================================================================
# AMS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(MAPMANIP_SRC
        MapManip.cpp
        MapManipOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(ams-map-manip ${MAPMANIP_SRC})
ADD_DEPENDENCIES(ams-map-manip ams_shared)

TARGET_LINK_LIBRARIES(ams-map-manip ${AMS_LIBS})

INSTALL(TARGETS
            ams-map-manip
        DESTINATION
            bin
        )

