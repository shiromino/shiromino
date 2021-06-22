# shiromino version history
## 0.2.1
### New features
- Added `SAMPLE_SIZE` configuration option for the audio mixer.
- Re-added `VIDEO_STRETCH` configuration option with updated functionality.
### Updated features
- Improved design of all tetrion graphics.
- Pentomino C revision: `1.4` → `1.5`.
- Changed floorkick rules for J5, L5, Ya, Yb, N, and G pentominoes.
### Bug fixes
- Section time display no longer renders behind the Multi-Editor menu while the game is stopped.
- Fixed crash on program exit while a game is open.
- Fixed erroneously busy-looping instead of delaying for frame timing.
## 0.2.0
### New features
- Re-introduced Multi-Editor.
- Added new background and music track to Multi-Editor.
- Added goal level, goal time, garbage delay, fumenize, and randomizer seed options to Multi-Editor.
- Added padlock button to Multi-Editor which locks the playfield grid from editing.
- Added several new keyboard shortcuts to Multi-Editor.
- Added new 10-wide tetrion graphics for both the G1 20G mode and the Multi-Editor.
- Live section times are now saved to the SQLite database during gameplay and automatically cleared upon game exit.
- Best section times for each mode are now automatically saved to the SQLite database upon game exit.
- Added support for ST, SK, CO, AC, RE, and RO medals to all game modes.
- Added credit roll to every mode.
- Added scrolling credits to credit roll.
- Added level-stop bell to Pentomino C and G3 Terror modes.
- Replays can now be deleted from within the replay menu.
### Updated features
- Pentomino C revision: `1.3` → `1.4`.
- Increased score requirements for each grade in Pentomino C.
- Removed rank control from Pentomino C.
- Pentomino C now has a difficulty curve.
- Pentomino C: Removed ceiling kicks for tetrominoes.
- Changed pentomino/tetromino naming scheme to use '5' suffix for pentominoes instead of using '4' suffix for tetrominoes.
- Improved kick behavior and rotation tables for T5 pentomino and the P/Q pentominoes respectively.
- Improved stack animations and added new overlay text for game over, credit roll, and credit roll clear.
- The M and GM grades are no longer awarded unless the game was started from level 0.
- Sprites and GUI now use integer scaling only and automatically center to the window.
- The game no longer forces 640x480 resolution before upscaling to fit the window.
- Removed OpenGL.
- Swapped hard fullscreen and soft fullscreen hotkeys.
- Removed FRAME_DELAY configuration option.
- Removed VIDEO_STRETCH configuration option.
- Renamed `tetrion_qs_white` to `qs_tetrion`.
### Bug fixes
- Fixed combo value calculation.
- Fixed all-clear (bravo) detection.
## 0.1.1
### New features
- The game now supports looking up the configuration file, the SQLite database, and the assets under various different paths.
- The build system now supports installing the game into a specified directory, making it much simpler to package the game.
### Updated features
- OpenGL has been disabled by default when using macOS, since Apple has deprecated it.
- You can now compile the game with `g++-5` and `clang++-5`.
- The CI pipeline now adds a new release for every pushed tag.
- The CI pipeline also provides Windows builds for every release.
### Bug fixes
- Garbage creation has been fixed in G3 Terror.
## 0.1.0
### New features
- Joystick input is now fully configurable; no joystick input is enabled by default.
- Vertical sync option added; off by default.
- Frame delay setting added; set to 1 ms by default. Only applies when not using vertical sync.
- "Vsync timestep" setting added; disabled by default. Allows forcing game frame rate to be in step with display refresh rate when vsync and vsync timestep are both enabled.
- The G1, G2, and G3 modes now all run at the FPS of the original games they're based on.
- Interpolation option added; forces OpenGL rendering and applies a smoothing filter to the screen, where the filtration tries to keep the graphics looking pixelated.
### Updated features
- Build system changed to CMake; the old Makefile is no longer available.
- Change to an INI file loader, replacing the CFG loader; `game.cfg` is now `shiromino.ini`, `audio/volume.cfg` is now `assets/audio/volume.ini`.
### Bug fixes
- Preview sounds are now playing.
- Sound volume settings for individual music tracks and sound effects works now.
- Frame timing now guaranteed to be perfect over time, regardless of mode; individual frames might be off a bit, depending on vertical sync and frame delay settings.
## 0.0.3
Originally released as `v.beta3-pre2` on 2018-06-19.
### New features
- Added support for big blocks in all modes.
- New mode: Big Master.
- Added credit roll to G3 Terror.
- Added Pentomino C revision string (output to console on startup).
### Updated features
- Increased logical playfield height from 22 to 24.
- Renamed "Pentomino" mode to "Pentomino C".
- Pentomino C revision: `1.0` → `1.1`.
- Pentomino C lock delay made longer throughout speed curve.
- All pentominoes and tetrominoes now have unique colors.
- Updated line clear animation's per-piece colors.
- Improved the look of some minor graphics.
### Bug fixes
- Fixed T tetromino floor-kick orientation limitations.
- Fixed grade display when green line or orange line flags are applied.
- No more grade-up sound effect in multi-editor.
## 0.0.2
Originally released as `v.beta3-pre1` on 2018-05-08.
### Bug fixes
- Fixed I tetromino's wallkick behavior.
### New features
- Window can be resized arbitrarily larger than the default `640×480`.
- Added fullscreen mode and soft full-screen mode (hotkeys `F11` and `Shift + F11` respectively).
### Updated features
- Moved from C to C++.
- Replays are saved to an SQLite database now instead of the filesystem.
- Ripped out old menu system and introduced SGUIL, a GUI library for SDL.
- Overhauled and gave significantly more structure to game logic.
- Cleaned up the embarrassing mess of spaghetti code that comprised the Multi-Editor.
- Removed `bstrlib` dependency.
- V pentomino now floor-kicks.
- T pentomino now floor-kicks even if airborne.
## 0.0.1
Originally released as `v.beta2` on 2017-08-23.
### New features
- Joystick support (kind of… still very much recommend `Joy2Key`).
- "Escape" button is now mappable.
- New mode: G1 MASTER.
- New mode: G1 20G.
- New mode: G2 MASTER.
- Three new backgrounds past level 1000.
- G1 and G2 MASTER grade systems added.
- PENTOMINO grade system added (based on G1 grade system).
- All grade sprites added.
- Grade and medal animations and grade-up sounds.
- New fonts: `font_small.png`, `font_tiny.png`, `font_fixedsys_excelsior.png`.
- New tetrion graphics.
- Full block face palette in the practice tool.
- New options in the practice tool: infinite floor kicks, lock protection.
- Practice tool UI moved around a bit.
- Basic support for Sakura gems in the practice tool.
### Updated features
- Darkened a few backgrounds.
- Removed PENTOMINO 20G and NIGHTMARE MODE.
- MASTER renamed to PENTOMINO.
- PENTOMINO speed curve is more difficult.
- PENTOMINO randomizer heavily re-balanced.
- New piece colors for several pentominoes.
- T (pentomino) now floor-kicks.
- `is_recovering` threshold changed from >= 150 blocks in playfield to >= 170.
- Recovery medal threshold changed from <= 70 blocks to <= 85.
- PENTOMINO maximum floor kicks lowered from 3 to 2.
- G1 and G3 randomizers are authentic.
- Randomizer code modularized (can now implement many new randomizers with ease).
- Main menu now remembers the choice you had selected.
- Main menu no longer reloads pointlessly if you press escape.
- Practice tool now saves your work until you close the game completely, except the piece sequence. It's on the to-do list.
- Lots of minor code refactoring.
### Bug fixes
- Using hold for the first time at the end of a user-defined sequence no longer crashes the game.
- Replays no longer influence the randomizer seed of the next game.
- Grades are correctly displayed.
- G3 TERROR ends at level 1300 as intended.
- T4 piece now floor-kicks correctly.
- Garbage (gray) cells are now drawn with an outline.
- `HOME_PATH` setting can work with different system path formats.
## 0.0.0
Originally released as `v.beta1` on 2017-04-16.
### New features
- First release.