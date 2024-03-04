#!/bin/sh
# first clean images older than 24h, you will run out of disk space one day
docker image prune -a --force --filter "until=24h"
docker build -t esp .
docker run --rm -v ${PWD}:/project -w /project esp idf.py build