sh build_clean.sh
mkdir -p dist
cp build/solomons_key dist/
cp *.osk dist/
cp -r res dist/res
tar -czvf osk-$(date +%Y-%m-%d).tar.gz dist
rm -rf dist
