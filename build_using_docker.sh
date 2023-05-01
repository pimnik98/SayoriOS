#!/bin/bash

sudo docker build -t sayorios .
sudo docker run -v $PWD:/output sayorios
