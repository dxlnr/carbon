#!/bin/bash

# Build and run the render program.
scons > /dev/null 2>&1
./render $*
scons -c > /dev/null 2>&1
