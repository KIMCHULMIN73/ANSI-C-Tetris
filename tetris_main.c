/******************************************
 **                                      **
 **          Tetris on NCURSES           **
 **                                      **
 ******************************************
 **            tetris_main.c             **
 ******************************************
 **   CopyLeft by kimchulmin, 2024.10    **
 ******************************************/

#include "tetris.h"

/* reference of extern variables */
extern enum flag on_game;
extern enum flag game_status[TOTAL_PLAYER];
extern int total_players;
extern int exception_code;
extern int exception_data[10];
extern char i_tetromino[4][4][4];
extern char o_tetromino[4][4][4];
extern char t_tetromino[4][4][4];
extern char j_tetromino[4][4][4];
extern char l_tetromino[4][4][4];
extern char s_tetromino[4][4][4];
extern char z_tetromino[4][4][4];


/* main(start point of C) of Tetris process */
enum result main(int argc, char const *argv[])
{
  int scr_width, scr_height;    // size of standard screen(linux terminal/console)

  /* initialize Ncurses to use */
  init_curses(&scr_width, &scr_height);

  if( scr_width < SCR_MIN_WIDTH || scr_height < SCR_MIN_HEIGHT ) {
    exception_code = _SMALL_SIZE_SCREEN;
    exception_data[0] = scr_width;
    exception_data[1] = scr_height;
    goto EXIT;
  }
  else refresh();    // update standard screen of Ncurses

  /* initialize Ncurses to use */
  intro_game();
  
  /* turn on the all status of games */
  on_game = on;
  game_status[game1] = on;
  if(total_players == 2)
    game_status[game2] = on;
  else 
    game_status[game2] = off;

  /* initialize each tetris game */
  init_game(game1, newmode);
  if(total_players == 2)
    init_game(game2, newmode);

  /* infinite loop to hold on main process not to ends, but to executes 'gametask' callback function */
  while(on_game)
    ;    // NULL

  /* deinitialize each tetris game */
  deinit_game(game1);
  if(total_players == 2)
    deinit_game(game2);
 
  exception_code = _NORMAL_PROCESS_END;
  
EXIT :
  /* exceptionhandler to exit game */
  exceptionhandler(exception_code);

  if (exception_code == _NORMAL_PROCESS_END) return success;
  else return fail;
}


/* handling exception situation */
void exceptionhandler(int code)
{
  char *pmsg;

  init_callback(off, 0);    // deinitialize callback signal(timer) & handler
  endwin();                 // deinitialize ncurses to go back normal console control
  system("clear");

  switch (code) {
    case _NORMAL_PROCESS_END	: pmsg = "\n\nTETRIS GAME is OVER. Bye~! \n\n";
				  break;

    case _SMALL_SIZE_SCREEN	: pmsg = "\n\nScreen size is too small to run TETRIS GAME.\n\n";
                                  printf("\nMIN_X : %d , MIN_Y : %d\n", SCR_MIN_WIDTH, SCR_MIN_HEIGHT);
                                  printf("\nreal_x : %d , real_y : %d\n", exception_data[0], exception_data[1]);
				  break;

    default			: pmsg = "\n\nApplication is halted by unkown cause\n\n";
				  break;
  }

  printf("%s", pmsg);
}
