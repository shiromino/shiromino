# shiromino version history
## 0.1
### New features
- Joystick input is now fully configurable; no joystick input is enabled by default
- Vertical sync option added; off by default
- Frame delay setting added; set to 1 ms by default. Only applies when not using vertical sync
- "Vsync timestep" setting added; disabled by default. Allows forcing game frame rate to be in step with display refresh rate when vsync and vsync timestep are both enabled.
- The G1, G2, and G3 modes now all run at the FPS of the original games they're based on
### Updated features
- Build system changed to CMake; the old Makefile is no longer available
- Change to an INI file loader, replacing the CFG loader; game.cfg is now game.ini, audio/volume.cfg is now audio/volume.ini
### Bug fixes
- Preview sounds now play
- Sound volume settings for individual music tracks and sound effects works now
- Frame timing now guaranteed to be perfect over time, regardless of mode; individual frames might be off a bit, depending on vertical sync and frame delay settings
## beta3-pre2
### New features
- Added support for big blocks in all modes
- New mode: Big Master
- Added credit roll to G3 Terror
- Added Pentomino C revision string (output to console on startup)
### Updated features
- Increased logical playfield height from 22 to 24
- Renamed "Pentomino" mode to "Pentomino C"
- Pentomino C revision: 1.0 → 1.1
- Pentomino C lock delay made longer throughout speed curve
- All pentominoes and tetrominoes now have unique colors
- Updated line clear animation's per-piece colors
- Improved the look of some minor graphics
### Bug fixes
- Fixed T tetromino floor-kick orientation limitations
- Fixed grade display when green line or orange line flags are applied
- No more grade-up sound effect in multi-editor
## 2018.05.07 beta3-pre1
### Bug fixes
- Fixed I tetromino's wallkick behavior
## beta3
### New features
- Window can be resized arbitrarily larger than the default 640×480
- Added fullscreen mode and soft full-screen mode (hotkeys F11 and Shift+F11 respectively)
### Updated features
- Moved from C to C++
- Replays are saved to an SQLite database now instead of the filesystem
- Ripped out old menu system and introduced SGUIL, a GUI library for SDL
- Overhauled and gave significantly more structure to game logic
- Cleaned up the embarrassing mess of spaghetti code that comprised the Multi-Editor
- Removed `bstrlib` dependency
- V pentomino now floor-kicks
- T pentomino now floor-kicks even if airborne
## 2017.08.23 beta2
### New features
- Joystick support (kind of… still very much recommend `Joy2Key`)
- "Escape" button is now mappable
- New mode: G1 MASTER
- New mode: G1 20G
- New mode: G2 MASTER
- Three new backgrounds past level 1000
- G1 and G2 MASTER grade systems added
- PENTOMINO grade system added (based on G1 grade system)
- All grade sprites added
- Grade and medal animations and grade-up sounds
- New fonts: `font_small.png`, `font_tiny.png`, `font_fixedsys_excelsior.png`
- New tetrion graphics
- Full block face palette in the practice tool
- New options in the practice tool: infinite floor kicks, lock protection
- Practice tool UI moved around a bit
- Basic support for Sakura gems in the practice tool
### Updated features
- Darkened a few backgrounds
- Removed PENTOMINO 20G and NIGHTMARE MODE
- MASTER renamed to PENTOMINO
- PENTOMINO speed curve is more difficult
- PENTOMINO randomizer heavily re-balanced
- New piece colors for several pentominoes
- T (pentomino) now floor-kicks
- `is_recovering` threshold changed from >= 150 blocks in playfield to >= 170
- Recovery medal threshold changed from <= 70 blocks to <= 85
- PENTOMINO maximum floor kicks lowered from 3 to 2
- G1 and G3 randomizers are authentic
- Randomizer code modularized (can now implement many new randomizers with ease)
- Main menu now remembers the choice you had selected
- Main menu no longer reloads pointlessly if you press escape
- Practice tool now saves your work until you close the game completely (except the piece sequence! It's on the todo list.)
- Lots of minor code refactoring
### Bug fixes
- Using hold for the first time at the end of a user-defined sequence no longer crashes the game
- Replays no longer influence the randomizer seed of the next game
- Grades are correctly displayed
- G3 TERROR ends at level 1300 as intended
- T4 piece now floor-kicks correctly
- Garbage (gray) cells are now drawn with an outline
- `HOME_PATH` setting can work with different system path formats
## 2017.04.16 beta1
### New features
- First release