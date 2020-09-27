#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>

// a class that makes creating text less painful albeit with 999999 parameters if you want to change things

class Text {
	public:
		SDL_Rect rect = { 0, 0, 0, 0 };
		SDL_Texture* texture = nullptr;

		Text() {};

		Text (std::string text_to_be_rendered, SDL_Renderer* renderer, std::vector<SDL_Texture*> &texture_container, int start_pos_x = 0, int start_pos_y = 0,
		int text_size = 24, SDL_Colour text_colour = { 0, 0, 0 },  const char* font_file = "fonts/dogicapixelbold.ttf", int text_width = 69, int text_height = 69) {

			if (texture != nullptr) {
				SDL_DestroyTexture(texture);
				texture = nullptr;
			}

			TTF_Font* font = TTF_OpenFont(font_file, text_size);
			if (font == NULL)
				std::cerr << "failed to open font file \"" << font_file << "\", error: " << TTF_GetError() << std::endl;
			else {
				TTF_SizeText(font, text_to_be_rendered.c_str(), &text_width, &text_height); // set text width and height automatically based on font/size/text
				SDL_Surface* text_surface = TTF_RenderText_Solid(font, text_to_be_rendered.c_str(), text_colour);

				texture = SDL_CreateTextureFromSurface(renderer, text_surface);
				texture_container.emplace_back(texture);

				rect.x = start_pos_x;
				rect.y = start_pos_y;
				rect.w = text_width;
				rect.h = text_height;

				SDL_FreeSurface(text_surface);
				text_surface = nullptr;
				TTF_CloseFont(font);
			}
		}
};