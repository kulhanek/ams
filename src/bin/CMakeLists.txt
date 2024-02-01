# ==============================================================================
# AMS CMake File
# ==============================================================================

# general commands -------------------------------
ADD_SUBDIRECTORY(ams-config)
ADD_SUBDIRECTORY(ams-host)
ADD_SUBDIRECTORY(ams-user)

# core commands ----------------------------------
ADD_SUBDIRECTORY(_ams-cgen)
ADD_SUBDIRECTORY(_ams-module-var)
ADD_SUBDIRECTORY(_ams-site-cmd)
ADD_SUBDIRECTORY(_ams-module-cmd)
ADD_SUBDIRECTORY(_ams-setenv)

# administrative commands ------------------------
ADD_SUBDIRECTORY(ams-bundle)
ADD_SUBDIRECTORY(ams-index-create)
ADD_SUBDIRECTORY(ams-index-diff)


