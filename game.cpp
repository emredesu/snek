#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <iterator>
#include "game.h"
#include "sprite.h"
#include "text.h"
#include "snek.h"

Game::Game() {
	// initialize SDL
	if (SDL_Init((SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0))
		std::cerr << "failed to initialize SDL: " << SDL_GetError() << std::endl;
	else {
		window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
			std::cerr << "window couldn't be created: " << SDL_GetError() << std::endl;
		else {
			// initialize renderer
			game_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // vsync on
			if (game_renderer == NULL)
				std::cerr << "renderer couldn't be created: " << SDL_GetError() << std::endl;
			else {
				// initialize renderer colour (background colour when nothing is rendered on it)
				SDL_SetRenderDrawColor(game_renderer, 255, 255, 255, 255);

				// initialize the ability to load PNG files
				int img_flags = IMG_INIT_PNG;
				if (!(IMG_Init(img_flags) & img_flags))
					std::cerr << "SDL_image couldn't initialize: " << IMG_GetError() << std::endl;
				else {
					// initialize SDL text stuff
					if (TTF_Init() == -1)
						std::cerr << "SDL_ttf couldn't initialize: " << IMG_GetError() << std::endl;
					else {
						// initialize audio
						if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
							std::cerr << "SDL_mixer couldn't initialize: " << Mix_GetError() << std::endl;
						else {
							// now that we successfully initialized everything, we load some textures and audio and then set the game_state to main menu
							menu_image = { "sprites/menu.png", game_renderer, textures, 0, 0 };
							instructions_image = { "sprites/instructions.png", game_renderer, textures, 0, 0 };
							start_text = { "press a snek", game_renderer, textures, 400, 400 };
							grid = { "sprites/grid.png", game_renderer, textures, 0, 0 };
							congratulations_image = { "sprites/end.png", game_renderer, textures, 0, 0 };
							sound_on_sprite = { "sprites/sound_on.png", game_renderer, textures, 0, SCREEN_HEIGHT - 50 };
							sound_off_sprite = { "sprites/sound_off.png", game_renderer, textures, 0, SCREEN_HEIGHT - 50 };
							github_logo_sprite = { "sprites/github_logo.png", game_renderer, textures, 60, SCREEN_HEIGHT - 50 };

							restart_game_text = { "Press ENTER to restart the game!", game_renderer, textures, 200, 490, 20 };
							exit_game_text = { "Press ESC to exit snek (makes snek sad) :c", game_renderer, textures, 190, 600, 18 };
							you_won_text = { "YOU HECKIN WON!!!", game_renderer, textures, 250, 300, 40 };

							menu_music = Mix_LoadMUS("music/menu_music.wav");
							ingame_music = Mix_LoadMUS("music/ingame_music.wav");

							collect_sfx = Mix_LoadWAV("sfx/collect.wav");
							end_sfx = Mix_LoadWAV("sfx/end.wav");

							Mix_PlayMusic(menu_music, -1);

							game_state = GameState::GAME_MENU;
						}
					}
				}
			}
		}
	}
}

void Game::quit() {
	game_state = GameState::GAME_QUIT;

	// free the memory from all the loaded textures
	for (SDL_Texture *texture : textures) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}

	SDL_DestroyRenderer(game_renderer);
	SDL_DestroyWindow(window);
	
	Mix_FreeChunk(collect_sfx);
	Mix_FreeChunk(end_sfx);

	Mix_FreeMusic(menu_music);
	Mix_FreeMusic(ingame_music);

	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Game::initialize_game() {
	if (snek_nodes.size() > 0)
		snek_nodes.clear();

	SnekNode head = { "sprites/snek_head.png", game_renderer, textures, 400, 350 };
	snek_nodes.emplace_back(head);

	if (sound_on) {
		if (Mix_PlayingMusic() == 1)
			Mix_HaltMusic();
		Mix_PlayMusic(ingame_music, -1);
	}

	// reset game vars
	score = 0;
	foods_eaten = 0;
	snek_move_interval = 1000;
	last_speed_increase = 0;

	score_text = { "Score: 0", game_renderer, textures, 10, SCREEN_HEIGHT - 30 };

	spawn_food();
}

void Game::play_if_sound_on(Mix_Chunk *sfx, int loops=0) {
	if (sound_on)
		Mix_PlayChannel(-1, sfx, loops);
}

void Game::check_food_eat() {
	if (SDL_HasIntersection(&snek_nodes[0].node_sprite.rect, &food.rect)) { // stuff that happens when snek eats food
		foods_eaten++;

		play_if_sound_on(collect_sfx);

		score += foods_eaten * 10;
		score_text_stream << "Score: " << score;
		score_text = { score_text_stream.str(), game_renderer, textures, 10, SCREEN_HEIGHT - 30 };
		score_text_stream.str(std::string()); // empty the string stream

		if (snek_move_interval != 50) {
			snek_move_interval -= 50;
		}
		SDL_DestroyTexture(food.texture);
		food.texture = nullptr;

		create_new_snek_node();
		spawn_food();
	}
}

void Game::move_snek() {
	SDL_Rect rect = snek_nodes[0].node_sprite.rect;
	int index = 0;
	
	for (auto&& node : snek_nodes) {
		if (index == 0) {
			node.move(current_direction);
			index++;
			continue;
		}

		SDL_Rect new_rect = rect;
		rect = node.node_sprite.rect;
		node.node_sprite.rect = new_rect;
		index++;
	}
	check_food_eat();
}

int Game::generate_random_number(int range_begin, int range_end, int multiples = 1) {
	std::random_device rand;
	std::mt19937 gen(rand());
	std::uniform_int_distribution<> dis(range_begin, range_end);
	return dis(gen) * multiples;
}

void Game::create_new_snek_node() {
	SnekNode last_in_vec = snek_nodes.back();
	int startx = last_in_vec.node_sprite.rect.x;
	int starty = last_in_vec.node_sprite.rect.y;


	if (snek_nodes.size() >= 2) {
		SnekNode one_before_last = snek_nodes[snek_nodes.size() - 2];

		if (one_before_last.node_sprite.rect.x + 50 == last_in_vec.node_sprite.rect.x)
			startx += 50;
		else if (one_before_last.node_sprite.rect.x - 50 == last_in_vec.node_sprite.rect.x)
			startx -= 50;
		else if (one_before_last.node_sprite.rect.y - 50 == last_in_vec.node_sprite.rect.y)
			starty -= 50;
		else if (one_before_last.node_sprite.rect.y + 50 == last_in_vec.node_sprite.rect.y)
			starty += 50;
	}
	else {
		switch (current_direction) {
			case NodeDirection::UP:
				starty += 50;
				break;
			case NodeDirection::DOWN:
				starty -= 50;
				break;
			case NodeDirection::LEFT:
				startx += 50;
				break;
			case NodeDirection::RIGHT:
				startx -= 50;
				break;
		}
	}

	SnekNode node = { "sprites/snek_tail.png", game_renderer, textures, startx, starty };
	snek_nodes.emplace_back(node);
}

void Game::spawn_food() {
	// move food until a free place has been found - todo - surely there's a more efficient way to do this?
	while (true) {
		bool was_found = false;

		int x = generate_random_number(0, (SCREEN_WIDTH / tile_size) - 1, tile_size); // 20x15 grid
		int y = generate_random_number(0, (SCREEN_HEIGHT / tile_size) - 1, tile_size);

		food.rect.x = x;
		food.rect.y = y;

		for (auto&& node : snek_nodes) {
			if (SDL_HasIntersection(&food.rect, &node.node_sprite.rect)) {
				was_found = true;
				break;
			}
		}

		if (was_found)
			continue;
		else
			break;
	}
	food.swap_textures(food_sprites[generate_random_number(0, 3)], game_renderer, textures);
}

void Game::end_screen() {
	SDL_DestroyTexture(end_score_text.texture);
	end_score_text.texture = nullptr;

	if (Mix_PlayingMusic() == 1)
		Mix_HaltMusic();

	play_if_sound_on(end_sfx);

	end_score_text_stream << "Your score: " << score;
	end_score_text = { end_score_text_stream.str(), game_renderer, textures, 200, 400 };
	end_score_text_stream.str(std::string());
}

void Game::handle_events() {
	SDL_Event event;

	if (SDL_PollEvent(&event) != 0) { // if the event poll is not empty
		if (event.type == SDL_QUIT)
			quit();
		else if (event.type == SDL_KEYDOWN) {
			SDL_Keycode pressed_key = event.key.keysym.sym;
			process_input(pressed_key);
		}
	}

	if (game_state == GameState::GAME_MENU) { // main menu buttons
		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			SDL_Rect mouse_rect;
			mouse_rect.w = mouse_rect.h = 1;
			SDL_GetMouseState(&mouse_rect.x, &mouse_rect.y);

			if (SDL_HasIntersection(&mouse_rect, &sound_on_sprite.rect) && sound_on) {
				sound_on = false;
				if (Mix_PlayingMusic() == 1)
					Mix_HaltMusic();
			}
			else if (SDL_HasIntersection(&mouse_rect, &sound_off_sprite.rect) && !sound_on) {
				sound_on = true;
				if (Mix_PlayingMusic() == 0)
					Mix_PlayMusic(menu_music, -1);
			}
			else if (SDL_HasIntersection(&mouse_rect, &github_logo_sprite.rect)) {
				#ifdef __WIN32__
				system("start https://github.com/emredesu/snek");
				#elif __APPLE__ || __MACH__
				system("open https://github.com/emredesu/snek");
				#elif __linux__
				system("xdg-open https://github.com/emredesu/snek");
				#endif
			}
		}
	}
	else if (game_state == GameState::GAME_ACTIVE) {
		if (snek_move_timer + snek_move_interval < SDL_GetTicks()) { // automatic snek movement
			snek_move_timer = SDL_GetTicks();
			move_snek();
		}
	
		// teleport snek to the other side if it's out of bounds
		if (snek_nodes[0].node_sprite.rect.x >= SCREEN_WIDTH) {
			snek_nodes[0].node_sprite.rect.x = 0;
			check_food_eat();
		}
		if (snek_nodes[0].node_sprite.rect.x < 0) {
			snek_nodes[0].node_sprite.rect.x = SCREEN_WIDTH - tile_size;
			check_food_eat();
		}
		if (snek_nodes[0].node_sprite.rect.y < 0) {
			snek_nodes[0].node_sprite.rect.y = SCREEN_HEIGHT - tile_size;
			check_food_eat();
		}
		if (snek_nodes[0].node_sprite.rect.y >= SCREEN_HEIGHT) {
			snek_nodes[0].node_sprite.rect.y = 0;
			check_food_eat();
		}

		bool first = true;
		for (auto&& node : snek_nodes) { // snek self collusion
			if (first) {
				first = false;
				continue;
			}

			if (SDL_HasIntersection(&snek_nodes[0].node_sprite.rect, &node.node_sprite.rect)) {
				game_state = GameState::GAME_END;
				end_screen();
			}
		}
		
	}
	else if (game_state == GameState::GAME_END) { // text that changes colour
		if (end_text_colour_change_interval + end_text_colour_change_timer < SDL_GetTicks()) {
			change_colour = !change_colour;

			SDL_DestroyTexture(you_won_text.texture);
			you_won_text.texture = nullptr;

			if (change_colour)
				you_won_text = { "YOU HECKIN WON!!!", game_renderer, textures, 250, 300, 40, {128, 0, 128} };
			else
				you_won_text = { "YOU HECKIN WON!!!", game_renderer, textures, 250, 300, 40 };

			end_text_colour_change_timer = SDL_GetTicks();
		}
	}
}

void Game::process_input(SDL_Keycode pressed_key) {
	if (pressed_key == SDLK_ESCAPE)
		quit();

	if (game_state == GameState::GAME_MENU and pressed_key != SDLK_RETURN)
		game_state = GameState::GAME_INSTRUCTIONS;

	if (game_state == GameState::GAME_INSTRUCTIONS and pressed_key == SDLK_RETURN) {
		initialize_game();
		game_state = GameState::GAME_ACTIVE;
	}

	if (game_state == GameState::GAME_ACTIVE) {
		switch (pressed_key) {
			case SDLK_w:
			case SDLK_UP:
				// snake head up
				if (!(current_direction == NodeDirection::UP || current_direction == NodeDirection::DOWN)) {
					current_direction = NodeDirection::UP;
					move_snek();
					snek_move_timer = SDL_GetTicks();
				}
				break;
			case SDLK_d:
			case SDLK_RIGHT:
				// snake head right
				if (!(current_direction == NodeDirection::LEFT || current_direction == NodeDirection::RIGHT)) {
					current_direction = NodeDirection::RIGHT;
					move_snek();
					snek_move_timer = SDL_GetTicks();
				}
				break;
			case SDLK_a:
			case SDLK_LEFT:
				// snake head left
				if (!(current_direction == NodeDirection::LEFT || current_direction == NodeDirection::RIGHT)) {
					current_direction = NodeDirection::LEFT;
					move_snek();
					snek_move_timer = SDL_GetTicks();
				}
				break;
			case SDLK_s:
			case SDLK_DOWN:
				// snake head down
				if (!(current_direction == NodeDirection::UP || current_direction == NodeDirection::DOWN)) {
					current_direction = NodeDirection::DOWN;
					move_snek();
					snek_move_timer = SDL_GetTicks();
				}
				break;
			case SDLK_m:
				sound_on = !sound_on;
				if (sound_on)
					Mix_PlayMusic(ingame_music, -1);
				else
					Mix_HaltMusic();
				break;
		}		
	}
	else if (game_state == GameState::GAME_END) {
		if (pressed_key == SDLK_RETURN) {
			game_state = GameState::GAME_ACTIVE;
			initialize_game();
		}
	}
}

void Game::update() {
	SDL_RenderClear(game_renderer); // clear screen - this always has to be on top of the update function

	if (game_state == GameState::GAME_MENU) {
		SDL_RenderCopy(game_renderer, menu_image.texture, 0, &menu_image.rect);
		SDL_RenderCopy(game_renderer, github_logo_sprite.texture, 0, &github_logo_sprite.rect);

		if (menu_text_flash_interval + menu_text_flash_timer < SDL_GetTicks()) { // blinking text
			menu_text_flash_timer = SDL_GetTicks();
			render_menu_text = !render_menu_text;
		}

		if (render_menu_text)
			SDL_RenderCopy(game_renderer, start_text.texture, 0, &start_text.rect);

		if (sound_on)
			SDL_RenderCopy(game_renderer, sound_on_sprite.texture, 0, &sound_on_sprite.rect);
		else if (!sound_on)
			SDL_RenderCopy(game_renderer, sound_off_sprite.texture, 0, &sound_off_sprite.rect);
	}
	else if (game_state == GameState::GAME_ACTIVE) {
		SDL_RenderCopy(game_renderer, grid.texture, 0, &grid.rect);
		SDL_RenderCopy(game_renderer, score_text.texture, 0, &score_text.rect);
		SDL_RenderCopy(game_renderer, food.texture, 0, &food.rect);

		for (auto&& node : snek_nodes)
			SDL_RenderCopy(game_renderer, node.node_sprite.texture, 0, &node.node_sprite.rect);
	}
	else if (game_state == GameState::GAME_INSTRUCTIONS)
		SDL_RenderCopy(game_renderer, instructions_image.texture, 0, &instructions_image.rect);
	else if (game_state == GameState::GAME_END) {
		SDL_RenderCopy(game_renderer, congratulations_image.texture, 0, &congratulations_image.rect);
		SDL_RenderCopy(game_renderer, you_won_text.texture, 0, &you_won_text.rect);
		SDL_RenderCopy(game_renderer, end_score_text.texture, 0, &end_score_text.rect);
		SDL_RenderCopy(game_renderer, restart_game_text.texture, 0, &restart_game_text.rect);
		SDL_RenderCopy(game_renderer, exit_game_text.texture, 0, &exit_game_text.rect);
	}
}

void Game::render() {
	SDL_RenderPresent(game_renderer);
}

int main(int argc, char *args[]) {
	Game game;

	while (game.game_state != GameState::GAME_QUIT) {
		game.update();
		game.render();
		game.handle_events();
	}
	return 0;
}