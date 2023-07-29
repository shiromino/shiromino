#pragma once
#include "video/Gfx.h"
#include "asset/Image.h"
#include "Grid.h"
#include "SPM_Structures.h"
#include <unordered_map>
#include <cstdint>

// TODO: replace references to Shiro::ImageAsset with new Shiro::AssetRef class
// instantiations of these classes would use std::string filenames to pass to AssetRef

namespace Shiro {
    namespace SPM {
        struct FieldEncoding_GfxMap
        // maps encoded ints (from Shiro::Grid) to Shiro::Gfx::Rect (describing a portion of the piece texture atlas)
        // wraps the calculations that would otherwise need to be handled by game gfx logic into a handy object
        {
            FieldEncoding_GfxMap() = delete;
            FieldEncoding_GfxMap(std::unordered_map<int32_t, Shiro::Gfx::Rect> *gfxMap) : gfxMap(gfxMap) {}

            virtual const Shiro::Gfx::Rect& get(int32_t) = 0;

            std::unordered_map<int32_t, Shiro::Gfx::Rect> *gfxMap;

            // function to actually create the map, which is declared const because it should not need to change while the game is running
            // actual implementations will derive this abstract class and define their maps using the following function archetype to construct them

            // static std::unordered_map<int, Shiro::Gfx::Rect> createMap();
        };

        struct MinoIDEncoding_GfxMap
        // maps encoded SPM_MinoID to Shiro::Gfx::Rect (describing a portion of the piece texture atlas)
        // wraps the calculations that would otherwise need to be handled by game gfx logic into a handy object
        {
            MinoIDEncoding_GfxMap() = delete;
            MinoIDEncoding_GfxMap(std::unordered_map<SPM_minoID, Shiro::Gfx::Rect> *gfxMap) : gfxMap(gfxMap) {}

            virtual const Shiro::Gfx::Rect& get(SPM_minoID) = 0;

            std::unordered_map<SPM_minoID, Shiro::Gfx::Rect> *gfxMap;

            // function to actually create the map, which is declared const because it should not need to change while the game is running
            // actual implementations will derive this abstract class and define their maps using the following function archetype to construct them

            // static std::unordered_map<int, Shiro::Gfx::Rect> createMap() = 0;
        };

        struct GameDecorations_Gfx : public Shiro::Sprite
        // generic sprite class for decorations surrounding the playfield associated with the running game
        // this would include playfield bezel + next box + hold box (in the context of typical falling-block games, those all appearing in one texture)
        {
            GameDecorations_Gfx() = delete;
            GameDecorations_Gfx(Shiro::ImageAsset *bezel, Shiro::Gfx::Point pos)
                : Shiro::Sprite(pos), bezel(bezel) {}

            GameDecorations_Gfx(Shiro::ImageAsset *bezel)
                : GameDecorations_Gfx(bezel, {0,0}) {}

            void draw() const override;

            Shiro::ImageAsset *bezel;
        };

        struct Field_Gfx : public Shiro::Sprite
        // sprite class specifically for drawing the playfield blocks, along with any extra non-animated graphical elements
        // specialized Shiro::Grid encoding schemes can be handled by derived classes; this is just a simple base implementation
        // in the future, it may be worthwhile to make a sophisticated polymorphic grid encoding -> graphical output class to handle those details,
        //   instead of using derived Field_Gfx classes for specialization
        {
            Field_Gfx() = delete;
            Field_Gfx(Shiro::ImageAsset *tileGfx, Shiro::Grid *field, Shiro::Gfx::Point pos)
                : Shiro::Sprite(pos), tileGfx(tileGfx), field(field)
            {
                drawOptions.gridlines = false;
                drawOptions.outline = true;
            }

            Field_Gfx(Shiro::ImageAsset *tileGfx, Shiro::Grid *field)
                : Field_Gfx(tileGfx, field, {0,0}) {}

            void draw() const override;

            Shiro::Grid *field;
            Shiro::ImageAsset *tileGfx;
            Shiro::u32 tileDrawSize = 16;

            struct {
                bool gridlines : 1;
                bool outline : 1;
            } drawOptions;
        };

        struct Polyomino_Gfx : public Shiro::Sprite
        // free standing polyomino sprite -- using ActivatedPolyomino class for purposes of access to orientation/minoID/physicState
        // ActivatedPolyomino data informs draw() of which rotation table and tile graphic(s) to use
        {
            Polyomino_Gfx() = delete;
            Polyomino_Gfx(Shiro::ImageAsset *tileGfx, ActivatedPolyomino *polyomino, Shiro::Gfx::Point pos)
                : Shiro::Sprite(pos), tileGfx(tileGfx), overrideID(0), polyomino(polyomino)
            {
                drawOptions.outline = false;
                drawOptions.flash = false;
                drawOptions.useOverrideID = false;
            }

            Polyomino_Gfx(Shiro::ImageAsset *tileGfx, ActivatedPolyomino *polyomino)
                : Polyomino_Gfx(tileGfx, polyomino, {0,0}) {}

            void draw() const override;

            ActivatedPolyomino *polyomino;
            Shiro::ImageAsset *tileGfx;
            Shiro::u32 tileDrawSize = 16;

            SPM_minoID overrideID; // overrideID would be used in the case of, e.g., bone blocks in G3 Terror

            struct {
                bool outline : 1;
                bool flash : 1;
                bool useOverrideID : 1;
            } drawOptions;
        };
    }
}
