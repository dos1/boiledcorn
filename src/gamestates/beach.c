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
#include <math.h>
#include <libsuperderpy.h>

struct GamestateResources {
		// This struct is for every resource allocated and used by your gamestate.
		// It gets created on load and then gets passed around to all other function calls.
		ALLEGRO_FONT *font;

		bool started;
		bool started_once;
		int frames;
		int counter;
		ALLEGRO_BITMAP *boy, *cloud, *girl, *lost, *off, *on, *overlay, *sand, *sea, *corn, *pow;
		ALLEGRO_BITMAP *towels[3];
		ALLEGRO_BITMAP *canvas, *tmp;
		struct {
				int x, y;
				bool satisfied;
				ALLEGRO_BITMAP *human, *towel;
		} people[6];
		int power;
		int left;
		bool preparing;
		bool throwing;
		float throwx;
		int throwy;
		int target;
		int score;
		int sandx;
		int seax;
		int seay;
		int maxsea;
		int sandleft;
		bool seain;
		struct Character *guy;

		ALLEGRO_AUDIO_STREAM *seanoise, *music;
		ALLEGRO_SAMPLE *win_sample, *lose_sample, *throw_sample, *corn_sample[3];
		ALLEGRO_SAMPLE_INSTANCE *win, *lose, *thr, *boiledcorn[3];

};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	data->frames++;
	data->counter++;
	if (data->preparing) {
		data->power++;
		if (data->power > 32) {
			data->power = 32;
		}
	}
	if (data->throwing) {
		data->throwx += 2;
		if (data->throwx >= data->target) {
			data->throwing = false;
			data->preparing = false;
			data->left--;

			bool fine = false;
			for (int i=0; i<6; i++) {
				int x = data->throwx;
				int y = data->throwy;
				int x1 = data->people[i].x, y1 = data->people[i].y;
				int x2 = x1 + al_get_bitmap_width(data->people[i].towel), y2 = y1 + al_get_bitmap_height(data->people[i].towel);

				fine = ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));
				if (fine && !data->people[i].satisfied) {
					data->people[i].satisfied = true;
					break;
				}
			}
			if (!fine) {
				al_set_target_bitmap(data->canvas);
				al_draw_bitmap(data->lost, data->throwx, data->throwy - 3, 0);
				al_set_target_backbuffer(game->display);
				data->score--;
				al_play_sample_instance(data->lose);
			} else {
				al_play_sample_instance(data->win);
				data->score++;
			}

			if (data->left == 0) {
				al_set_audio_stream_playing(data->music, false);
				data->started = false;
			}
		}
	}

	if ((data->started) && (data->counter >= 8)) {
		data->counter = 0;
		AnimateCharacter(game, data->guy, 1);
		data->seay++;

		al_set_target_bitmap(data->tmp);
		al_clear_to_color(al_map_rgba(0,0,0,0));
		al_draw_bitmap(data->canvas, 0, 0, 0);
		al_set_target_bitmap(data->canvas);
		al_clear_to_color(al_map_rgba(0,0,0,0));
		al_draw_bitmap(data->tmp, 0, 1, 0);
		al_set_target_backbuffer(game->display);

		if (data->throwing) {
			data->throwy++;
		}

		for (int i=0; i<6; i++) {
			data->people[i].y++;
			if (data->people[i].y > 95) {
				data->people[i].y = -20 + (rand() % 5);
				data->people[i].human = (rand() % 2) ? data->boy : data->girl;
				data->people[i].towel = data->towels[rand() % 3];
				data->people[i].satisfied = false;
				if (rand() % 10 == 0) {
					data->people[i].satisfied = true;
				}
				if ((i!=0) && (i!=5)) {
					data->people[i].x = (rand() % 100) + 45;
				}
			}
		}
	}
	if (data->seay == 120) {
		data->seay = 0;
		al_play_sample_instance(data->boiledcorn[rand() % 3]);
	}
	data->seax = fabs(sin(data->frames / 64.0)) * 16;
	if (data->seax == 15) {
		data->maxsea = rand() % 7;
	}
	data->seax = fmax(data->seax, data->maxsea);
	if (data->seax == data->maxsea) {
		data->sandx = data->maxsea;
		data->sandleft = 255;
	}
	data->sandleft-=2;
	if (data->sandleft < 0) {
		data->sandleft = 0;
	}
}

void Gamestate_Draw(struct Game *game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.
	al_clear_to_color(al_map_rgb(255, 234, 206));
	al_draw_tinted_bitmap(data->sand, al_map_rgba(data->sandleft, data->sandleft, data->sandleft, data->sandleft), -data->sandx, data->seay, 0);
	al_draw_bitmap(data->sea, -data->seax, data->seay, 0);
	al_draw_tinted_bitmap(data->sand, al_map_rgba(data->sandleft, data->sandleft, data->sandleft, data->sandleft), -data->sandx, data->seay - 120, 0);
	al_draw_bitmap(data->sea, -data->seax, data->seay - 120, 0);

//	for (int i=0; i<10; i++) {
	al_draw_bitmap(data->overlay, 0, data->seay - 120, 0);
	al_draw_bitmap(data->overlay, 0, data->seay, 0);
//	}

	DrawCharacter(game, data->guy, al_map_rgb(255,255,255), 0);

	for (int i=0; i<6; i++) {
		al_draw_bitmap(data->people[i].towel, data->people[i].x, data->people[i].y, 0);
		al_draw_bitmap(data->people[i].human, data->people[i].x+5, data->people[i].y+3, 0);
	}

	al_draw_bitmap(data->canvas, 0, 0, 0);

	for (int i=0; i<6; i++) {
		if ((!data->people[i].satisfied) && (data->people[i].y > 13)) {
			al_draw_bitmap(data->cloud, data->people[i].x - 1, data->people[i].y - 13, 0);
		}
	}


	if (data->started_once) {
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1+1, 2+1, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1-1, 2-1, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1+1, 2-1, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1-1, 2+1, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1-1, 2, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1+1, 2, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1, 2-1, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(99,99,99), 160-1, 2+1, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
		al_draw_textf(data->font, al_map_rgb(255,255,255), 160-1, 2, ALLEGRO_ALIGN_RIGHT, "%d", data->score);
	}

	if (data->throwing) {
		al_draw_bitmap(data->corn, data->throwx, data->throwy, 0);
	}

	if (data->started) {
		al_draw_filled_rectangle(0, 85, 160, 90, al_map_rgba(0,0,0,128));


		if ((data->preparing) || (data->throwing)) {
			al_draw_bitmap(data->pow, 0, 80, 0);
			al_draw_bitmap(data->off, 5*data->power, 80, 0);
		} else {
			al_draw_bitmap(data->on, 0, 80, 0);
			al_draw_bitmap(data->off, 5*data->left, 80, 0);
		}

	} else {
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2+1, 90/2 - 12 + 1, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2+1, 90/2 + 6 + 1, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2-1, 90/2 - 12 - 1, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2-1, 90/2 + 6 - 1, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");

		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2+1, 90/2 - 12 - 1, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2+1, 90/2 + 6 - 1, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");

		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2-1, 90/2 - 12 + 1, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2-1, 90/2 + 6 + 1, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");

		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2-1, 90/2 - 12, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2-1, 90/2 + 6, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2+1, 90/2 - 12, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2+1, 90/2 + 6, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");

		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2, 90/2 - 12 + 1, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2, 90/2 + 6 + 1, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2, 90/2 - 12 - 1, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(0,0,0), 160/2, 90/2 + 6 - 1, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");

		al_draw_text(data->font, al_map_rgb(255,255,255), 160/2, 90/2 - 12, ALLEGRO_ALIGN_CENTER, "BOILED CORN");
		al_draw_text(data->font, al_map_rgb(255,255,255), 160/2, 90/2 + 6, ALLEGRO_ALIGN_CENTER, "Press SPACE to corn");
	}
}

void Gamestate_ProcessEvent(struct Game *game, struct GamestateResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_SPACE)) {
		if (!data->started) {
			al_rewind_audio_stream(data->music);
			al_set_audio_stream_playing(data->music, true);
			data->started = true;
			if (data->started_once) {
				for (int i=0; i<6; i++) {
					data->people[i].satisfied = true;
					data->people[i].x = (rand() % 100) + 45;
					data->people[i].y = 90 - 22 * i + (rand() % 5);
					data->people[i].human = (rand() % 2) ? data->boy : data->girl;
					data->people[i].towel = data->towels[rand() % 3];
				}
				data->people[0].x = 65;
				data->people[5].x = 110;
				data->people[4].satisfied = false;
				data->people[5].satisfied = false;

				al_set_target_bitmap(data->canvas);
				al_clear_to_color(al_map_rgba(0,0,0,0));
				al_set_target_backbuffer(game->display);
			}
			data->started_once = true;
			data->score = 0;
			data->left = 32;
			al_play_sample_instance(data->boiledcorn[rand() % 3]);
			SelectSpritesheet(game, data->guy, "walk");
		} else {
			if (!data->throwing) {
				data->preparing = true;
				data->power = 0;
			}
		}
	}

	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_SPACE)) {
		if (data->preparing) {
			data->preparing = false;
			data->throwing = true;
			al_play_sample_instance(data->thr);
			data->throwx = data->guy->x * 160 + 10;
			data->throwy = data->guy->y * 90 + 5;
			data->target = data->guy->x + 5*data->power;
		}
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
	data->pow = al_load_bitmap(GetDataFilePath(game, "power.png"));
	data->overlay = al_load_bitmap(GetDataFilePath(game, "overlay.png"));
	data->sand = al_load_bitmap(GetDataFilePath(game, "sand.png"));
	data->sea = al_load_bitmap(GetDataFilePath(game, "sea.png"));
	data->corn = al_load_bitmap(GetDataFilePath(game, "corn.png"));
	data->towels[0] = al_load_bitmap(GetDataFilePath(game, "towel1.png"));
	data->towels[1] = al_load_bitmap(GetDataFilePath(game, "towel2.png"));
	data->towels[2] = al_load_bitmap(GetDataFilePath(game, "towel3.png"));

	data->canvas = al_create_bitmap(160, 90);
	al_set_target_bitmap(data->canvas);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_set_target_backbuffer(game->display);

	data->tmp = al_create_bitmap(160, 90);

	data->guy = CreateCharacter(game, "guy");
	RegisterSpritesheet(game, data->guy, "stand");
	RegisterSpritesheet(game, data->guy, "walk");
	LoadSpritesheets(game, data->guy);

	data->seanoise = al_load_audio_stream(GetDataFilePath(game, "sea.flac"), 4, 1024);
	al_attach_audio_stream_to_mixer(data->seanoise, game->audio.fx);
	al_set_audio_stream_playmode(data->seanoise, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_playing(data->seanoise, false);
	data->music = al_load_audio_stream(GetDataFilePath(game, "music.flac"), 4, 1024);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);
	al_set_audio_stream_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_playing(data->music, false);

	data->win_sample = al_load_sample(GetDataFilePath(game, "point.flac"));
	data->win = al_create_sample_instance(data->win_sample);
	al_attach_sample_instance_to_mixer(data->win, game->audio.fx);

	data->lose_sample = al_load_sample(GetDataFilePath(game, "fail.flac"));
	data->lose = al_create_sample_instance(data->lose_sample);
	al_attach_sample_instance_to_mixer(data->lose, game->audio.fx);
	al_set_sample_instance_gain(data->lose, 1.5);

	data->throw_sample = al_load_sample(GetDataFilePath(game, "throw.flac"));
	data->thr = al_create_sample_instance(data->throw_sample);
	al_attach_sample_instance_to_mixer(data->thr, game->audio.fx);

	data->corn_sample[0] = al_load_sample(GetDataFilePath(game, "corn1.flac"));
	data->boiledcorn[0] = al_create_sample_instance(data->corn_sample[0]);
	al_attach_sample_instance_to_mixer(data->boiledcorn[0], game->audio.voice);

	data->corn_sample[1] = al_load_sample(GetDataFilePath(game, "corn2.flac"));
	data->boiledcorn[1] = al_create_sample_instance(data->corn_sample[1]);
	al_attach_sample_instance_to_mixer(data->boiledcorn[1], game->audio.voice);

	data->corn_sample[2] = al_load_sample(GetDataFilePath(game, "corn3.flac"));
	data->boiledcorn[2] = al_create_sample_instance(data->corn_sample[2]);
	al_attach_sample_instance_to_mixer(data->boiledcorn[2], game->audio.voice);

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
	al_destroy_bitmap(data->pow);
	al_destroy_bitmap(data->overlay);
	al_destroy_bitmap(data->sand);
	al_destroy_bitmap(data->sea);
	al_destroy_bitmap(data->corn);
	al_destroy_bitmap(data->towels[0]);
	al_destroy_bitmap(data->towels[1]);
	al_destroy_bitmap(data->towels[2]);
	al_destroy_bitmap(data->canvas);
	al_destroy_bitmap(data->tmp);
	DestroyCharacter(game, data->guy);
	al_destroy_audio_stream(data->seanoise);
	al_destroy_audio_stream(data->music);
	al_destroy_sample_instance(data->win);
	al_destroy_sample_instance(data->lose);
	al_destroy_sample_instance(data->thr);
	al_destroy_sample(data->win_sample);
	al_destroy_sample(data->lose_sample);
	al_destroy_sample(data->throw_sample);
	for (int i=0; i<3; i++) {
		al_destroy_sample_instance(data->boiledcorn[i]);
		al_destroy_sample(data->corn_sample[i]);
	}
	free(data);
}

void Gamestate_Start(struct Game *game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	data->counter = 0;
	for (int i=0; i<6; i++) {
		data->people[i].satisfied = true;
		data->people[i].x = (rand() % 100) + 45;
		data->people[i].y = 90 - 22 * i + (rand() % 5);
		data->people[i].human = (rand() % 2) ? data->boy : data->girl;
		data->people[i].towel = data->towels[rand() % 3];
	}
	data->people[0].x = 65;
	data->people[5].x = 110;
	data->people[4].satisfied = false;
	data->people[5].satisfied = false;
	data->power = 0;
	data->left = 0;
	data->preparing = false;
	data->throwing = false;
	data->throwx = 0;
	data->seay = 0;
	data->score = -1;
	data->sandx = 0;
	data->seax = 0;
	data->maxsea = 0;
	data->sandleft = 0;
	data->seain = false;
	data->started = false;
	data->started_once = false;
	SetCharacterPosition(game, data->guy, 27, 42, 0);
	SelectSpritesheet(game, data->guy, "stand");

	al_play_sample_instance(data->boiledcorn[rand() % 3]);
	al_set_audio_stream_playing(data->seanoise, true);
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
