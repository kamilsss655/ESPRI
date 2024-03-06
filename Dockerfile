FROM --platform=amd64 espressif/idf:v5.2.1
WORKDIR /main
COPY . .

# RUN git submodule update --init --recursive
#RUN make && cp firmware* compiled-firmware/
