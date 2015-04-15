#!/bin/bash

docker run --rm -it --name=openpilot -v $(pwd)/../:/var/code openpilot
