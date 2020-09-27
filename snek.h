#pragma once
#include "sprite.h"

enum class NodeDirection {
	DUMMY_VALUE,
	UP,
	DOWN,
	LEFT,
	RIGHT
};

class SnekNode {
	public:
		Sprite node_sprite;

		SnekNode() {};

		SnekNode(const char *path, SDL_Renderer *renderer, std::vector<SDL_Texture*> &texture_container, int startx, int starty) {
			node_sprite = { path, renderer, texture_container, startx, starty };
		}

		void move(NodeDirection dir) {
			switch (dir) {
				case NodeDirection::UP:
					node_sprite.rect.y -= 50;
					break;
				case NodeDirection::DOWN:
					node_sprite.rect.y += 50;
					break;
				case NodeDirection::LEFT:
					node_sprite.rect.x -= 50;
					break;
				case NodeDirection::RIGHT:
					node_sprite.rect.x += 50;
					break;
			}
		}
};