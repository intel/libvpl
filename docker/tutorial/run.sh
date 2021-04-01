#!/bin/bash

docker build -t onevpl_tutorial . 
docker run -it --rm --privileged -v `pwd`/../../examples:/examples -w /examples onevpl_tutorial:latest

