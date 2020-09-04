/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include <vector>
#include <forward_list>
#include <memory>

// TODO: Consider moving this somewhere when there's more than just one basic screen type, where each screen type needs its own layout of layers.
enum class GfxLayer {
    base, // For the game or menu.
    buttons,
    messages,
    animations
};

// TODO: Create a struct type for portable colors and convert to backend format in each backend's Graphic subclass.
#define R(N) ((N & 0xFF000000) / 0x1000000)
#define G(N) ((N & 0x00FF0000) / 0x0010000)
#define B(N) ((N & 0x0000FF00) / 0x0000100)
#define A(N) (N & 0x000000FF)

namespace Shiro {
    struct Graphic {
        virtual ~Graphic();

        virtual void draw() const = 0;
    };

    class Gfx;
    class Layers {
        friend Gfx;

    public:
        /**
         * Pushes a new graphic onto the end of a layer's graphic list.
         * You can hold a shared_ptr to the graphic in an entity if you want to reuse it over multiple updates.
         * If you don't hold the pointer, then the graphic will be freed after a draw() or clear() call.
         */
        void push(const size_t layerNum, std::shared_ptr<Graphic> graphic);

    private:
        /**
         * Draw all graphics' layers in increasing order, with layer 0 being the bottom-most.
         * Graphics within a layer are drawn in submission order, back to front.
         */
        void draw();

        /**
         * Clears all layers.
         */
        void clear();

        std::vector<std::vector<std::shared_ptr<Graphic>>> graphics;
    };

    class Entity {
    public:
        virtual ~Entity();

        /**
         * Returns true if the entity should remain allocated for another update.
         * Returns false if the entity is finished updating and is ready to be freed.
         */
        virtual bool update(Layers& layers) = 0;
    };

    class Gfx {
    public:
        /**
         * Pushes a new entity.
         */
        void push(std::unique_ptr<Entity> entity);

        /**
         * Updates all entities.
         * All entities that did not indicate they're ready to be freed will be updated on the next call of update().
         * Don't depend on entities updating in a specific order.
         */
        void update();

        /**
         * Draws all graphics of all layers. Call after an update() call.
         */
        void draw();

        /**
         * Clears all entities and layers.
         */
        void clear();

        /**
         * Clears all layers.
         */
        void clearLayers();

    private:
        Layers layers;
        std::forward_list<std::unique_ptr<Entity>> entities;
    };
}