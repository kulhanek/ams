# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            postinst
       DESTINATION
            share/dpkg/personal/DEBIAN
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

INSTALL(FILES
            control.tmp
            PUBLIC
       DESTINATION
            share/dpkg/personal/DEBIAN
        PERMISSIONS
            OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
