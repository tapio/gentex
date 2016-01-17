#!/bin/bash

ROOT="$(dirname "$(readlink -f "$0")")"/..

cppcheck --enable=all --quiet --verbose --config-exclude="$ROOT/third-party" "$ROOT/src" "$ROOT/tests"


