FROM ubuntu:16.04 AS builder

RUN apt-get update && apt-get install -y build-essential file

COPY . .
RUN LDFLAGS=-static make
RUN strip build/bin/*
RUN ls -l build/bin/*
RUN file build/bin/*
RUN ldd build/bin/* || true
RUN mkdir -p dist/usr/bin
RUN cp build/bin/* dist/usr/bin
RUN build/bin/mkbom dist/ dist/Bom

# ===========================================================================
FROM scratch

COPY --from=builder build/bin/* /usr/bin/
COPY --from=builder dist/Bom /Bom