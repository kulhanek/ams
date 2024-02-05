# ==============================================================================
# Advanced Module System (AMS) CMake File
# ==============================================================================

SET(AMS_PERSONAL_ROOT "${CMAKE_INSTALL_PREFIX}")
CONFIGURE_FILE(../personal/ams-personal.tmp ams-personal @ONLY)

INSTALL(FILES
            ams-sync-bundle
            ams-sync-core
            ams-sync-dpkg
       DESTINATION
            share/scripts
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )       
