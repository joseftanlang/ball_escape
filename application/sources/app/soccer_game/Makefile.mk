CFLAGS		+= -I./sources/app/soccer_game
CPPFLAGS	+= -I./sources/app/soccer_game

VPATH += sources/app/soccer_game

# CPP source files
SOURCES_CPP += sources/app/soccer_game/ar_striker.cpp
SOURCES_CPP += sources/app/soccer_game/ar_keeper.cpp
SOURCES_CPP += sources/app/soccer_game/ar_ball.cpp
SOURCES_CPP += sources/app/soccer_game/ar_entrance_game.cpp
SOURCES_CPP += sources/app/soccer_game/ar_goalPost.cpp
