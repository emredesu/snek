#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>

// a container class made for automatically getting the width and height of an image and setting its starting coordinates on initialization

class Sprite {
    public:
        SDL_Rect rect = {0, 0, 0, 0};
        SDL_Texture* texture = nullptr;

        Sprite() {};

        Sprite(const char* path, SDL_Renderer *renderer, std::vector<SDL_Texture*> &texture_container, int start_pos_x, int start_pos_y) {
            if (texture != nullptr) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }

            texture = IMG_LoadTexture(renderer, path);

            if (texture == NULL)
                std::cerr << "failed to load texture \"" << path << "\", error: " << IMG_GetError() << std::endl;

            texture_container.emplace_back(texture);
            SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h); // fill rect.w and rect.h with the width and height of the loaded image
            rect.x = start_pos_x;
            rect.y = start_pos_y;
        }

        void swap_textures(const char* new_texture_path, SDL_Renderer *renderer, std::vector<SDL_Texture*> texture_container) {
            SDL_DestroyTexture(texture);
            texture = nullptr;

            texture = IMG_LoadTexture(renderer, new_texture_path);
            texture_container.emplace_back(texture);
            SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
        }
};