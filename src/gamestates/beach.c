/*! \file empty.c
 *  \brief Empty gamestate.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"
#include <libsuperderpy.h>

struct GamestateResources {
		// This struct is for every resource allocated and used by your gamestate.
		// It gets created on load and then gets passed around to all other function calls.
		ALLEGRO_FONT *font;

		int counter;
		ALLEGRO_BITMAP *boy, *cloud, *girl, *lost, *off, *on, *overlay, *sand, *sea, *towel1, *towel2;
		ALLEGRO_BITMAP *canvas, *tmp;
		struct {
				int x, y;
				bool satisfied;
		} people[5];
		int power;
		int left;
		bool preparing;
		bool throwing;
		int throwx;
		int score;
		int sandx;
		int seax;
		int maxsea;
		int sandleft;
		bool seain;
		struct Character *guy;

};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	data->counter++;
	if (data->counter >= 10) {
		data->counter = 0;
		AnimateCharacter(game, data->guy, 1);
	}
}

void Gamestate_Draw(struct Game *game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.
	al_clear_to_color(al_map_rgb(255, 234, 206));
	al_draw_bitmap(data->sand, data->sandx, 0, 0);
	al_draw_bitmap(data->sea, data->seax, 0, 0);
	al_draw_bitmap(data->overlay, 0, 0, 0);
	al_draw_bitmap(data->off, 0, 80, 0);
	al_draw_bitmap(data->on, 0, 80, 0);
	DrawCharacter(game, data->guy, al_map_rgb(255,255,255), 0);
}

void Gamestate_ProcessEvent(struct Game *game, struct GamestateResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources *data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar

	data->boy = al_load_bitmap(GetDataFilePath(game, "boy.png"));
	data->cloud = al_load_bitmap(GetDataFilePath(game, "cloud.png"));
	data->girl = al_load_bitmap(GetDataFilePath(game, "girl.png"));
	data->lost = al_load_bitmap(GetDataFilePath(game, "lost.png"));
	data->off = al_load_bitmap(GetDataFilePath(game, "off.png"));
	data->on = al_load_bitmap(GetDataFilePath(game, "on.png"));
	data->overlay = al_load_bitmap(GetDataFilePath(game, "overlay.png"));
	data->sand = al_load_bitmap(GetDataFilePath(game, "sand.png"));
	data->sea = al_load_bitmap(GetDataFilePath(game, "sea.png"));
	data->towel1 = al_load_bitmap(GetDataFilePath(game, "towel1.png"));
	data->towel2 = al_load_bitmap(GetDataFilePath(game, "towel2.png"));

	data->canvas = al_create_bitmap(160, 90);
	al_set_target_bitmap(data->canvas);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_set_target_backbuffer(game->display);

	data->tmp = al_create_bitmap(160, 90);

	data->guy = CreateCharacter(game, "guy");
	RegisterSpritesheet(game, data->guy, "stand");
	RegisterSpritesheet(game, data->guy, "walk");
	LoadSpritesheets(game, data->guy);

	return data;
}

void Gamestate_Unload(struct Game *game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	al_destroy_bitmap(data->boy);
	al_destroy_bitmap(data->cloud);
	al_destroy_bitmap(data->girl);
	al_destroy_bitmap(data->lost);
	al_destroy_bitmap(data->off);
	al_destroy_bitmap(data->on);
	al_destroy_bitmap(data->overlay);
	al_destroy_bitmap(data->sand);
	al_destroy_bitmap(data->sea);
	al_destroy_bitmap(data->towel1);
	al_destroy_bitmap(data->towel2);
	al_destroy_bitmap(data->canvas);
	al_destroy_bitmap(data->tmp);
	DestroyCharacter(game, data->guy);
	free(data);
}

void Gamestate_Start(struct Game *game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	data->counter = 0;
	for (int i=0; i<5; i++) {
		data->people[i].satisfied = false;
		// TODO: set x and y
	}
	data->power = 0;
	data->left = 0;
	data->preparing = false;
	data->throwing = false;
	data->throwx = 0;
	data->score = 0;
	data->sandx = 0;
	data->seax = 0;
	data->maxsea = 0;
	data->sandleft = 0;
	data->seain = false;
	SetCharacterPosition(game, data->guy, 23, 47, 0);
	SelectSpritesheet(game, data->guy, "walk");
}

void Gamestate_Stop(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

void Gamestate_Pause(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets paused (so only Draw is being called, no Logic not ProcessEvent)
	// Pause your timers here.
}

void Gamestate_Resume(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets resumed. Resume your timers here.
}

// Ignore this for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct GamestateResources* data) {}
