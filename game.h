#ifndef GAME_H
#define GAME_H

#include <SOIL/SOIL.h>
#include <gl/gl.h>
#include <Box2D/Box2D.h>
#include <time.h>
#include <iostream>//for debug
#include <vector>
#include "base64.h"
#include "files_in_text.h"
#include "sound.h"
#include "MyContactListener.h"
#include "gamepad.h"
#include "display_7seg.h"
#include "decal.h"

#define _version 1.20

#define _key_delay 0.2

#define _Deg2Rad 0.0174532925
#define _Rad2Deg 57.2957795
#define _Met2Pix 20.0
#define _Pix2Met 1.0/20.0

#define _intro_shade_delay 4.0
#define _info_box_shade_delay 5.0
#define _wall_color_delay 2.0
#define _game_start_delay 3.0
#define _goal_delay 3.0
#define _leave_delay 2.0
#define _game_over_delay 9.5
#define _col_sound_cooldown 0.1
#define _goal_limit 9

#define _move_speed 2.3
#define _move_damping 2.0
#define _rotate_speed 2.3
#define _rotate_damping 15.0

#define _goal_density 30.0

using namespace std;

enum states
{
    state_error=0,
    state_init,
    state_lobby,
    state_running,
    state_game_over
};

struct st_point
{
    st_point()
    {
        x=0; y=0;
    }
    st_point(float nx,float ny)
    {
        x=nx; y=ny;
    }
    float x,y;
    st_point operator+(st_point point)
    {
        st_point new_point;
        new_point.x=x+point.x;
        new_point.y=y+point.y;
        return new_point;
    }
    st_point operator-(st_point point)
    {
        st_point new_point;
        new_point.x=x-point.x;
        new_point.y=y-point.y;
        return new_point;
    }
};

struct st_color
{
    float R,G,B;
    float brightness;
};

struct st_body_user_data
{
    int i_player_number;
    string s_info;
    vector<st_point> vec_vertex_points;
    vector<st_point> vec_texture_points;
    st_color col_color;
    float col_update_speed;
};

class game
{
    public:
        game();

        bool m_ready;
        bool m_debug_mode;

        bool init(int window_width,int window_height,bool debug_mode);
        bool draw(void);
        bool cycle(bool keys[],bool mouse_but[],int mouse_pos[]);

    private:

        float m_screen_ratio_x,m_screen_ratio_y;

        float m_time_this_cycle,m_time_last_cycle,m_key_delay;

        bool  m_reset_board,m_game_start_requested;
        float m_reset_board_tick,m_game_over_tick,m_game_start_tick;

        int m_state;
        int m_window_width,m_window_height;
        float m_f_size_ratio;

        int m_texture;//temp

        sound* m_p_SoundEngine;
        float  m_col_sound_cooldown;
        int    m_music_source;
        bool   m_music_loop_started,m_music_enabled;

        st_color m_wall_colors[8];
        int      m_wall_color_shift;
        float    m_wall_color_tick,m_wall_color_progress;

        gamepad m_gamepad[4];
        bool    m_connected_player[4];
        bool    m_player_ingame[4];
        float   m_leave_timer[4];

        b2World*   m_World;
        b2World*   m_Menu_World;
        b2Body*    m_ball;
        b2Body*    m_p_flipper_body[4];
        b2Body*    m_walls[8];
        b2Body*    m_goal_left;
        b2Body*    m_goal_right;
        b2Body*    m_body_pointers[2];
        /*b2Body     m_body_dummy1;
        b2Body     m_body_dummy2;*/
        b2Fixture* m_goal_sensor_left;
        b2Fixture* m_goal_sensor_right;
        int*       m_p_event_flag;
        int*       m_p_sound_flag;
        float*     m_p_sound_vol;

        //main menu letters
        b2Body*    m_letter_F;
        b2Body*    m_letter_L;
        b2Body*    m_letter_I;
        b2Body*    m_letter_Idot;
        b2Body*    m_letter_P;
        b2Body*    m_info_bodies[4];

        display_7seg  m_score_display_left,m_score_display_right,m_countdown_timer;
        decal m_dec_game_over,m_dec_credits;
        float m_intro_shade_tick;
        float m_intro_shade_progress; // 1-0 -> 1
        float m_info_box_shade_tick;
        float m_info_box_shade_progress; // 0-1

        string m_s_winner;//Right or Left

        MyContactListener m_myContactListenerInstance;//for collision detection

        bool init_box2d(void);
        bool init_main_menu(float xpos,float ypos,float size);
        bool reset_main_menu(void);
        bool add_new_player(int player_number,b2World* world);
        bool remove_player(int player_number);
        bool load_textures(void);
        bool load_sounds(void);
        bool handle_event(void);
        bool handle_sound_event(void);
        bool reset_board(void);
        bool game_over(string winner);
};

void func(b2Contact*);


#endif // GAME_H
