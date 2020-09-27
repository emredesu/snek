#pragma once
#include <SDL.h>
#include <vector>
#include <sstream>
#include <memory>
#include "sprite.h"
#include "text.h"
#include "snek.h"

enum class GameState {
	DUMMY_VALUE,
	GAME_MENU,
	GAME_INSTRUCTIONS,
	GAME_ACTIVE,
	GAME_END,
	GAME_QUIT
};

class Game {
	public:
		const int tile_size = 50;
		const int SCREEN_WIDTH = tile_size * 20; // 1000px width
		const int SCREEN_HEIGHT = tile_size * 15; // 750px height
		const char* window_title = "snek";
		GameState game_state = GameState::DUMMY_VALUE;
		SDL_Renderer* game_renderer = NULL;
		SDL_Window* window = NULL;

		std::vector<SDL_Texture*> textures;

		bool sound_on = true;

		// menu
		Text start_text;
		Uint32 menu_text_flash_interval = 500;
		Uint32 menu_text_flash_timer = 0;
		bool render_menu_text = true;

		Sprite menu_image;
		Sprite sound_on_sprite;
		Sprite sound_off_sprite;
		Sprite github_logo_sprite;

		Mix_Music* menu_music = nullptr;

		// instructions
		Sprite instructions_image;

		// in-game
		NodeDirection current_direction = NodeDirection::UP;
		std::vector<SnekNode> snek_nodes;

		Text score_text;
		std::ostringstream score_text_stream;

		unsigned int foods_eaten = 0;
		unsigned int score = 0;
		short last_speed_increase = 0;

		Sprite grid;

		Sprite food;
		std::vector<const char*> food_sprites = { "sprites/food1.png", "sprites/food2.png", "sprites/food3.png", "sprites/food4.png" };

		Uint32 snek_move_interval = 1000;
		Uint32 snek_move_timer = 0;

		SDL_RendererFlip flip = (SDL_RendererFlip)(SDL_FLIP_NONE);

		Mix_Music* ingame_music = nullptr;

		Mix_Chunk* collect_sfx = nullptr;

		// end screen
		Sprite congratulations_image;
		Text you_won_text;
		Text end_score_text;
		Text restart_game_text;
		Text exit_game_text;
		std::ostringstream end_score_text_stream;

		Uint32 end_text_colour_change_interval = 400;
		Uint32 end_text_colour_change_timer = 0;
		bool change_colour = true;

		Mix_Chunk* end_sfx = nullptr;

		Game();
		void quit();
		void initialize_game();
		void move_snek();
		void spawn_food();
		void create_new_snek_node();
		void check_food_eat();
		int generate_random_number(int, int, int);
		void play_if_sound_on(Mix_Chunk*, int);
		void end_screen();
		void handle_events();
		void process_input(SDL_Keycode);
		void update();
		void render();
};