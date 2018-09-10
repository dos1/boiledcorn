/*! \file main.c
 *  \brief Main file of Super Examples.
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

#include "common.h"
#include "defines.h"
#include <libsuperderpy.h>
#include <signal.h>
#include <stdio.h>

static _Noreturn void derp(int sig) {
	write(STDERR_FILENO, "Segmentation fault\nI just don't know what went wrong!\n", 54);
	abort();
}

int main(int argc, char** argv) {
	signal(SIGSEGV, derp);

	srand(time(NULL));

	al_set_org_name("dosowisko.net");
	al_set_app_name(LIBSUPERDERPY_GAMENAME_PRETTY);

	struct Game* game = libsuperderpy_init(argc, argv, LIBSUPERDERPY_GAMENAME, (struct Viewport){160, 90});
	if (!game) { return 1; }

	al_set_window_title(game->display, LIBSUPERDERPY_GAMENAME_PRETTY);

	game->handlers.event = &GlobalEventHandler;
	game->handlers.destroy = &DestroyGameData;

	StartGamestate(game, "dosowisko");

	game->data = CreateGameData(game);

	return libsuperderpy_run(game);
}
