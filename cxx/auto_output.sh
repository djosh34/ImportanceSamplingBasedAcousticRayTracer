#!/bin/bash

for flag in "${flags[@]}"
do
    ./Raytracer --config ./input/input_warehouse.json
done
