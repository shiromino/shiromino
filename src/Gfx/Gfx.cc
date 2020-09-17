/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Gfx/Gfx.h"

using namespace Shiro;
using namespace std;

Graphic::~Graphic() {}

Layers::Layers(const Screen& screen) : screen(screen) {}

void Layers::push(const size_t layerNum, shared_ptr<Graphic> graphic) {
    if (layerNum >= graphics.size()) {
        graphics.resize(layerNum + 1);
    }
    graphics[layerNum].push_back(graphic);
}

void Layers::draw() {
    for (auto& layer : graphics) {
        for (auto& graphic : layer) {
            graphic->draw(screen);
        }
    }
}

void Layers::clear() {
    graphics.clear();
}

Entity::~Entity() {}

Gfx::Gfx(const Screen& screen) : screen(screen), layers(screen) {}

void Gfx::push(unique_ptr<Entity> entity) {
    entities.push_front(move(entity));
}

void Gfx::update() {
    for (auto it = entities.before_begin(); next(it) != entities.end();) {
        if (!(*next(it))->update(layers)) {
            entities.erase_after(it);
        }
        else {
            it++;
        }
    }
}

void Gfx::draw() {
    layers.draw();
}

void Gfx::clear() {
    entities.clear();
    clearLayers();
}

void Gfx::clearLayers() {
    layers.clear();
}