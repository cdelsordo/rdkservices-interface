#/*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************/

set(RDKSERVICE_NAME RdkServicesCSinterface)
set(MODULE_NAME ${NAMESPACE}${RDKSERVICE_NAME})

# this will need to be available as a shared library so we can reuse in a dac environment. Controller
add_library(${RDKSERVICE_NAME} SHARED
        RdkSrvController.cpp
        WSClientconnectionmetadata.cpp
        WSClientwebsocketendpoint.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)
target_link_libraries(${RDKSERVICE_NAME} ${PATH_SHARED_LIBRARIES})
#target_include_directories(${MODULE_NAME})
