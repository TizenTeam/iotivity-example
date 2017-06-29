# //******************************************************************
# //
# // Copyright 2016 Samsung Electronics France SAS All Rights Reserved.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# //
# // Licensed under the Apache License, Version 2.0 (the "License");
# // you may not use this file except in compliance with the License.
# // You may obtain a copy of the License at
# //
# //      http://www.apache.org/licenses/LICENSE-2.0
# //
# // Unless required by applicable law or agreed to in writing, software
# // distributed under the License is distributed on an "AS IS" BASIS,
# // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# // See the License for the specific language governing permissions and
# // limitations under the License.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

git?=git
export git
git_clone?=${git} clone --depth=1

# Override here if needed
git_clone=${git} clone
#iotivity_version?=master
#iotivity_version=1.1-rel
#iotivity_version=1.2-rel
#iotivity_src?=${HOME}/usr/local/src/

# Supported configuration
iotivity_mode?=release
TARGET_OS?=linux
iotivity_url?=http://github.com/iotivity/iotivity
iotivity_version?=1.2.1
export iotivity_version

tinycbor_url?=https://github.com/01org/tinycbor.git

ifeq (${iotivity_version}, 1.1.1)
tinycbor_version?=v0.2.1
endif
ifeq (${iotivity_version}, 1.2.0)
tinycbor_version?=v0.3.1
endif
ifeq (${iotivity_version}, 1.2.1)
tinycbor_version?=v0.4
endif
# 
tinycbor_version?=master
tinycbor_version=v0.4

gtest_url?=http://pkgs.fedoraproject.org/repo/pkgs/gtest/gtest-1.7.0.zip/2d6ec8ccdf5c46b05ba54a9fd1d130d7/gtest-1.7.0.zip
# TODO:
gtest_url?=https://github.com/google/googletest/archive/release-1.7.0.zip

platform?=default
export platform
iotivity_src?=${CURDIR}/tmp/src/
iotivity_dir?=${iotivity_src}iotivity-${iotivity_version}-${platform}
export iotivity_dir

iotivity_out?=${iotivity_dir}/out/${TARGET_OS}/${arch}/${iotivity_mode}

iotivity_cflags+=\
 -I${iotivity_dir}/resource/c_common/ \
 -I${iotivity_dir}/resource/csdk/stack/include \
 -I${iotivity_dir}/resource/csdk/include \
 -I${iotivity_dir}/resource/csdk/logger/include \
 #eol

iotivity_cflags+=-I${iotivity_dir}/resource/stack
#TODO: rm
iotivity_cflags+=-I${iotivity_dir}/resource/c_common/ocrandom/include

iotivity_libs?=\
 ${iotivity_out}/liboctbstack.a \
 ${iotivity_out}/resource/csdk/routing/libroutingmanager.a \
 ${iotivity_out}/libocsrm.a \
 ${iotivity_out}/libconnectivity_abstraction.a \
 ${iotivity_out}/resource/c_common/libc_common.a \
 ${iotivity_out}/libcoap.a \
 #eol

iotivity_libs+=${iotivity_out}/resource/csdk/logger/liblogger.a 

ifeq (${iotivity_version}, master)
iotivity_libs+=${iotivity_out}/libtimer.a
endif

CPPFLAGS+=${iotivity_cflags}

deps+=${iotivity_dir} tinycbor gtest

iotivity_out?=${iotivity_dir}/out/${platform}/${arch}/${iotivity_mode}

all+=deps
all+=iotivity_out

#TODO
#scons_flags+=LOGGING=1
scons_flags+=RELEASE=1
scons_flags+=ROUTING=EP 
scons_flags+=SECURED=0
scons_flags+=TARGET_TRANSPORT=IP
scons_flags+=VERBOSE=1 
scons_flags+=WITH_CLOUD=0
scons_flags+=WITH_PROXY=0
scons_flags+=WITH_TCP=0
scons_flags+=WITH_PRESENCE=0
export scons_flags

#iotivity_cflags+=-DTB_LOG=1
#iotivity_cflags+=-DROUTING_GATEWAY=1

V=1
rule/iotivity/build: ${iotivity_dir} deps
	cd $< && scons ${scons_flags}

rule/iotivity/rebuild: ${iotivity_dir} deps
	cd $< && scons ${scons_flags} -c
	cd $< && scons ${scons_flags}


${iotivity_out}: rule/iotivity/build
	@echo "# $@: $^"

iotivity_out: rule/iotivity/build
#	ls ${iotivity_out} || ${MAKE} platform=${platform} rule/iotivity/build
	@echo "# $@: $^"

${iotivity_dir}:
	mkdir -p ${@D}
	cd ${@D} && ${git_clone} ${iotivity_url} -b ${iotivity_version} ${@}

gtest: ${iotivity_dir}/extlibs/gtest/gtest-1.7.0.zip
	@echo "# $@: $^"

${iotivity_dir}/extlibs/gtest/gtest-1.7.0.zip: 
	${MAKE} ${iotivity_dir}
	mkdir -p ${@D} ; cd ${@D} 
	wget -c ${gtest_url} -O ${@}

#TODO: 
${iotivity_dir}/extlibs/gtest/gtest-1.7.0.zip/github: 
	${MAKE} ${iotivity_dir}
	mkdir -p ${@D} ; cd ${@D} 
	wget -c ${gtest_url} -O ${@}
	cd ${@D} && unp $@ && mv googletest-release-1.7.0 gtest-1.7.0

${iotivity_dir}/extlibs/tinycbor/tinycbor:
	${MAKE} ${iotivity_dir}
	cd "${iotivity_dir}" \
 && git clone ${tinycbor_url} -b ${tinycbor_version} "extlibs/tinycbor/tinycbor"


tinycbor: ${iotivity_dir}/extlibs/tinycbor/tinycbor
	@echo "# tinycbor_version=${tinycbor_version}"

iotivity/setup/debian: /etc/debian_version
	which apt-get
	which sudo || apt-get install sudo
	sudo apt-get install make psmisc wget git unp sudo aptitude xterm
	sudo apt-get install uuid-dev
	sudo apt-get install libboost1.55-dev libboost1.55-all-dev libboost-thread1.55-dev \
	|| sudo apt-get install libboost-dev libboost-program-options-dev libexpat1-dev libboost-thread-dev

iotivity/setup: iotivity/setup/debian
	@echo "# $@: $^"
