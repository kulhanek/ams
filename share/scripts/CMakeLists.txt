# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-sync
            ams-personal
       DESTINATION
            share/scripts
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )       
