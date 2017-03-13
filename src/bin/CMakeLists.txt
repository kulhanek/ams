# ==============================================================================
# AMS CMake File
# ==============================================================================

# general commands -------------------------------
ADD_SUBDIRECTORY(ams-root)
ADD_SUBDIRECTORY(ams-config)
ADD_SUBDIRECTORY(ams-host)
ADD_SUBDIRECTORY(ams-user)
ADD_SUBDIRECTORY(ams-autosite)

# core commands ----------------------------------
ADD_SUBDIRECTORY(_ams-cgen)
ADD_SUBDIRECTORY(_ams-module-var)
ADD_SUBDIRECTORY(_ams-site-cmd)
ADD_SUBDIRECTORY(_ams-module-cmd)
ADD_SUBDIRECTORY(_ams-setenv)

# administrative commands ------------------------
ADD_SUBDIRECTORY(ams-cache)
ADD_SUBDIRECTORY(ams-map-manip)
ADD_SUBDIRECTORY(ams-list-dirs)
ADD_SUBDIRECTORY(ams-repoindex-create)
ADD_SUBDIRECTORY(ams-repoindex-diff)

# actions ----------------------------------------
ADD_SUBDIRECTORY(actions)
