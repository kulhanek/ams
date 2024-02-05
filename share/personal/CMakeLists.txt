# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

INSTALL(FILES
            ams-personal-install
            ams-personal-envinit
            ams-personal-gensshkeys
            ams-personal-remove
            ams-personal-showsshkey
            ams-personal-useradd
            ams-personal
       DESTINATION
            share/personal
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

