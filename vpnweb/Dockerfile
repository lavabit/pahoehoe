FROM debian:buster AS build
RUN apt-get -q update && env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential pkg-config golang-go git ca-certificates openvpn \
    && rm -rf /var/lib/apt/lists/*

# don't need to do bash tricks to keep the layers small, as this is a multi-stage build
ENV GOPATH=/go
WORKDIR $GOPATH
#RUN go get -u 0xacab.org/leap/vpnweb

COPY . /vpnweb
RUN cd /vpnweb && go build
RUN strip /vpnweb/vpnweb

FROM registry.git.autistici.org/ai3/docker/chaperone-base

COPY --from=build /vpnweb/vpnweb /usr/local/bin/vpnweb
COPY chaperone.d/ /etc/chaperone.d

ENTRYPOINT ["/usr/local/bin/chaperone"]
