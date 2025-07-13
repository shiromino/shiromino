#pragma once
#include <cstddef>
#include <forward_list>
#include <memory>
#include <vector>

// TODO: Create a struct type for portable colors and convert to backend format in each backend's Graphic subclass.
#define R(C) (((C) & 0xFF000000) >> 24)
#define G(C) (((C) & 0x00FF0000) >> 16)
#define B(C) (((C) & 0x0000FF00) >>  8)
#define A(C) (((C) & 0x000000FF) >>  0)

/**
 * When put into an Entity subclass, this allows creating entities via
 * EntityType::push(gfx, entityTypeConstructorArgs...).
 *
 * This can't be put into the Gfx class, because then an attempt to instantiate
 * multiple push() functions with the same signature for different entity types
 * might occur.
 */
#define DEFINE_ENTITY_PUSH(EntityType) \
template<typename... Args> \
static inline void push(Shiro::Gfx& gfx, Args&&... args) { \
    gfx.push(std::make_unique<Shiro::EntityType>(args...)); \
}

namespace Shiro {
    // TODO: Consider moving this somewhere when there's more than just one basic screen type, where each screen type needs its own layout of layers.
    /**
     * This allows scoped enum constants just like `enum class GfxLayer`, but
     * also allows the constants to be implicitly cast to integral values. For
     * this application, it's acceptable to not use the type safety guarantee of
     * `enum class`, because these really can be interpreted as ordered numeric
     * indices, since the Layers class expects layer numbers. This also allows
     * defining a base layer number and offsets from that layer, with those
     * offsets being added to the base layer's constant.
     * -Brandon McGriff
     */
    namespace GfxLayer {
        enum type {
            base, // For the game or menu.
            buttons,
            messages,
            animations,
            emergencyBgDarken,
            emergencyButtons,
            emergencyMessages,
            emergencyAnimations
        };
    }

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
        void push(const std::size_t layerNum, std::shared_ptr<Graphic> graphic);

    private:
        /**
         * Draw all graphics' layers in increasing order, with layer 0 being the bottom-most.
         * Graphics within a layer are drawn in submission order, back to front.
         */
        void draw() const;

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
         * Returns false if the entity should remain allocated for another update.
         * Returns true if the entity is finished updating and is ready to be freed.
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
        void draw() const;

        /**
         * Clears all entities.
         */
        void clearEntities();

        /**
         * Clears all layers.
         */
        void clearLayers();

    private:
        Layers layers;
        std::forward_list<std::unique_ptr<Entity>> entities;
    };
}
