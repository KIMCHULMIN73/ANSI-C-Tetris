/******************************************
 **                                      **
 **          Tetris on NCURSES           **
 **                                      **
 ******************************************
 **               tetris.h               **
 ******************************************
 **   CopyLeft by kimchulmin, 2024.10    **
 ******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <ncurses.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>


/* 7 symbols of one-sided tetrominos */
#define I_TETROMINO    0
#define O_TETROMINO    1
#define T_TETROMINO    2
#define J_TETROMINO    3
#define L_TETROMINO    4
#define S_TETROMINO    5
#define Z_TETROMINO    6


/* rotation degree */
#define DEG_0      0
#define DEG_90     1
#define DEG_180    2
#define DEG_270    3


/* virtual timer interval(speed) - msec) */
#define INTV_0    50000
#define INTV_1    40000
#define INTV_2    30000
#define INTV_3    20000
#define INTV_4    10000
#define INTV_5    5000


/* game table size */
#define PLAY_BD_HEIGHT    20                      // board height to play
#define PLAY_BD_WIDTH     10                      // board width to play
#define BOARD_HEIGHT      (PLAY_BD_HEIGHT + 1)    // board to play + 'bottom' edge(wall) ( 'up' edge is not needed)
#define BOARD_WIDTH       (PLAY_BD_WIDTH + 2)     // board to play + 'left/right' edge(wall)


/* minimal standard screen size to draw tetris game board */
#define SCR_MIN_HEIGHT    (PLAY_BD_HEIGHT * 2)
#define SCR_MIN_WIDTH     ((PLAY_BD_WIDTH * 2) * 4)


/* title size */
#define TITLE_HEIGHT    5     // height of 'TETRIS' title graphic
#define TITLE_WIDTH     40    // width of 'TETRIS' title graphic


/* position offset of game board(window) */
#define YWIN_OFFSET    (TITLE_HEIGHT * 3)    // y start position of game window (on standard screen) 
#define XWIN_OFFSET    10                    // x start position of game window (on standard screen)


/* etc */
#define TOTAL_PLAYER           2                      // maximum number of players
#define TOTAL_STAGE            6                      // maximum number of stages
#define SCORE_UNIT             10                     // can get 1 points scrore per every movedown
#define ONELINE_CLR_SCORE      100                    // can get 100 points score per every full-line 
#define STAGE_CLEAR_BASE       10                     // to clear stage, must erase number of STAGE_CLEAR_BASE lines
#define BONUS_BASE             1000                   // to get 1 bonus item, every 1000 point 
#define MAX_BONUS_ITEM         5                      // maximum number of bonus items
#define MAX_MOVE_RESOLUTION    7                      // max number of ticks, needed to move down tetromino
#define SPEED_UP_BASE          500                    // to adjust game level (to decrease 'move period' value, every SPEED_UP_BASE ticks)
#define BOTTOM_COUNT           MAX_MOVE_RESOLUTION    // maximum value of 'bottom_cnt' to count ticks after tetromino moved down bottom
#define DIGIT_OF_NUM           10                     // maximum number of digit of integer to convert 'sting to integer'
#define STRING_LEN             80                     // maximum lenth of string
#define DEAD_BLOCK             8                      // symbol of dead block from other player's game board.
#define EDGE                   9                      // symbol of edge around game board
#define ACS_SPACE              32                     // ASCII code of space bar
#define LINECLR_SIM            2                      // number of line which cleared simultaneously, to send blocks to other player
#define DISP_GAMEOVER          0                      // parameter value for display_ending()
#define DISP_NEXTGAME          1                      // parameter value for display_ending()
#define DISP_CLEAR             2                      // parameter value for display_ending()
#define DISP_WINNER            3                      // parameter value for display_ending()


/* exception code */
#define _NORMAL_PROCESS_END     0                      // total process is ended by normal way
#define _SMALL_SIZE_SCREEN      1                      // screen size is too small to draw game board


/* user enumeration types */
enum result {fail, success};
enum flag {off, on};
enum gameid {game1, game2, game3, game4, nop};
enum direction {up, down, left, right, rotate};               // 'up' is not used in tetris game but, reserved for other games
enum gamemode {newmode, continuemode};                        // to decide next game mode
enum heartbeat {beat0, beat1, beat2, beat3, beat4, beat5};    // to decide virtual timer interval(speed)


/* data structure of parameters for each game */
struct gameparam
{
    /* regarding game window */
    WINDOW *wnd;            // pointer of window(independent partial screen on standard screen) to draw game board
    int wnd_width;          // x-size of window
    int wnd_height;         // y-sixe of window
    int wnd_x;              // x-position of window
    int wnd_y;              // y-position of window

    /* regarding tetromino */
    int curr_tetromino;     // current one among 7 tetromino symbols
    int next_tetromino;     // next one among 7 tetromino symbols
    int rot_degree;         // rotation degree of curr_tetromino
    int tet_x;              // x-position of curr_tetromino
    int tet_y;              // y-position of curr_tetromino
    int key_in;             // key input of game window to move tetromino
    enum heartbeat tick;    // to save current heartbeat
    int num_of_tet;         // number of tetrominos those used in one-game
  
    /* regarding game record */
    int score;              // score of game
    int line_cnt;           // number of erase line by 'check_full_lint()'
    int bonus_score;        // bonus score to get bonus item
    int bonus_item;         // bonus item
    int highscore;          // hi-score of game
    int stage;              // stage of game
    int win_count;          // count of number of win
  
    /* regarding game speed & level */
    int tickcount;          // to count virtual timer ticks
    int move_resolution;    // a number of ticks, needed to move tetromino
    long speedup_count;     // to adjust game level (to decrease 'move_period' value, every SPEED_UP_BASE ticks)
    int bottom_cnt;         // maximum value of 'bottom_cnt' to count ticks after tetromino moved down bottom
};


/* frontend functions */
enum result init_curses(int *xmax, int *ymax);
enum result intro_game(void);
enum result display_manpage(void);
enum result init_window(enum gameid gid);
enum result draw_title(void);
enum result display_info(int pos_x, int pos_y, enum gameid gid);
enum result display_game_board(enum gameid gid);
enum result display_ending(int mode, enum gameid gid);
int show_next_tetromino(int pos_x, int pos_y, enum gameid gid);
enum result draw_message(int pos_x, int pos_y, char *msg, enum gameid gid);
WINDOW *create_newwin(int height, int width, int starty, int startx);
enum result destroy_win(WINDOW *local_win);


/* backend funcions */
enum result init_game(enum gameid gid, enum gamemode mod);
enum result deinit_game(enum gameid gid);
enum result init_param(enum gameid gid, enum gamemode mod);
enum result init_board(enum gameid gid);
enum result init_callback(enum flag mode, int timevalue);
enum result init_timer(enum gameid gid, enum flag mode);
enum result tetris_callback(int signum);
enum result gametask(enum gameid gid);
enum result keypad_hadler(enum gameid gid, enum flag gameover);
enum result move_tetromino(enum direction dir, enum gameid gid);
enum result drop_tetromino(enum gameid gid);
enum flag check_collision(enum direction dir, enum gameid gid);
int check_line_clear(enum gameid gid);
enum flag board_clear(enum gameid gid);
enum result use_bonus_item(enum gameid gid);
enum result send_lines(enum gameid gid, int curr_line_cnt);
enum result control_game_level(enum gameid gid);
enum result next_stage(enum gameid gid);
enum flag check_gameover(enum gameid gid);
enum result gameovertask(enum flag status);
enum result next_game(enum gamemode mod);
struct gameparam *parameter_db(enum gameid gid);
char (*get_board(enum gameid gid))[];
char (*get_tetromino(int tetromino))[4][4][4];
enum result int2str(int number, char* num_str);


/* etc. funcions */
void exceptionhandler(int code);
