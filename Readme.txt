"Racod's Lair" is a free pixelart dungeoncrawling game. You (and your
friends) struggle through various dungeon levels to find and defeat
the evil Racod. Start your journey alone or with multiple friends on
one local machine.

Follow this game on:
  - http://www.indiedb.com/games/racods-lair
  - http://www.twitter.com/racodslair
  - http://www.facebook.com/racodslair/

Visit the devlog threads on:
  - http://en.sfml-dev.org/forums/index.php?topic=16367.0
  - http://www.spieleprogrammierer.de/12-projektvorstellungen-und-stellenangebote/23552-racod-s-lair-ein-coop-dungeoncrawler/

Thanks for playing :)


~~===~~ System requirements ~~===~~

The following is suggested:
  - OS: Windows, Linux
    (try wine on other systems)
  - CPU/GPU/RAM: unspecified, sorry :D
    It runs on my netbook (1.6 GHz, 2 GB Ram, integrated Intel graphics
    card) if lighting is disabled (due to GPU)
  - Software dependencies: see section on windows or linux build (below)

Note: playing in multiplayer will increase the GPU's workload if
lighting is enabled.


~~===~~ How to report ~~===~~

Error reporting is crucial to software of all kind. So feel free to
report any errors - like typos or more serious errors or crashes - to
me. Please visit

    https://github.com/cgloeckner/racods-lair/issues

to check whether your problem has been reported yet. If not you're
welcome! In case you already have a github account, please create a new
issue directly. Otherwise, you can use the above links to indiedb page,
twitter profile, facebook page, sfml-dev.org or spieleprogrammierer.de
forum thread of the game.

Anyway, please attach the crash.log and a more detailed description of
what you where doing. The logfile's path depends on your system:

    Windows: %APPDATA%\racod\crash.log
    Linux: ~/.local/share/racod/crash.log


~~===~~ Game Controls ~~===~~

The menu can be controlled either via keyboard (arrow keys, space/return,
escape) or by using a gamepad. There is no mouse support implemented, yet.
The actual game's controls can be customized after profile creation. Due
to the alpha state of the game, some keys are not used yet (like switching
the selected quickslot).

The major concept of range-based combat is the differentiation of moving
and looking. Thus both actions can be performed from each other. Here's
a quick overview:
  - At default, the character is looking towards the movement direction
    while moving. This automatic looking can be disabled or re-enabled.
  - Press a key for any looking direction (or the "Toggle Automatic Facing"
    key) to disable autolooking. The character will keep looking to the
    same direction, despite moving to another one.
  - Press the "Toggle Automatic Facing" key again to re-enable automatic
    facing. Now the character will always face the direction which it he
    moving to.

In the later game, a tutorial will cover this aspects.


~~===~~ Multiplayer ~~===~~

The multiplayer is local only using one machine. You can adjust the camera
behavior through the settings menu's "Automatic Camera" option.
If enabled, the camera will automatically split and merge depending on
the players' positions. This behavior might be a bit disturbing, so it
can be easily disabled during the game. After that, you have the option
to manuelly setup the camera distribution across all players.


~~===~~ Windows Version ~~===~~

The windows version was build as 32-bit application using MinGW 4.9.2 and
has been tested on the following systems:
  - Windows 7, 64 Bit
  - Windows 10, 64 Bit

All required libraries are provided by this package. Run "racod_game.exe"
to start the


~~===~~ Linux Version ~~===~~

The linux version was built as 64-bit application using LLVM Clang 3.6.2
and has been tested on the following systems:
  - Ubuntu 14.04, 64 Bit
  - Ubuntu 15.10, 64 Bit
  - Ubuntu 16.04, 64 Bit

The following libraries are required to be installed
  - SFML >= 2.3
  - Lua >= 5.2

Note that this game was developed using SFML 2.3 and Lua 5.2. Later versions
are not tested. Run "racod_game" to start the game. If your systems
lacks a suitable SFML version, you can run "racod.sh" to use the SFML
libraries that are provided by this package.

There is no 32-bit linux version, yet. But you can try the windows version
through wine.


~~===~~ Licence ~~===~~

"Racod's Lair" is licenced under CC BY-NC 4.0 with attribution to
Christian Glöckner. See https://creativecommons.org/licenses/by-nc-sa/4.0/
for more information about the licence details. All game resources, except
the provided fonts, are part of "Racod's Lair" under the mentioned licence.

The used fonts were published as "free" by Hazel Abbiati, see
http://www.dafont.com/simone-u-dono.d3997 for more information.

The music is based on the "SalamanderGrandPiano" samples (CC-BY 3.0 by
Alexander Holm) and the "Sonatina Symphonic Orchestra" (CC Sampling Plus 1.0
license by Mattias Westlund) as well as the "fluid-soundfont-gm", taken
from the official ubuntu repository (originally by Toby Smithe).

The game is based on:
  - SFML, zlib/png license, http://www.sfml-dev.org
  - Thor, zlib/png licence, http://www.bromeon.ch/libraries/thor
  - Boost, boost licence, http://www.boost.org/
  - ImGui, MIT Licence, http://github.com/ocornut/imgui
  - ImGui-SFML, MIT Licence, http://github.com/EliasD/imgui-sfml
  - Sol2, MIT License, http://github.com/ThePhD/sol2
  - Lua, MIT Licence, http://www.lua.org

