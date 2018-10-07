#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <iostream>//temp
#include <gl/gl.h>
#include <Box2D/Box2D.h>
#include "decal.h"
#include "gamepad.h"

#define _Met2Pix 20
#define _Pix2Met 1/20

using namespace std;

struct st_body_user_data
{
    int i_player_number;
};

class main_menu
{
    public:
        main_menu();

        bool m_ready;

        bool init(int window_width,int window_height,int texture);
        bool draw(void);
        bool update(void);

    private:

        int m_window_width,m_window_height;
        int m_texture;
        float m_f_size_ratio;

        b2World* m_World;
        b2Body*  m_p_flipper_body[4];

        gamepad m_gamepad[4];
        bool    m_connected_player[4];

        decal m_dec_title,m_dec_player_icon[4];

        bool init_box2d(void);
        bool add_new_player(int player_number);
        bool remove_player(int player_number);
};

#endif // MAIN_MENU_H
