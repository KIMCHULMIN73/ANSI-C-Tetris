/******************************************
 **                                      **
 **          Tetris on NCURSES           **
 **                                      **
 ******************************************
 **           tetris_backend.c           **
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

/* initialize tetris game */
enum result init_game(enum gameid gid, enum gamemode mod)
{
    struct gameparam *game;

    game = parameter_db(gid);    // get parameter handle

    draw_title();                // draw title 'TETRIS'

    if(game_status[gid])
    {
        init_window(gid);          // init game window
        init_param(gid, mod);      // init game parameters
        init_board(gid);           // init game board
        wrefresh(game->wnd);
    }

    init_timer(gid, on);         // initialize timer

    return success;
}

/* deinitialize tetris game */
enum result deinit_game(enum gameid gid)
{
    struct gameparam *game;

    game = parameter_db(gid);      // get parameter handle

    init_timer(gid, off);          // deinitialize timer

    destroy_win(game->wnd);        // destroy game window for game1

    return success;  
}


/* initialize game parameters of each player */
enum result init_param(enum gameid gid, enum gamemode mod)
{
    static enum flag status = off;
    struct gameparam *game;

    game = parameter_db(gid);

    if(mod == newmode) srand((unsigned)time(NULL));    // set random number seed with current time to determine next tetromino (in 'decice_next_tetromino()' function)  

    /* initialize game parameters */
    game->curr_tetromino = I_TETROMINO;
    game->next_tetromino = show_next_tetromino(game->wnd_x, game->wnd_y - TITLE_HEIGHT, gid);
    game->rot_degree = DEG_90;
    game->tet_x = (BOARD_WIDTH / 2) - 1;               // start tet_x point
    game->tet_y = 0;                                   // start tet_y point
    if(mod == newmode)
        game->tick = beat0;
    game->num_of_tet = 0;
    if(mod == newmode) game->bonus_item = 0;
    game->bonus_score = 0;
    game->tickcount = 0;
    game->bottom_cnt = 0;
    game->speedup_count = 0;
    game->move_resolution = MAX_MOVE_RESOLUTION;
    game->score = 0;
    game->line_cnt = 0;
    if(status == off)
    {
        game->highscore = 0;
        status = on;
    }
    if(mod == newmode)
        game->stage = 1;
    if(mod == newmode)
        game->win_count = 0;
    game->key_in = 0;

    return success;
}


/* initialize game board */
enum result init_board(enum gameid gid)
{
    int i, j;
    char (*bd)[BOARD_WIDTH];
    struct gameparam *game; 

    bd = get_board(gid);
    game = parameter_db(gid);

    /* initialize board - play area to '0'(empty) & edge area to '1' */
    for(j = 0 ; j < PLAY_BD_HEIGHT ; j++)         // 20 EA (from 0 to 19)
        for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++)    // 10 EA (from 1 to 10)
            bd[j][i] = 0;                             // play area to '0'(empty)
      
    for(i = 0 ; i < BOARD_WIDTH ; i++)            // 12 EA(from 0 to 11)
        bd[PLAY_BD_HEIGHT][i] = EDGE;               // bottom edge area to '9'
      
    for(j = 0 ; j < BOARD_HEIGHT ; j++)          // 21 EA(from 0 to 20)
    {
        bd[j][0] = EDGE;                            // left edge area to '9'
        bd[j][BOARD_WIDTH - 1] = EDGE;              // right edge area to '9'
    }

    game->num_of_tet = 0;

    return success;
}


/* initialize callback signal & timer */  
enum result init_callback(enum flag mode, int timevalue)
{
    static struct sigaction sig_act;
    static struct itimerval timer;

    if ( mode == on )
    {
        /* initialize callback function */
        memset(&sig_act, 0, sizeof (sig_act));            // set all parameters in 'sig_act' to zero
        sig_act.sa_handler = (void *)&tetris_callback;    // assign 'virtual timer' signal handler(callback function)
        sigaction(SIGVTALRM, &sig_act, NULL);             // register signal action as a 'virtual timer alarm' event

        /* initialize timer : game speed(callback cycle) */
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = timevalue;               // expire time (msec)
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = timevalue;            // interval time (msec)
        setitimer(ITIMER_VIRTUAL, &timer, NULL);          // config a virtual timer 
    }
    else
    {
        /*deinialize callback & timer */
        memset(&sig_act, 0, sizeof (sig_act));
        sig_act.sa_handler = (void *)NULL;
        sigaction(SIGVTALRM, &sig_act, NULL);

        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = 0;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 0;
        setitimer(ITIMER_VIRTUAL,&timer,NULL);
    }
}


/* to init tick timer & callback functon */
enum result init_timer(enum gameid gid, enum flag mode)
{
    int time_interval;
    struct gameparam *game;

    game = parameter_db(gid);  

    switch(game->tick)
    {
        beat5   : time_interval = INTV_5; break;    // the fastest configuration
        beat4   : time_interval = INTV_4; break;
        beat3   : time_interval = INTV_3; break;
        beat2   : time_interval = INTV_2; break;
        beat1   : time_interval = INTV_1; break;
        beat0   : 
        default : time_interval = INTV_0; break;    // the lowest (default) configuration 
    }

    init_callback(mode, time_interval);           // initialize callback signal & timer

    return success;
}


/* callback handler by virtual timer every xx msec */  
enum result tetris_callback(int signum)
{
    int pos_x, pos_y;
    char *p_msg;
    static enum flag disp_1 = off, disp_2 = off, status = off;

    if(game_status[game1])          // on-game process of player1
    {
        gametask(game1);
        disp_1 = off;
        status = off;
    }
    else if(disp_1 == off)          // off-game task of player1
    {
        display_ending(DISP_GAMEOVER, game1);
        disp_1 = on;
    }

    if(game_status[game2])         // on-game task of player2
    {
        gametask(game2);
        disp_2 = off;
        status = off;
    }
    else if(disp_2 == off)         // off-game task of player2
    {
        display_ending(DISP_GAMEOVER, game2);
        disp_2 = on;
    }

    if( !game_status[game1] && !game_status[game2] )    // task for all players are off their game
    {
        if (status == off)
        {
            gameovertask(off);
            status = on;
        }
        else
            gameovertask(on);
    }

    return success;
}

/* main task of tetris game - sub-callback fuction */  
enum result gametask(enum gameid gid)
{
    int curr_line_cnt;
    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];

    game = parameter_db(gid);
    bd = get_board(gid);

    /* handle keypad input */
    keypad_hadler(gid, off);

    /*current tetromino will move down, every moving priod(number of ticks, needed to move tetromino) */
    game->tickcount++;
    game->tickcount %= game->move_resolution;

    if(game->tickcount == game->move_resolution - 1)
    {
        if(game->score >= game->highscore)
        game->highscore = game->score;
        move_tetromino(down, gid);
    }

    /* to check whether tetrominos make one line, or not */
    curr_line_cnt = check_line_clear(gid);
    game->line_cnt += curr_line_cnt;

    /* If more than 3 lines are cleared simultaneously, palyer can send lines/blocks to other player */
    if( game_status[game1] && game_status[game2] && (curr_line_cnt >= LINECLR_SIM) )
        send_lines(gid, curr_line_cnt);

    /* to check whether player can go to next stage or not */
    //if ( (game->line_cnt >= STAGE_CLEAR_BASE) || (game->num_of_tet > 2 && check_collision(down, gid) && board_clear(gid)) )
    if ( (game->line_cnt >= STAGE_CLEAR_BASE) )
        next_stage(gid);

    /* to control game level(speed) */
    control_game_level(gid);

    /* to check current tetromino down to bottom to prepare next tetromino when current tetromino was down on bottom */
    if(check_collision(down, gid))
    {
        if(game->bottom_cnt == (BOTTOM_COUNT-1))
        {
            game->curr_tetromino = game->next_tetromino;
            game->next_tetromino = show_next_tetromino(game->wnd_x, game->wnd_y - TITLE_HEIGHT, gid);
            game->rot_degree = DEG_90;
            game->tet_x = (BOARD_WIDTH / 2 - 1);
            game->tet_y = 0;
            game->num_of_tet++;
            game->score += SCORE_UNIT;
        }
        game->bottom_cnt++;
        game->bottom_cnt %= BOTTOM_COUNT;
    }

    /* to check game over situation */
    game_status[gid] = check_gameover(gid);
    if( game_status[gid] == off )
        game->num_of_tet = 0;

    /* show game board into display from memory */
    display_game_board(gid);
    refresh();

    return success;
}

/* to handle keypad input */
enum result keypad_hadler(enum gameid gid, enum flag gameover)
{
    int key_in;
    static enum flag toggle = on;
    struct gameparam *game;

    /* to execute pause mode */    
    while(!toggle)
    {
        key_in = getch();
        switch(key_in)
        {
            case 'P'           :
            case 'p'           : toggle = !toggle; break;
        }  
    }

    if(toggle)
    {
        key_in = getch();

        switch(key_in)        /* key_in for game1(Left side of keypad) */
        {
            case 'D'      : 
            case 'd'      : move_tetromino(left, game1);
                            break;
            case 'G'      :
            case 'g'      : move_tetromino(right, game1);
                            break;
            case 'F'      :
            case 'f'      : move_tetromino(down, game1);
                            break;
            case 'R'      :
            case 'r'      : move_tetromino(rotate, game1);
                            break;
            case 'A'      :
            case 'a'      : drop_tetromino(game1);
                            break;
            case 'S'      :
            case 's'      : game = parameter_db(game1);
                            if(game->bonus_item > 0)
                                use_bonus_item(game1);
                            break;
            case KEY_F(1) : init_board(game1);     // erase all tetromino on Game1 board
                            break;

            /* key_in for game2(Right side of keypad) */
            case KEY_LEFT      : move_tetromino(left, game2);
                                 break;
            case KEY_RIGHT     : move_tetromino(right, game2);
                                 break;
            case KEY_DOWN      : move_tetromino(down, game2);
                                 break;
            case KEY_UP        : move_tetromino(rotate, game2);
                                 break;
            case ','           : 
            case '<'           : drop_tetromino(game2);
                                 break;
            case '.'           :
            case '>'           : game = parameter_db(game2);
                                 if(game->bonus_item > 0)
                                     use_bonus_item(game2);
                                 break;
            case KEY_F(12)     : init_board(game2);        // erase all tetromino on Game2 board
                                 break;

            /* to enter pause mode */
            case 'P'           :
            case 'p'           : toggle = !toggle;
                                 break;

            /* key_in for gameover task */
            case KEY_F(4)      : if (gameover == on)
                                     next_game(continuemode);
                                 break;
            case KEY_F(5)     : if (gameover == on)
                                    next_game(newmode);
                                break;
            case KEY_F(6)     : if (gameover == on)
                                    on_game = off;
                                break;

            default       : break;
        }

/* #### Not used below code, because of performance issue of RasberryPHY-3.5B ####

    struct gameparam *game;

    game = parameter_db(gid);

    game->key_in = wgetch(game->wnd);

    if (gid == game1)
        switch(game->key_in)
        {
            case KEY_LEFT      : move_tetromino(left, gid);
                                 break;
            case KEY_RIGHT     : move_tetromino(right, gid);
                                 break;
            case KEY_DOWN      : move_tetromino(down, gid);
                                 break;
            case KEY_UP        : move_tetromino(rotate, gid);
                                 break;
            case KEY_IC        : drop_tetromino(gid);
                                 break;
            case KEY_BACKSPACE : on_game = off;
                                 // if(game->bonus_item > 0) item_use(gid);
                                 break;
            case KEY_F(12)     : cheat_key(gid);
                                 break;
            case KEY_F(11)     : game_status[gid] = off;
                                 break;
            default            : break;
        }
        else if (gid == game2)
            switch(game->key_in)
            {
                case 'G'      : 
                case 'g'      : move_tetromino(left, gid);
                                break;
                case 'J'      : 
                case 'j'      : move_tetromino(right, gid);
                                break;
                case 'H'      :
                case 'h'      : move_tetromino(down, gid);
                                break;
                case 'Y'      :
                case 'y'      : move_tetromino(rotate, gid);
                                break;
                case 'A'      :
                case 'a'      : drop_tetromino(gid);
                                break;
                case '1'      : on_game = off;
                                // if(game->bonus_item > 0) item_use(gid);
                                break;
                case KEY_F(1) : cheat_key(gid);
                                break;
                case KEY_F(2) : game_status[gid] = off;
                                break;
                default       : break;
            }
        else if (gid == game3) ;
        else if (gid == game4) ;
        else ;
*/
    }

    return success;
}

/* move tetromino with keypad */
enum result move_tetromino(enum direction dir, enum gameid gid)
{
    int i, j;
    int pre_x, pre_y, pre_rot_degree;
    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;

    game = parameter_db(gid);
    bd = get_board(gid);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    // save original value of tetromino before moving to erase previous one
    pre_x = game->tet_x;
    pre_y = game->tet_y;
    pre_rot_degree = game->rot_degree;

    // get new value after moving
    if(check_collision(dir, gid) == 0)
        switch(dir)
        {
            case left  : game->tet_x--;
                         break;
            case right : game->tet_x++;
                         break;
            case down  : game->tet_y++;
                         break;
            case rotate: game->rot_degree++;
                         game->rot_degree %= 4;
                         break;
        }
        else
            return fail;

    // erase old one
    for(j = pre_y ; j < (pre_y + 4) ; j++)
        for(i = pre_x ; i < (pre_x + 4) ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[pre_rot_degree][j-pre_y][i-pre_x] != 0)
                    bd[j][i] = 0;

    // draw new one
    for(j = game->tet_y ; j < (game->tet_y + 4) ; j++)
        for(i = game->tet_x ; i < (game->tet_x + 4) ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[game->rot_degree][j-game->tet_y][i-game->tet_x] != 0)
                    bd[j][i] = (*ptr_tetromino)[game->rot_degree][j-game->tet_y][i-game->tet_x];

    return success;
}

/* to drop current tetromino down */
enum result drop_tetromino(enum gameid gid)
{
    struct gameparam *game;

    game = parameter_db(gid);

    while(!check_collision(down, gid))
        move_tetromino(down, gid);

    return success;
}


/* check collision among around tetrominos & edge(wall) */
enum flag check_collision(enum direction dir, enum gameid gid)
{
    int i, j;
    int tmp_x, tmp_y, pre_x, pre_y;
    int tmp_rot_degree;
    char temp_bd[BOARD_HEIGHT][BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;
    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];

    game = parameter_db(gid);
    bd = get_board(gid);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    // copy original buffers to temp buffers
    pre_x = tmp_x = game->tet_x;
    pre_y = tmp_y = game->tet_y;
    tmp_rot_degree = game->rot_degree;
    for(j = 0 ; j < BOARD_HEIGHT ; j++)
        for(i = 0 ; i < BOARD_WIDTH ; i++)
            temp_bd[j][i] = bd[j][i];

    switch(dir)
    {
        case  left :  tmp_x--;
                      break;
        case  right : tmp_x++;
                      break;
        case  down :  tmp_y++;
                      break;
        case rotate : tmp_rot_degree++;
                      tmp_rot_degree %=  4;
                      break;
    }

    // erase old one on temp buffer
    for(j = pre_y ; j < pre_y + 4 ; j++)
        for(i = pre_x ; i < pre_x + 4 ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[game->rot_degree][j-pre_y][i-pre_x] != 0)
                    temp_bd[j][i] = 0;

    // check collision on temp buffer
    for(j = 0 ; j < 4 ; j++)
        for(i = 0 ; i < 4 ; i++)
            if(temp_bd[tmp_y+j][tmp_x+i] != 0 && (*ptr_tetromino)[tmp_rot_degree][j][i] != 0)
                return success;

    return off;
}


/* to clear one line, check whether one line has hole or not */
int check_line_clear(enum gameid gid)
{
    enum flag hollow;
    struct gameparam *game;
    int h, i, j, pre_x, pre_y, line_cnt;
    char (*bd)[BOARD_WIDTH];
    char temp_bd[BOARD_HEIGHT][BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;

    line_cnt = 0;
    game = parameter_db(gid);
    bd = get_board(gid);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    // copy original buffers to temp buffers
    pre_x = game->tet_x;
    pre_y = game->tet_y;
    for(j = 0 ; j < BOARD_HEIGHT ; j++)
        for(i = 0 ; i < BOARD_WIDTH ; i++)
            temp_bd[j][i] = bd[j][i];  

    // erase old tetromino on temp buffer
    for(j = pre_y ; j < pre_y + 4 ; j++)
        for(i = pre_x ; i < pre_x + 4 ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[game->rot_degree][j-pre_y][i-pre_x] != 0)
                    temp_bd[j][i] = 0;

    // check  line has hollow in it
    for(j = (PLAY_BD_HEIGHT - 1) ; j > 0 ; j--)
    {
        if(bd[j][PLAY_BD_WIDTH/2] != DEAD_BLOCK)
        {
            hollow = off;

            for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++)
                if(bd[j][i] == 0)
                    hollow = on;

            if(!hollow)
            {
                game->score += ONELINE_CLR_SCORE;
                line_cnt ++;

                for(h = j ; h > 0 ; h--)
                    for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++) 
                        temp_bd[h][i] = temp_bd[h-1][i];
            }
        }
        else
            ;
    }

    // copy temp buffers to original buffers when line_cnt is not zero
    if (line_cnt)
    {
        for(j = 0 ; j < BOARD_HEIGHT ; j++)
            for(i = 0 ; i < BOARD_WIDTH ; i++)
                bd[j][i] = temp_bd[j][i];  
    }

    return line_cnt;
}


/* check whether clear all tetromino on board */
enum flag board_clear(enum gameid gid)
{
    int i, j, total_sum;
    static enum flag status = off;
    int tmp_x, tmp_y;
    int tmp_rot_degree;
    char temp_bd[BOARD_HEIGHT][BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;
    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];

    game = parameter_db(gid);
    bd = get_board(gid);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    // copy original buffers to temp buffers
    tmp_x = game->tet_x;
    tmp_y = game->tet_y;
    tmp_rot_degree = game->rot_degree;
    for(j = 0 ; j < BOARD_HEIGHT ; j++)
        for(i = 0 ; i < BOARD_WIDTH ; i++)
            temp_bd[j][i] = bd[j][i];

    // erase old one on temp buffer
    for(j = tmp_y ; j < tmp_y + 4 ; j++)
        for(i = tmp_x ; i < tmp_x + 4 ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[tmp_rot_degree][j-tmp_y][i-tmp_x] != 0)
                    temp_bd[j][i] = 0;


    // check whether clear all tetromino on board
    total_sum = 0;
    for(j = 0 ; j < PLAY_BD_HEIGHT ; j++)
        for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++) 
            total_sum += temp_bd[j][i];

    if(total_sum == 0)
        return on;

    return off;
}


/* to use bonus item to erase 3-line */
enum result use_bonus_item(enum gameid gid)
{
    int i, j;
    int pre_x, pre_y, pre_rot_degree;

    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;

    game = parameter_db(gid);
    bd = get_board(gid);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    // save original value of tetromino before moving to erase previous one
    pre_x = game->tet_x;
    pre_y = game->tet_y;
    pre_rot_degree = game->rot_degree;

    // decrease # of bonus items, increase bonus score    
    game->bonus_item--;
    game->score += ONELINE_CLR_SCORE;

    // erase current tetromino
    for(j = pre_y ; j < (pre_y + 4) ; j++)
        for(i = pre_x ; i < (pre_x + 4) ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[pre_rot_degree][j-pre_y][i-pre_x] != 0)
                    bd[j][i] = 0;

    // erase 3-lines from bottom
    for(j = (PLAY_BD_HEIGHT - 1) ; j > 2 ; j--)
    {      
        for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++) 
            bd[j][i] = bd[j-3][i];
    }

    for(j = 0 ; j < 3 ; j++)
    {
      for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++) 
          bd[j][i] = 0;
    }

    return success;
}


/* If more than 3-lines are cleared simultaneously, palyer can send lines/blocks to other player */
enum result send_lines(enum gameid gid, int curr_line_cnt)
{
    int i, j, addline_cnt;
    int pre_x, pre_y, pre_rot_degree;
    enum gameid other_player_id;

    struct gameparam *game;
    char (*bd)[BOARD_WIDTH];
    char (*ptr_tetromino)[4][4][4] = NULL;

    if(gid == game1)
        other_player_id = game2;
    else if(gid == game2)
        other_player_id = game1;
    else
        return fail;

     addline_cnt = curr_line_cnt - 1;

    game = parameter_db(other_player_id);
    bd = get_board(other_player_id);
    ptr_tetromino = get_tetromino(game->curr_tetromino);

    // save original value of tetromino before moving to erase previous one
    pre_x = game->tet_x;
    pre_y = game->tet_y;
    pre_rot_degree = game->rot_degree;

    // erase current tetromino
    for(j = pre_y ; j < (pre_y + 4) ; j++)
        for(i = pre_x ; i < (pre_x + 4) ; i++)
            if(j > 0 && j < PLAY_BD_HEIGHT && i > 0 && i < (BOARD_WIDTH-1) )
                if((*ptr_tetromino)[pre_rot_degree][j-pre_y][i-pre_x] != 0)
                    bd[j][i] = 0;

    // add number of 'addline_cnt' lines from bottom
    for(j = addline_cnt ; j < PLAY_BD_HEIGHT ; j++)
    {
        for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++) 
            bd[j-addline_cnt][i] = bd[j][i];
    }

    for(j = 0 ; j < addline_cnt ; j++)
    {
        for(i = 1 ; i < (BOARD_WIDTH - 1) ; i++) 
            bd[(PLAY_BD_HEIGHT - 1) - j][i] = DEAD_BLOCK;
    }

    return success;
}


/* to control game level(speed) by adjusting vitual timer & move period */
enum result control_game_level(enum gameid gid)
{
    struct gameparam *game;

    game = parameter_db(gid);

    game->speedup_count++;
    game->speedup_count %= SPEED_UP_BASE;

    // decrease moving resolution, every 500(SPEED_UP_BASE) ticks
    if( game->speedup_count == (SPEED_UP_BASE - 1) )
        if(game->move_resolution > 0) game->move_resolution--;

    // player get 1 item, every BONUS_BASE(ex. 500) point
    if(game->score - game->bonus_score > BONUS_BASE)
    {
        game->bonus_score += BONUS_BASE;
        if(game->bonus_item < MAX_BONUS_ITEM)
            game->bonus_item++;
    }

    return success;
}


/* to go next stage */
enum result next_stage(enum gameid gid)
{
    enum heartbeat tick;
    struct gameparam *game;

    game = parameter_db(gid); 

    if(game->stage < TOTAL_STAGE)
        game->stage++;
    game->curr_tetromino = I_TETROMINO;
    game->next_tetromino = show_next_tetromino(game->wnd_x, game->wnd_y - TITLE_HEIGHT, gid);  
    game->rot_degree = DEG_90;
    game->line_cnt = 0;
    game->num_of_tet = 0;
    game->tickcount = 0;
    game->bottom_cnt = 0;
    game->speedup_count = 0;
    game->move_resolution = MAX_MOVE_RESOLUTION;
    init_board(gid);

    if(game->tick < beat5) game->tick++;
    init_timer(gid, on);

    return success;
}


/* to check game over situation */
enum flag check_gameover(enum gameid gid)
{
    struct gameparam *game;

    game = parameter_db(gid);

    if(game->tet_x == (BOARD_WIDTH / 2 - 1) && game->tet_y == 0)
        if(check_collision(left, gid) || check_collision(right, gid) || check_collision(down, gid) || check_collision(rotate, gid))
            return off;

    return on;
}


/* tasks to execute after game1 & game2 are over */
enum result gameovertask(enum flag status)
{
    int pos_x, pos_y;
    static enum flag disp;
    char *p_msg;
    enum gameid winner_gid;
    struct gameparam *game_1, *game_2;

    if(status == off)
        disp = off;
    else
        disp = on;

    game_1 = parameter_db(game1);

    if(total_players == 2) 
        game_2 = parameter_db(game2);

    keypad_hadler(winner_gid, on);

    if ( disp == off )
    {
        display_ending(DISP_NEXTGAME, nop);

        if(total_players == 2)
        {
            if( game_1->score >= game_2->score)
            {
                game_1->win_count++;
                winner_gid = game1;
            }

            if( game_1->score < game_2->score)
            {
                game_2->win_count++;
                winner_gid = game2;
            }

            display_ending(DISP_WINNER, winner_gid);     
        }

        disp = on;
    }

    return success;
}


/* to start game with new or continue mode */
enum result next_game(enum gamemode mod)
{
    char *p_msg;

    deinit_game(game1);
    if(total_players == 2)
        deinit_game(game2);

    display_ending(DISP_CLEAR, nop);

    game_status[game1] = on;
    if(total_players == 2)
        game_status[game2] = on;

    init_game(game1, mod);
    if(total_players == 2)
        init_game(game2, mod);

    return success;    
}


/* game parameter DB(data base) */
struct gameparam *parameter_db(enum gameid gid)
{
    static struct gameparam game[TOTAL_PLAYER];
    struct gameparam *p_game;

    switch(gid)
    {
        case game1 : p_game = &game[game1];
                     break;
        case game2 : p_game = &game[game2];
                     break;
        case game3 :
        case game4 :
        default    : break;
    }

    return p_game;
}

/* return game board of each gameid */
char (*get_board(enum gameid gid))[]
{
    // memory assignment for game board
    static char tetris_board_1[BOARD_HEIGHT][BOARD_WIDTH];
    static char tetris_board_2[BOARD_HEIGHT][BOARD_WIDTH];
    char (*bd)[BOARD_WIDTH];

    switch (gid)
    {
        case game1 : bd = tetris_board_1;
                     break;    
        case game2 : bd = tetris_board_2;
                     break;
        case game3 :
        case game4 :
        default    : bd = NULL;
                     break;
    }

    return bd;
}


/* return proper pointer of tetromino arrays */
char (*get_tetromino(int tetromino))[4][4][4]
{
    char (*ptr_tetromino)[4][4][4] = NULL;

    // mapping current tetromino to pointer
    switch(tetromino)
    {
        case I_TETROMINO : ptr_tetromino = &i_tetromino;
                           break;
        case O_TETROMINO : ptr_tetromino = &o_tetromino;
                           break;
        case T_TETROMINO : ptr_tetromino = &t_tetromino;
                           break;
        case J_TETROMINO : ptr_tetromino = &j_tetromino;
                           break;
        case L_TETROMINO : ptr_tetromino = &l_tetromino;
                           break;
        case S_TETROMINO : ptr_tetromino = &s_tetromino;
                           break;
        case Z_TETROMINO : ptr_tetromino = &z_tetromino;
                           break;
    }

    return ptr_tetromino;
}


/* convert integer to string */
enum result int2str(int number, char* num_str)
{
    float num = (float)number;
    int i = 0, pwr = 1;
    char temp_ch[256];

    while( (int)(num/pwr) > (float)((1/3)*3) )
    {
        temp_ch[i++] = '0' + (int)num % (pwr*10) / pwr;
        pwr *= 10;
    }

    if(number == 0) temp_ch[i++] = '0';

    while( i > 0 )
        *(num_str++) = temp_ch[--i];

    *num_str = '\0';

    return success;
}
