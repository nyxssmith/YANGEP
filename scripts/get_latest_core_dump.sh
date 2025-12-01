#!/bin/bash

# this only works on linux systems with coredumps enabled

coredumpctl -o core.dump dump /home/nyx/Git/YANGEP/build/yangep
