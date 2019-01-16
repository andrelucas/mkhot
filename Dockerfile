FROM alpine:latest as builder

RUN apk update
# XXX join
RUN apk add boost-dev build-base cmake

COPY src /src
WORKDIR /src
RUN cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo . && make -j$(nproc)

FROM alpine:latest
RUN apk update && apk add libstdc++
COPY --from=builder /src/mkhot /
ENTRYPOINT [ "/mkhot" ]
