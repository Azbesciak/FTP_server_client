#!/bin/bash
make server > /dev/null || exit 1
./server
