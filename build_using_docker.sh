#!/bin/bash

docker build -t sayorios .
docker run -v $PWD:/output sayorios
