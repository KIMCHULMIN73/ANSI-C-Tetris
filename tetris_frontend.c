/******************************************
 **                                      **
 **          Tetris on NCURSES           **
 **                                      **
 ******************************************
 **          tetris_frontend.c           **
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
extern char title[TITLE_HEIGHT][TITLE_WIDTH];

/* initialize NCURSES to use */
enum result  init_curses(int *xmax, int *ymax)
{
    initscr();        // init NCURSES lib.
    cbreak();         // no-wait mode for console(keypad) input
    noecho();         // no-echo mode for console(keypad) input
    curs_set(0);      // erase curson from screen
    start_color();    // init color functions and parameters

    init_pair(1, COLOR_RED, COLOR_BLACK);        // set color pair(character, background) into each number
    init_pair(2, COLOR_GREEN, COLOR_BLACK); 
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK); 
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_BLACK);

    getmaxyx(stdscr, *ymax, *xmax);    // get size of standard screen
    keypad(stdscr, TRUE);              // init to get keypad input from console
}


/* display intro screen */
enum result intro_game(void)
{
    int ch;
    int pos_x, pos_y;
    char *p_msg;

    timeout(-1);     // timeout of console(standard screen) input function (msec) : infinite delay for negative value

    draw_title();    // draw title 'TETRIS'

INTRO :
    pos_x = XWIN_OFFSET-2;
    pos_y = YWIN_OFFSET;
    p_msg = "========================================================================";
    draw_message(pos_x, pos_y, p_msg, nop);
    p_msg = "||                                                                    ||";
    draw_message(pos_x, pos_y+1, p_msg, nop);
    p_msg = "||          Press '1' for 1-Player or '2' for 2-Players Game          ||";
    draw_message(pos_x, pos_y+2, p_msg, nop);  
    p_msg = "||                                                                    ||";
    draw_message(pos_x, pos_y+3, p_msg, nop);
    p_msg = "||                or If you wanna Game Manual, Press 'H'              ||";
    draw_message(pos_x, pos_y+4, p_msg, nop);    
    p_msg = "||                                                                    ||";
    draw_message(pos_x, pos_y+5, p_msg, nop);
    p_msg = "========================================================================";
    draw_message(pos_x, pos_y+6, p_msg, nop);

    ch = getch();    // wait during infinite delay to get user input, becase of timeout(-1)  
    if (ch == 'H' || ch == 'h') { display_manpage(); goto INTRO; }
    else if (ch == '1') total_players = 1;
    else if (ch == '2') total_players = 2;
    else goto INTRO;

    p_msg = "                                                                        ";
    draw_message(pos_x, pos_y, p_msg, nop);
    draw_message(pos_x, pos_y+1, p_msg, nop);
    draw_message(pos_x, pos_y+2, p_msg, nop);
    draw_message(pos_x, pos_y+3, p_msg, nop);
    draw_message(pos_x, pos_y+4, p_msg, nop);
    draw_message(pos_x, pos_y+5, p_msg, nop);
    draw_message(pos_x, pos_y+6, p_msg, nop);

    timeout(5);    // timeout of console(standard screen) input function (for xx msec)

    return success;  
}


/*display game manual */
enum result display_manpage(void)
{
    int ch;
    int pos_x, pos_y;
    char *p_msg;

    //timeout(-1);     // timeout of console(standard screen) input function (msec) : infinite delay for negative value

    pos_x = XWIN_OFFSET/2;
    pos_y = YWIN_OFFSET;

    p_msg = "*******************************************************************************";
    draw_message(pos_x, pos_y, p_msg, nop);
    p_msg = "**                               GAME   MANUAL                               **";
    draw_message(pos_x, pos_y+1, p_msg, nop);
    p_msg = "*******************************************************************************";;
    draw_message(pos_x, pos_y+2, p_msg, nop);
    p_msg = "**  PLAYER 1 : [F]left, [H]right, [G]down, [T]rotate, [A]drop, [S]use item   **";
    draw_message(pos_x, pos_y+3, p_msg, nop);
    p_msg = "**             [F1]cheat key , [P]pause                                      **";
    draw_message(pos_x, pos_y+4, p_msg, nop);  
    p_msg = "**                                                                           **";
    draw_message(pos_x, pos_y+5, p_msg, nop);  
    p_msg = "**  PLAYER 2 : [ARROW KEYS]left, right, down, rotate, [+]drop, [BS]use item  **";
    draw_message(pos_x, pos_y+6, p_msg, nop);    
    p_msg = "**             [F12]cheat key , [P]pause                                     **";
    draw_message(pos_x, pos_y+7, p_msg, nop);  
    p_msg = "*******************************************************************************";;
    draw_message(pos_x, pos_y+8, p_msg, nop);
    p_msg = "                  To quit from manual page, Press Any Key                      ";
    draw_message(pos_x, pos_y+10, p_msg, nop);

    ch = getch();    // wait during infinite delay to get user input, becase of timeout(-1)  

    p_msg = "                                                                                ";
    draw_message(pos_x, pos_y, p_msg, nop);
    draw_message(pos_x, pos_y+1, p_msg, nop);
    draw_message(pos_x, pos_y+2, p_msg, nop);
    draw_message(pos_x, pos_y+3, p_msg, nop);
    draw_message(pos_x, pos_y+4, p_msg, nop);  
    draw_message(pos_x, pos_y+5, p_msg, nop);
    draw_message(pos_x, pos_y+6, p_msg, nop);
    draw_message(pos_x, pos_y+7, p_msg, nop);  
    draw_message(pos_x, pos_y+8, p_msg, nop);
    draw_message(pos_x, pos_y+10, p_msg, nop);  

    //timeout(5);    // timeout of console(standard screen) input function (for xx msec)

    return success;  
}

/* initialize game windows */
enum result init_window(enum gameid gid)
{
    struct gameparam *game;

    game = parameter_db(gid);

        switch(gid)
        {
            case game1 : game->wnd_x = XWIN_OFFSET;
                         game->wnd_y = YWIN_OFFSET;
                         game->wnd_width = BOARD_WIDTH * 2;
                         game->wnd_height = BOARD_HEIGHT;
                         break;

            case game2 : game->wnd_x = (XWIN_OFFSET + BOARD_WIDTH) * 2;
                         game->wnd_y = YWIN_OFFSET;
                         game->wnd_width = BOARD_WIDTH * 2;
                         game->wnd_height = BOARD_HEIGHT;
                         break;

            case game3 :
            case game4 :
            default    : break;
        }

    game->wnd = create_newwin(game->wnd_height, game->wnd_width, game->wnd_y, game->wnd_x);

    //keypad(game->wnd, TRUE);      // turn on the keypad-input of each game window
    //wtimeout(game->wnd, 10);      // timeout of console(each game window) input function (msec)

    return success;
}

/* draw title */
enum result draw_title(void)
{
    int i, j;
    char *p_str = "CopyLeft(L) by Kim chulmin, 2024.10";

    for( j = 0 ; j < TITLE_HEIGHT ; j++)
        for( i = 0 ; i < TITLE_WIDTH ; i++)
        {
            attron(COLOR_PAIR((j+i) % 8)|A_BOLD);
            if( title[j][i] != 0 )
                mvaddch(j + 1, (i + XWIN_OFFSET*2), ACS_CKBOARD);
        }

    attron(COLOR_PAIR(3)|A_BOLD);
    mvprintw(j + 2 , (i + XWIN_OFFSET*2) - strlen(p_str), p_str);

    refresh();

    return success;
}


/* draw game information */
enum result display_info(int pos_x, int pos_y, enum gameid gid)
{
    struct gameparam *game;  
    char scr[DIGIT_OF_NUM], item[DIGIT_OF_NUM], hi_scr[DIGIT_OF_NUM], stg[DIGIT_OF_NUM], ntet[DIGIT_OF_NUM], nline[DIGIT_OF_NUM];
    char *p_str;

    game = parameter_db(gid);

    int2str(game->score, scr);
    int2str(game->bonus_item, item); 
    int2str(game->highscore, hi_scr);
    int2str(game->stage, stg);  
    int2str(game->num_of_tet, ntet);
    int2str(game->line_cnt, nline);

    switch(gid)
    {
        case game1 : p_str = "PLAYER 1 : %d";
                     break;
        case game2 : p_str = "PLAYER 2 : %d";
                     break;
        case game3 : p_str = "PLAYER 3 : %d";
                     break;
        case game4 : p_str = "PLAYER 4 : %d";
                     break;
        default :    p_str = "Nobody";
                     break;
    }

    attron(COLOR_PAIR(7)|A_BOLD);
    mvprintw(pos_y - BOARD_HEIGHT - TITLE_HEIGHT - 1 , pos_x, p_str, game->win_count);

    p_str = "SCORE : ";
    attron(COLOR_PAIR(2)|A_BOLD); 
    mvaddstr(pos_y, pos_x, p_str);
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, "          ");
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, scr);

    p_str = "ITEM : ";
    mvaddstr(++pos_y, pos_x, p_str);
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, "          ");
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, item);

    p_str = "HI-SCORE : ";
    mvaddstr(++pos_y, pos_x, p_str);
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, "          ");
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, hi_scr);

    p_str = "STAGE : ";
    mvaddstr(++pos_y, pos_x, p_str);
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, "          ");
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, stg);

    p_str = "NUM-TET : ";
    mvaddstr(++pos_y, pos_x, p_str);
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, "          ");
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, ntet);

    p_str = "LINE-CLR : ";
    mvaddstr(++pos_y, pos_x, p_str);
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, "          ");
    mvaddstr(pos_y, pos_x+strlen(p_str)+1, nline);

    refresh();

    return success;
}


/* display game board */
enum result display_game_board(enum gameid gid)
{
    int i, j;
    int pos_x, pos_y;
    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;

    game = parameter_db(gid);
    bd = get_board(gid);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    /* show current game window */  
    for(j = 0 ; j < BOARD_HEIGHT ; j++)
        for(i = 0 ; i < BOARD_WIDTH ; i++)
            if(bd[j][i] == EDGE)
            {
                wattron(game->wnd, COLOR_PAIR(7)|A_BOLD);
                mvwaddch(game->wnd, j, (i*2), ACS_LANTERN);
                mvwaddch(game->wnd, j, (i*2) + 1, ACS_LANTERN);
            }
            else
            {
                if(bd[j][i] != 0)
                {
                    wattron(game->wnd, COLOR_PAIR(bd[j][i])|A_BOLD);          
                    mvwaddch(game->wnd, j, (i*2), ACS_CKBOARD);
                    mvwaddch(game->wnd, j, (i*2) + 1, ACS_CKBOARD);
                }
                else
                {
                    mvwaddch(game->wnd, j, (i*2), ACS_SPACE);
                    mvwaddch(game->wnd, j, (i*2) + 1, ACS_SPACE);
                }
            }

    wrefresh(game->wnd);

    /* show game information(score, items, stage... etc.) */
    switch(gid)
    {
        case game1 : pos_x = XWIN_OFFSET;
                     pos_y = YWIN_OFFSET + BOARD_HEIGHT;
                     break;
        case game2 : pos_x = (XWIN_OFFSET + BOARD_WIDTH) * 2;
                     pos_y = YWIN_OFFSET + BOARD_HEIGHT;
                     break;
        case game3 :
        case game4 :
        default    : pos_x = pos_y = 0;
                     break;
    }

    display_info(pos_x, pos_y, gid);

    return success;
}

/*display gameover information */
enum result display_ending(int mode, enum gameid gid)
{
    char *p_msg;
    int pos_x, pos_y;

    if(mode == DISP_GAMEOVER)
    {
        p_msg = "GAME OVER";
        pos_x = (BOARD_WIDTH - strlen(p_msg)/2);
        pos_y = (BOARD_HEIGHT/2);
        draw_message(pos_x, pos_y, p_msg, gid);
    }
    else if(mode == DISP_NEXTGAME)
    {
        p_msg = "=====================================";
        pos_x = XWIN_OFFSET*2;
        pos_y = YWIN_OFFSET;
        draw_message(pos_x, pos_y, p_msg, nop);
        p_msg = "==                                 ==";
        draw_message(pos_x, pos_y+1, p_msg, nop);
        p_msg = "==  [F4]Continue [F5]New [F6]Exit  ==";
        draw_message(pos_x, pos_y+2, p_msg, nop);
        p_msg = "==                                 ==";
        draw_message(pos_x, pos_y+3, p_msg, nop);    
        p_msg = "=====================================";
        draw_message(pos_x, pos_y+4, p_msg, nop);
    }
    else if(mode == DISP_CLEAR)
    {
        p_msg = "                                     ";
        pos_x = XWIN_OFFSET*2;
        pos_y = YWIN_OFFSET;
        draw_message(pos_x, pos_y, p_msg, nop);
        draw_message(pos_x, pos_y+1, p_msg, nop);
        draw_message(pos_x, pos_y+2, p_msg, nop);
        draw_message(pos_x, pos_y+3, p_msg, nop);    
        draw_message(pos_x, pos_y+4, p_msg, nop);
    }
    else if(mode == DISP_WINNER)
    {
        p_msg = "==========";
        pos_x = (BOARD_WIDTH - strlen(p_msg)/2);
        pos_y = (BOARD_HEIGHT/2) - 4;
        draw_message(pos_x, pos_y, p_msg, gid);
        p_msg = "= WINNER =";
        draw_message(pos_x, pos_y+1, p_msg, gid);
        p_msg = "==========";
        draw_message(pos_x, pos_y+2, p_msg, gid);    
    }

    return success;
}


/* show(& determine) next tetromino */
int show_next_tetromino(int pos_x, int pos_y, enum gameid gid)
{
    int i, j;
    static int random[TOTAL_PLAYER];
    char (*ptr_tetromino)[4][4][4] = NULL;

    random[gid] = rand() % 7;

    ptr_tetromino = get_tetromino(random[gid]);

    /* show next tetromino */
    mvprintw(pos_y, pos_x,  "Next Tetromino : ");

    for(j = 0 ; j < 4 ; j++)
        for(i = 0 ; i < 4 ; i++)
            if((*ptr_tetromino)[DEG_90][j][i] != 0)
            {
                attron(COLOR_PAIR((*ptr_tetromino)[DEG_90][j][i])|A_BOLD);
                mvaddch(j + pos_y + 1 , (i*2) + pos_x, ACS_CKBOARD);
                mvaddch(j + pos_y + 1 , (i*2) + 1 + pos_x, ACS_CKBOARD);
            }
            else
            {
                mvaddch(j + pos_y + 1 , (i*2) + pos_x, ACS_SPACE);
                mvaddch(j + pos_y + 1 , (i*2) + 1 + pos_x, ACS_SPACE);
            }

    return random[gid];
}

/* draw any message onto screen */
enum result draw_message(int pos_x, int pos_y, char *msg, enum gameid gid)
{
    struct gameparam *game;
    game = parameter_db(gid);

    if ( gid != nop )
    {
        wattron(game->wnd, COLOR_PAIR(7)|A_BOLD);
        mvwprintw(game->wnd, pos_y , pos_x, msg);
        wrefresh(game->wnd);
    }
    else
    {
        attron(COLOR_PAIR(7)|A_BOLD);
        mvprintw(pos_y , pos_x, msg);
        refresh();
    }

    return success;
}

/* create independent window on standard screen */
WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);

    //box(local_win, 0, 0);
    //wrefresh(local_win);

    return local_win;
}

/* delete window on standard screen */
enum result  destroy_win(WINDOW *local_win)
{
    //wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    //wrefresh(local_win);
    delwin(local_win);
}
