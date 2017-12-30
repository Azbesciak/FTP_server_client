#!/bin/bash
mkdir -p bin
make > /dev/null || exit 1
./bin/ftp_server
