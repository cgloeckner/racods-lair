![alt text](http://media.indiedb.com/images/games/1/41/40378/add.png "Racod's Lair")

Racod, scourge for mankind, escaped to its lair to regenerate for his next decade of terror.
You and your brave fellows are willed to find and defeat Racod.

It's a Coop Dungeon Crawling / Hack'n'Slashing RPG (or it will be some day^^).

More information can be found in the [SFML forum](http://en.sfml-dev.org/forums/index.php?topic=16367.0) and at [Spieleprogrammierer.de](https://www.spieleprogrammierer.de/12-projektvorstellungen-und-stellenangebote/23552-racod-s-lair-ein-coop-dungeoncrawler). You can follow Racod's Lair on [Twitter](http://www.twitter.com/racodslair) and [Facebook](http://www.facebook.com/racodslair).

And this repo's [Wiki](https://github.com/cgloeckner/racods-lair/wiki)

## Build

Replace BRANCH_NAME by the corresponding branch name
```
git clone https://github.com/cgloeckner/racods-lair.git
git checkout BRANCH_NAME
git submodule update --init --recursive
```

```
mkdir build-release
cd build-release
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
ninja
```
```
mkdir build-debug
cd build-debug
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug
ninja
```

```
mkdir build-test
cd build-test
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Test
ninja
```

Optional:
- Force clang: `-DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang`
- Use proper lua: `-DLUA_INCLUDE_DIR=/usr/include/lua5.2 -DLUA_LIBRARY=lua5.2`

## Dependencies

```
Building:
    lua 5.2
    boost >= 1.36.x
    Dependencies for SFML
```

For example, a typical linux ubuntu requires the following packages:
```
# boost
libboost-all-dev

# Lua
liblua5.2-dev

# dependencies for SMFL
libpthread-stubs0-dev libgl1-mesa-dev libx11-dev libxrandr-dev libfreetype6-dev libglew1.5-dev libjpeg8-dev libsndfile1-dev libopenal-dev libudev-dev libxcb-image0-dev libjpeg-dev libflac-dev
```
(This list might change according to the latest SFML version)

## Issue

I'm using the issue functionality to track all reported issues related to the game or its content data. Feel free to join the discussion!

## Licence

"Racod's Lair" is licenced under CC BY-NC 4.0 with attribution to Christian Glöckner. See https://creativecommons.org/licenses/by-nc-sa/4.0/ for more information about the licence details. All game resources, except the provided fonts, are part of "Racod's Lair" under the mentioned licence.

The used fonts were published as "free" by Hazel Abbiati, see http://www.dafont.com/simone-u-dono.d3997 for more information.

The music is based on the "SalamanderGrandPiano" samples (CC-BY 3.0 by Alexander Holm) and the "Sonatina Symphonic Orchestra" (CC Sampling Plus 1.0 license by Mattias Westlund) as well as the "fluid-soundfont-gm", taken from the official ubuntu repository (originally by Toby Smithe).
