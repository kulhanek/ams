# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-rsync-bundle
            ams-rsync-core
            ams-rsync-dpkg
       DESTINATION
            share/sync/rsync/push
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

