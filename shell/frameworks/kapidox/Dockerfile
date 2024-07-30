FROM debian:bullseye-slim

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install --no-install-recommends -y doxygen graphviz python3-pip python3-gv \
    && apt-get autoremove -y && apt-get clean

RUN useradd --create-home --shell /bin/bash kapidox

USER kapidox
ENV KAPIDOX_HOME=/home/kapidox
ENV BUILD_DIR=${KAPIDOX_HOME}/apidox-build
ENV PATH=${KAPIDOX_HOME}/.local/bin:${PATH}

WORKDIR ${KAPIDOX_HOME}

RUN pip3 install --upgrade pip

COPY --chown=kapidox:kapidox requirements.frozen.txt .
RUN pip3 install --no-cache-dir -r requirements.frozen.txt

COPY --chown=kapidox:kapidox . .
RUN pip3 install --no-cache-dir --no-deps --editable .

RUN mkdir ${BUILD_DIR}
WORKDIR ${BUILD_DIR}
