#!/bin/bash

rm -rf frontend
mkdir frontend
mv qtloader.js frontend/
mv qtlogo.svg frontend/
mv WASM_Higiia.html frontend/
mv WASM_Higiia.js frontend/
mv WASM_Higiia.wasm frontend/
#scp -r frontend anoti@200.143.219.66:/home/anoti/TMP
