#!/bin/bash

# Build and run the render program.
scons > /dev/null 2>&1
./render $1
scons -c > /dev/null 2>&1

