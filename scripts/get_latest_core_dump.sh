#!/bin/bash

# this only works on linux systems with coredumps enabled
rm -f core.dump
coredumpctl -o core.dump dump /home/nyx/Git/YANGEP/build/yangep
