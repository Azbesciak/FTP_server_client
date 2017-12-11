#!/bin/bash
make > /dev/null || exit 1
./bin/ftp_server
