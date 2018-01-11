#!/bin/bash
# -------------------------------------------------------------
# --- Bundle everything together -----------------------------
#   TODO adjust paths
# -------------------------------------------------------------


DIR=$PWD;

# build linux binaries
cd ~/.build/racod_64
make
tmp=$?
if [ "$tmp" != "0" ]; then 
	exit $tmp;
fi

# build windows binaries
cd $DIR;
wineconsole build.bat
tmp=$?
if [ "$tmp" != "0" ]; then 
	exit $tmp;
fi

# copy binaries to target directory
cp ~/.build/racod_64/racod_game ./bin/racod_game
cp ~/.wine/drive_c/users/christian/.build/racod_x86/racod_game.exe ./bin/racod_game.exe

# bundle zip
TMP=/tmp/racod_bundle_tmp
mkdir $TMP
mkdir $TMP/racod
cp ./Readme.txt $TMP/racod/
cp ./bin/* $TMP/racod/
cp -R ./data $TMP/racod/
cd $TMP
rm $DIR/racod.zip
zip -r $DIR/racod.zip racod/*
rm -rf $TMP
cd $DIR
