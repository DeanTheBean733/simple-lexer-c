#!/bin/bash

make

if [ -x ./example ]; then
    ./example
else
    echo "Build failed or executable not found."
fi
