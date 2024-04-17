FROM alpine:3.15.0 AS build

COPY repositories /etc/apk/repositories
ADD . /src/

RUN apk update && \
    apk add build-base cmake git libusb-dev && \
    cd /src/ && \
    cmake . -DCMAKE_INSTALL_PREFIX=/usr/ && make -j24

FROM alpine:3.15.0

COPY repositories /etc/apk/repositories
RUN apk --no-cache add libusb
COPY --from=build /src/src/dalicmd /usr/bin/

