# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Packaging
#

set(CPACK_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(VPL_COMPONENT_DEV dev)
set(VPL_COMPONENT_LIB lib)
set(VPL_COMPONENT_TOOLS tools)

set(CPACK_COMPONENTS_ALL all ${VPL_COMPONENT_DEV} ${VPL_COMPONENT_LIB}
                         ${VPL_COMPONENT_TOOLS})
set(CPACK_COMPONENTS_GROUPING IGNORE)

include(CPack)
cpack_add_component(${VPL_COMPONENT_LIB})
cpack_add_component(${VPL_COMPONENT_DEV} DEPENDS ${VPL_COMPONENT_LIB})
cpack_add_component(${VPL_COMPONENT_TOOLS} DEPENDS ${VPL_COMPONENT_LIB})
