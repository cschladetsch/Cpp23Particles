#!/bin/bash

mkdir -p build && cd build && cmake .. && make && ./particle_system $@

