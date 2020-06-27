#!/bin/bash
sh build_clean.sh
mkdir -p dist
cp build/solomons_key dist/
cp build/sdl_solomons_key dist/
cp build/solomons_key.exe dist/
cp *.osk dist/
cp -r res dist/res
zip -r osk-$(date +%Y-%m-%d).zip dist
rm -rf dist
