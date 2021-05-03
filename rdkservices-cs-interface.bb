#*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************
SUMMARY = "CommScope Rdk Services Interface for embedded and DAC environments"
DESCRIPTION = "CommScope Rdk Services Interface for embedded and DAC environments"
LICENSE = "CLOSED"
DEPENDS += "cjson websocketpp boost"
RDEPENDS_${PN} += " cjson boost "
#RDEPENDS_${PN}-dev += "${PN}-staticdev"

inherit cmake

# determine if we will use CommScope videogit server could also use github for now will use local files
# directory.  If video git is used where we build it could differ for now use separate directory
#S = "${WORKDIR}/RDK_Arris_private_common/RDKServices"
S = "${WORKDIR}/RDKServices"
SB = "${WORKDIR}/build"

EXTRA_OECMAKE += "-DDAC=false"
EXTRA_OECMAKE += "-DRDKSERVICES_INCLUDE_DIR=${PKG_CONFIG_SYSROOT_DIR}/usr/include"
EXTRA_OECMAKE += "-DPATH_SHARED_LIBRARIES=${PKG_CONFIG_SYSROOT_DIR}/usr/lib"

LDFLAGS += "-L$(PKG_CONFIG_SYSROOT_DIR)/usr/lib -lboost_system -lboost_random -lboost_thread -D_WEBSOCKETPP_CPP11_STL_ -lcjson"
#-lboost_system -lboost_thread -lboost_filesystem -lboost_date_time -lboost_regex

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
# for now everthing is in Controller later expand directory structure
FILESEXTRAPATHS_prepend := "${THISDIR}/files/Controller:"

# for now use local files directory
SRC_URI += "file://CMakeLists.txt"
SRC_URI += "file://RdkSrvController.cpp"
SRC_URI += "file://RdkSrvController.h"
SRC_URI += "file://WSClientconnectionmetadata.cpp"
SRC_URI += "file://WSClientconnectionmetadata.h"
SRC_URI += "file://WSClientwebsocketendpoint.cpp"
SRC_URI += "file://WSClientwebsocketendpoint.h"

# for now since we are not using videogit server
addtask do_unpack_additional after do_unpack before do_patch
do_unpack_additional() {
mkdir -p ${S}
cp ${WORKDIR}/CMakeLists.txt ${S}/CMakeLists.txt
cp ${WORKDIR}/RdkSrvController.cpp ${S}/RdkSrvController.cpp
cp ${WORKDIR}/RdkSrvController.h ${S}/RdkSrvController.h
cp ${WORKDIR}/WSClientconnectionmetadata.cpp ${S}/WSClientconnectionmetadata.cpp
cp ${WORKDIR}/WSClientconnectionmetadata.h ${S}/WSClientconnectionmetadata.h
cp ${WORKDIR}/WSClientwebsocketendpoint.cpp ${S}/WSClientwebsocketendpoint.cpp
cp ${WORKDIR}/WSClientwebsocketendpoint.h ${S}/WSClientwebsocketendpoint.h
}

do_install() {
	install -m 0755 ${S}/RdkSrvController.h ${PKG_CONFIG_SYSROOT_DIR}/usr/include/RdkSrvController.h
	install -m 0755 ${S}/WSClientwebsocketendpoint.h ${PKG_CONFIG_SYSROOT_DIR}/usr/include/WSClientwebsocketendpoint.h
	install -m 0755 ${S}/WSClientconnectionmetadata.h ${PKG_CONFIG_SYSROOT_DIR}/usr/include/WSClientconnectionmetadata.h
    install -d ${D}${libdir}
    install -m 0755 ${SB}/libRdkServicesCSinterface.so ${D}${libdir}/
}

FILES_${PN} += "${libdir}/RdkServicesCSinterface.so"
FILES_${PN} += "${incdir}/*"

FILES_SOLIBSDEV = ""
SOLIBS = ".so"
INSANE_SKIP_${PN} += "dev-so"
#RDEPENDS_${PN}-dev_pn-websocketpp = ""

