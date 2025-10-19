FROM debian:13@sha256:fd8f5a1df07b5195613e4b9a0b6a947d3772a151b81975db27d47f093f60c6e6

RUN apt update && \
    apt install -yq \
        build-essential \
        valgrind

WORKDIR /usr/src/parking

COPY . .

RUN make && \
    valgrind --error-exitcode=1 -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./parking
