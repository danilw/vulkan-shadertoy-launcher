#!/bin/sh

# build yariv first and put yariv_pack binary to this folder

rm -rf src
rm -rf shadertoy
rm -rf bin

cp -r ../../launcher/shaders/src/ src
cp -r ../../launcher/shaders/shadertoy/ shadertoy 

python yariv_to_hex.py src/main.vert
python yariv_to_hex.py src/main.frag

python yariv_to_hex.py src/buf.vert
python yariv_to_hex.py src/buf.frag
python yariv_to_hex.py src/buf1.frag
python yariv_to_hex.py src/buf2.frag
python yariv_to_hex.py src/buf3.frag

mv src/bin .
rm -rf src
rm -rf shadertoy
