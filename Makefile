#! /usr/bin/make -f
# //******************************************************************
# //
# // Copyright 2014 Intel Corporation.
# // Copyright 2015 Eurogiciel <philippe.coval@eurogiciel.fr>
# // Copyright 2016 Samsung <philippe.coval@osg.samsung.com>
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

default: all

package?=iotivity-example
name?=${package}
config_pkgconfig?=1
export config_pkgconfig

#TODO: workaround missing /usr/include/iotivity namespace
iotivity_dir?=iotivity
includedir?=/usr/include
include_dir?=${PKG_CONFIG_SYSROOT_DIR}/${includedir}
ifeq (${config_pkgconfig},1)
CPPFLAGS+=$(shell pkg-config iotivity --cflags)
LIBS+=$(shell pkg-config iotivity --libs)
iotivity_dir=${include_dir}/iotivity
CPPFLAGS+=-I${iotivity_dir}/resource/c_common
CPPFLAGS+=-I${iotivity_dir}/resource/oc_logger
CPPFLAGS+=-I${iotivity_dir}/resource/stack
else
LIBS+=-loc -loc_logger -loctbstack
CPPFLAGS+=-I${iotivity_dir}
CPPFLAGS+=-I${iotivity_dir}/resource
CPPFLAGS+=-I${iotivity_dir}/resource/c_common
CPPFLAGS+=-I${iotivity_dir}/resource/oc_logger
CPPFLAGS+=-I${iotivity_dir}/resource/stack
all+=${iotivity_dir}
srcs_all+=$(wildcard src/*.cpp)
endif
CPPFLAGS+=-I.


local_bindir?=bin
local_optdir?=opt
install_dir?=${DESTDIR}/${local_optdir}/${package}/
_unitdir?=/lib/systemd/

vpath+=src
VPATH+=src

CPPFLAGS+=-DCONFIG_SERVER_MAIN=1
CPPFLAGS+=-DCONFIG_CLIENT_MAIN=1
CPPFLAGS+=-DCONFIG_OBSERVER_MAIN=1

V=1

CXXFLAGS+=-std=c++0x

srcs?=platform.cpp common.cpp
objs?=${srcs:.cpp=.o}

client?=${local_bindir}/client

server?=${local_bindir}/server

observer?=${local_bindir}/observer

exes?=${client} ${observer}
exes+=${server}
all+=${exes}


${local_bindir}/server: server.o ${server_objs} ${objs}
	@-mkdir -p ${@D}
	${CXX} -o ${@} $^ ${LDFLAGS} ${LIBS}

${local_bindir}/client: client.o ${client_objs} ${objs}
	@-mkdir -p ${@D}
	${CXX} -o ${@} $^ ${LDFLAGS} ${LIBS}

${local_bindir}/observer: observer.o ${observer_objs} ${objs}
	@-mkdir -p ${@D}
	${CXX} -o ${@} $^ ${LDFLAGS} ${LIBS}

all: help ${all}

${local_bindir}/%: %.o ${objs}
	@-mkdir -p ${@D}
	${CXX} -o ${@} $^ ${LDFLAGS} ${LIBS}

clean:
	rm -f *.o *~ ${objs} */*.o

cleanall: clean
	rm -f ${client} ${server}
	rm -rf ${local_bindir}

distclean: cleanall
	-rm iotivity

install: ${exes}
	install -d ${install_dir}
	install $^ ${install_dir}

rule/install/service: ${name}.service
	install -d ${DESTDIR}${_unitdir}/
	install $< ${DESTDIR}${_unitdir}/

${name}.service: extra/iotivity-example.service
	sed -e "s|ExecStart=.*|ExecStart=/opt/%{name}/server|g" < $< > $@

iotivity: ${include_dir}
	@echo "TODO: workaround for namespace"
	-@rm -f $@
	ls $</iotivity && ln -fs $</iotivity $@ || ln -fs $< $@
	ls -l $@

${srcs_all}: ${iotivity_dir}

run/%: ${local_bindir}/%
	${<D}/${<F}

xterm/% : ${local_bindir}/%
	xterm -e ${MAKE} run/${@F} &
	sleep 5

run: xterm/server xterm/client

run/client-auto:  ${local_bindir}/client
	-while true ; do echo 2 ; sleep 5 ;  done | $<

xterm/client-auto : ${local_bindir}/client
	sleep 5
	xterm -e ${MAKE} run/${@F} &
	sleep 5

auto: all xterm/server  run/client-auto
	killall client server

demo:all xterm/server  run/client
	killall client server

help:
	@echo "# iotivity_dir=${iotivity_dir}"
	@echo "# all=${all}"
	set
