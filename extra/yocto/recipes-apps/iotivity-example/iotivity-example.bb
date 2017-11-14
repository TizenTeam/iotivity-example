SUMMARY = "IoTivity Example"
DESCRIPTION = "Minimalist Iotivity Client/Server application"
HOMEPAGE = "http://git.s-osg.org/iotivity-example"
SECTION = "apps"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

branch ?= "switch/master"
basesurl ?= "git://git.s-osg.org/iotivity-example;protocol=http"
SRC_URI = "${basesurl};branch=${branch}"
# TODO: Override for local development ie:
# SRC_URI = "git:///home/user/mnt/iotivity-example;nobranch=1;protocol=file"

SRCREV  = "${AUTOREV}"
PV = "0+git${SRCPV}"
S  = "${WORKDIR}/git"

inherit systemd pkgconfig

LOCAL_OPT_DIR = "/opt"
LOCAL_OPT_DIR_D = "${D}${LOCAL_OPT_DIR}"

DEPENDS += " iotivity"
BDEPENDS += " iotivity-dev"

DEPENDS_${PN} += "iotivity-resource-dev iotivity-resource-thin-staticdev iotivity-service-dev iotivity-service-staticdev"

BBCLASSEXTEND = "native nativesdk"
RDEPENDS_${PN} += " iotivity-resource"

SYSTEMD_SERVICE_${PN} = "${PN}.service"
EXTRA_OEMAKE += " name=${PN}"
EXTRA_OEMAKE += " DESTDIR=${D}"
EXTRA_OEMAKE += " unitdir=${base_libdir}/systemd/system/"

do_configure() {
}

do_compile_prepend() {
    export PKG_CONFIG_PATH="${PKG_CONFIG_PATH}"
    export PKG_CONFIG="PKG_CONFIG_SYSROOT_DIR=\"${PKG_CONFIG_SYSROOT_DIR}\" pkg-config"
    export LD_FLAGS="${LD_FLAGS}"
}

do_compile() {
    cd ${S}
    LANG=C
    export LANG
    unset DISPLAY
    LD_AS_NEEDED=1; export LD_AS_NEEDED;
    oe_runmake all
}

do_install() {
    export RPM_BUILD_ROOT=${D}
    cd ${S}
    LANG=C
    export LANG
    unset DISPLAY
    rm -rf ${D}
    oe_runmake install
    oe_runmake rule/systemd/install
}

FILES_${PN} += "${LOCAL_OPT_DIR}/${PN}/*"
FILES_${PN}-dbg += "${LOCAL_OPT_DIR}/${PN}/bin/.debug"
