#!/bin/bash

VERSION="$(git describe --tags --dirty)"

echo "version !text \"${VERSION^^}\", 0" > version.asm