FROM python:3.8
ARG CORE=arduino:samd

RUN apt update \
    && apt install -y \
    ca-certificates \
    git \
    curl \
    && apt clean \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --upgrade pip setuptools
RUN pip install -U platformio

RUN groupadd -r arduino && useradd --no-log-init --create-home -g arduino arduino
RUN sed "s/^dialout.*/&arduino/" /etc/group -i \
    && sed "s/^root.*/&arduino/" /etc/group -i

USER arduino

CMD ["/bin/bash", "--login"]

WORKDIR /home/arduino/

RUN pio platform install atmelsam

RUN mkdir /home/arduino/setup/ \
    /home/arduino/Documents/ \
    /home/arduino/Documents/BPM

COPY --chown=arduino . /home/arduino/Documents/BPM/

VOLUME /home/arduino/Documents
WORKDIR /home/arduino/Documents/BPM/
