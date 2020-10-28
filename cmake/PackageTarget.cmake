# ##############################################################################
# Copyright (C) 2020 Intel Corporation
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

set(CPACK_COMPONENTS_ALL all dev runtime license)
set(CPACK_COMPONENTS_GROUPING IGNORE)

include(CPack)
cpack_add_component(license)
cpack_add_component(runtime DEPENDS license)
cpack_add_component(dev DEPENDS runtime)
