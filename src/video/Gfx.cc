#include "video/Gfx.h"
#include <iterator>
#include <utility>

namespace Shiro {
	Graphic::~Graphic() {}

	void Layers::push(const std::size_t layerNum, std::shared_ptr<Graphic> graphic) {
		if (layerNum >= graphics.size()) {
			graphics.resize(layerNum + 1);
		}
		graphics[layerNum].push_back(graphic);
	}

	void Layers::draw() const {
		for (auto& layer : graphics) {
			for (auto& graphic : layer) {
				graphic->draw();
			}
		}
	}

	void Layers::clear() {
		graphics.clear();
	}

	Entity::~Entity() {}

	void Gfx::push(std::unique_ptr<Entity> entity) {
		entities.push_front(std::move(entity));
	}

	void Gfx::update() {
		for (auto it = entities.before_begin(); next(it) != entities.end();) {
			if ((*next(it))->update(layers)) {
				entities.erase_after(it);
			}
			else {
				it++;
			}
		}
	}

	void Gfx::draw() const {
		layers.draw();
	}

	void Gfx::clearEntities() {
		entities.clear();
	}

	void Gfx::clearLayers() {
		layers.clear();
	}
}
