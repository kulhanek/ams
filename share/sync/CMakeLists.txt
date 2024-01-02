# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

ADD_SUBDIRECTORY(rsync)

INSTALL(FILES
            ams-sync-lib
       DESTINATION
            share/sync
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
