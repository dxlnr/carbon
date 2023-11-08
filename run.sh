#!/bin/bash

# Build and run the render program.
scons > /dev/null 2>&1
./carbon $*
scons -c > /dev/null 2>&1
