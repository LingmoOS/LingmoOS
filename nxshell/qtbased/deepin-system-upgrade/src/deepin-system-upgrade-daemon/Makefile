PRJ=deepin-system-upgrade-daemon
PROG_UPGRADER=${PRJ}
PWD=$(shell pwd)
GOCODE=/usr/share/gocode
GOPATH_DIR=gopath
CURRENT_DIR=$(nodir $(shell pwd))

export GO111MODULE=off

all:build

prepare:
	@if [ ! -d ${GOPATH_DIR}/src/${PRJ} ]; then \
		mkdir -p ${GOPATH_DIR}/src/${PRJ}; \
		ln -sf ${PWD}/pkg ${GOPATH_DIR}/src/${PRJ}; \
		ln -sf ${PWD}/cmd ${GOPATH_DIR}/src/${PRJ}; \
	fi

$(info, $(GOPATH))
$(warning, $(GOPATH))
$(error, $(GOPATH))

build: prepare

	@env GOPATH=${PWD}/${GOPATH_DIR}:${GOCODE}:${GOPATH} go build -o ${PWD}/${PROG_UPGRADE}  ${PRJ}/cmd/${PROG_UPGRADER}
	rm -rf ${GOPATH_DIR}


clean: 
	rm -rf ${PWD}/${PROG_UPGRADER}

rebuild: clean build
