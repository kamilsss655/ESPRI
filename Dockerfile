FROM --platform=amd64 espressif/idf:latest
WORKDIR /main
COPY . .

# RUN git submodule update --init --recursive
#RUN make && cp firmware* compiled-firmware/
