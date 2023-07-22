#include "video/Gfx.h"

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

	void Renderer::push(std::unique_ptr<Entity> entity) {
		entities.push_front(std::move(entity));
	}

	void Renderer::update() {
		for (auto it = entities.before_begin(); next(it) != entities.end();) {
			if ((*next(it))->update(layers)) {
				entities.erase_after(it);
			}
			else {
				it++;
			}
		}
	}

	void Renderer::draw() const {
		layers.draw();
	}

	void Renderer::clearEntities() {
		entities.clear();
	}

	void Renderer::clearLayers() {
		layers.clear();
	}
}
