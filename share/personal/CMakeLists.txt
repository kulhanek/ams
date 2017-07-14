# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-personal
       DESTINATION
            share/personal
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

