# Faza 1: Build okruženje
FROM ubuntu:22.04 AS build

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    make \
    autoconf \
    autoconf-archive \
    automake \
    libtool \
    libtool-bin \
    && rm -rf /var/lib/apt/lists/*

# Instaliraj vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /vcpkg \
    && /vcpkg/bootstrap-vcpkg.sh

# Instaliraj biblioteke preko vcpkg-a
RUN /vcpkg/vcpkg install cpp-httplib nlohmann-json libsodium

# Kopiraj SAMO fajlove koji su nam potrebni za server
WORKDIR /app
COPY RACUNI-SISTEM_Server/Server.cpp RACUNI-SISTEM/Baza.cpp RACUNI-SISTEM/Baza.h RACUNI-SISTEM/sqlite3.c RACUNI-SISTEM/sqlite3.h RACUNI-SISTEM/StmtPtr.h RACUNI-SISTEM/RACUNI-SISTEM.db ./

# Kompajliraj sqlite3.c posebno (C kompajler)
RUN gcc -c sqlite3.c -o sqlite3.o -I/vcpkg/installed/x64-linux/include

RUN g++ -std=c++17 -o racuniSistemServer \
    Server.cpp Baza.cpp sqlite3.o \
    -I/vcpkg/installed/x64-linux/include \
    -L/vcpkg/installed/x64-linux/lib \
    -lsodium -lpthread -ldl

# Faza 2: finalna, lagana slika
FROM ubuntu:22.04
WORKDIR /app
COPY --from=build /app/racuniSistemServer /app/RACUNI-SISTEM.db ./

EXPOSE 8080
CMD ["./racuniSistemServer"]