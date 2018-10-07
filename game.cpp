#include "game.h"

#include <fstream>//TEMP

game::game()
{
    m_time_this_cycle=m_time_last_cycle=0;

    m_state=state_init;

    m_ready=false;
    m_debug_mode=false;
}

bool game::init(int window_width,int window_height,bool debug_mode)
{
    m_debug_mode=debug_mode;
    m_key_delay=0.0;

    m_window_width=window_width;
    m_window_height=window_height;

    m_screen_ratio_x=(float)window_width/1920.0;
    m_screen_ratio_y=(float)window_height/1080.0;

    //test if 16:9 ratio
    float screen_ratio=m_screen_ratio_x/m_screen_ratio_y; //fix ratio problem
    if( screen_ratio>1.01 )//too wide
    {//lower x ratio
        //black right
        float fake_width=m_screen_ratio_y*16.0/9.0*1080.0;
        m_screen_ratio_x=fake_width/1920.0;
    }
    else if( screen_ratio<0.99 )//too tall
    {//lower y ratio
        //black bottom
        float fake_height=m_screen_ratio_x*9.0/16.0*1920.0;
        m_screen_ratio_y=fake_height/1080.0;
    }

    m_p_event_flag=new int;
    *m_p_event_flag=0;
    m_p_sound_flag=new int;
    *m_p_sound_flag=-1;
    m_p_sound_vol=new float;
    *m_p_sound_vol=0;
    m_col_sound_cooldown=0;

    //load data
    load_textures();
    load_sounds();

    //init box2d
    init_box2d();

    for(int i=0;i<4;i++)
    {
        m_gamepad[i]=gamepad(i);
        m_connected_player[i]=false;
        m_player_ingame[i]=false;
        m_leave_timer[i]=_leave_delay;
    }

    m_score_display_left.init(96.0*m_screen_ratio_x, 75.6*m_screen_ratio_y, 71.04*m_screen_ratio_x, 86.4*m_screen_ratio_y);
    m_score_display_right.init(1766.4*m_screen_ratio_x, 75.6*m_screen_ratio_y, 71.04*m_screen_ratio_x, 86.4*m_screen_ratio_y);
    m_score_display_left.set_texture(m_texture,98.0,183.0,26.0,49.0);
    m_score_display_right.set_texture(m_texture,98.0,183.0,26.0,49.0);
    m_countdown_timer.init(825.6*m_screen_ratio_x, 702.0*m_screen_ratio_y, 213.12*m_screen_ratio_x, 259.2*m_screen_ratio_y);
    m_countdown_timer.set_texture(m_texture,98.0,183.0,26.0,49.0);

    float size=2.0;
    m_dec_game_over=decal( 530.0*m_screen_ratio_x,400.0*m_screen_ratio_y,(623.0-212.0)*size*m_screen_ratio_x,(92.0-6.0)*size*m_screen_ratio_y,m_texture,212.0,623.0,6.0,92.0 );
    float color[]={1.0,0,0};
    m_dec_game_over.set_color_mask(color);
    //m_dec_game_over.zoom(0.8);
    /*//old decal
    float credit_size=1.0;
    m_dec_credits=decal( 1790.0*m_screen_ratio_x,1050.0*m_screen_ratio_y,(778.0-655.0)*credit_size*m_screen_ratio_x,(51-28)*credit_size*m_screen_ratio_y,m_texture,655,778,28,51 );
    */
    float credit_size=1.0;
    m_dec_credits=decal( 1540.0*m_screen_ratio_x,1040.0*m_screen_ratio_y,
                         (966.0-594.0)*credit_size*m_screen_ratio_x,(199-162)*credit_size*m_screen_ratio_y,
                         m_texture,594,966,162,199 );
    float color_credits[]={0.3,0.3,0.3};
    m_dec_credits.set_color_mask(color_credits);

    //set wall colors
    m_wall_color_shift=7;
    m_wall_color_progress=0;
    m_wall_color_tick=_wall_color_delay;

    m_wall_colors[0].R=0.1; m_wall_colors[0].G=0.1; m_wall_colors[0].B=0.1; m_wall_colors[0].brightness=1.0;
    m_wall_colors[1].R=0.1; m_wall_colors[1].G=0.1; m_wall_colors[1].B=0.8; m_wall_colors[1].brightness=1.0;
    m_wall_colors[2].R=0.3; m_wall_colors[2].G=0.3; m_wall_colors[2].B=0.9; m_wall_colors[2].brightness=1.0;
    m_wall_colors[3].R=0.1; m_wall_colors[3].G=0.1; m_wall_colors[3].B=0.8; m_wall_colors[3].brightness=1.0;
    m_wall_colors[4].R=0.1; m_wall_colors[4].G=0.1; m_wall_colors[4].B=0.1; m_wall_colors[4].brightness=1.0;
    m_wall_colors[5].R=0.8; m_wall_colors[5].G=0.1; m_wall_colors[5].B=0.1; m_wall_colors[5].brightness=1.0;
    m_wall_colors[6].R=0.9; m_wall_colors[6].G=0.3; m_wall_colors[6].B=0.3; m_wall_colors[6].brightness=1.0;
    m_wall_colors[7].R=0.8; m_wall_colors[7].G=0.1; m_wall_colors[7].B=0.1; m_wall_colors[7].brightness=1.0;

    m_state=state_lobby;
    m_reset_board=false;
    m_reset_board_tick=0;
    m_game_over_tick=0;
    m_intro_shade_progress=0;
    m_info_box_shade_tick=_info_box_shade_delay;
    m_info_box_shade_progress=0;
    m_intro_shade_tick=_intro_shade_delay;
    m_game_start_tick=_game_start_delay;
    m_game_start_requested=false;

    //play music
    m_music_source=m_p_SoundEngine->playSimpleSound(wav_music_intro,1.0,20);
    m_music_loop_started=false;
    m_music_enabled=true;

    m_ready=true;

    return true;
}

bool game::draw(void)
{

    switch(m_state)
    {
        case state_lobby:
        {

            if(m_debug_mode)//debug drawing
            {
                glColor3f(1,1,1);
                b2Body* tmp=m_Menu_World->GetBodyList();
                while(tmp)
                {
                    glPushMatrix();
                    b2Vec2 center=tmp->GetWorldCenter();
                    b2Vec2 mass_center=tmp->GetLocalCenter();

                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(tmp->GetAngle()*180/3.14159,0,0,1);
                    //dont shift circles
                    if( ((b2Shape*)tmp->GetFixtureList()->GetShape())->m_type != 0 )
                    {
                        glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);//shift to rotation center
                    }

                    for( b2Fixture* fixture=tmp->GetFixtureList(); fixture; fixture=fixture->GetNext() )
                    {
                        //test if sensor
                        if( fixture->IsSensor() )
                        {
                            //red
                            glColor3f(1,0.1,0.1);
                        }
                        else
                        {
                            //white
                            glColor3f(1,1,1);
                        }


                        //test if circle
                        if( ((b2Shape*)fixture->GetShape())->m_type == 0 )
                        {
                            float radius=((b2Shape*)fixture->GetShape())->m_radius;
                            glBegin(GL_TRIANGLE_FAN);
                            glVertex2f(0,0);
                            for(float i=0.0;i<=360;i+=36)
                            {
                                glVertex2f( cosf( i*3.14159/180 )*radius*_Met2Pix*m_screen_ratio_x,
                                            sinf( i*3.14159/180 )*radius*_Met2Pix*m_screen_ratio_y );
                            }
                            glEnd();
                        }
                        else //not circle
                        {
                            //int vertex_count=((b2PolygonShape*)tmp->GetFixtureList()->GetShape())->GetVertexCount();
                            int vertex_count=((b2PolygonShape*)fixture->GetShape())->GetVertexCount();
                            b2Vec2 points[vertex_count];

                            for(int i=0;i<vertex_count;i++)
                                points[i]=((b2PolygonShape*)fixture->GetShape())->GetVertex(i);

                            glBegin(GL_LINE_STRIP);
                            for(int i=0;i<vertex_count;i++)
                                glVertex2f(points[i].x*_Met2Pix*m_screen_ratio_x,points[i].y*_Met2Pix*m_screen_ratio_y);

                            glVertex2f(points[0].x*_Met2Pix*m_screen_ratio_x,points[0].y*_Met2Pix*m_screen_ratio_y);

                            glEnd();
                        }

                        //fixture=fixture->GetNext();
                    }

                    glPopMatrix();

                    tmp=tmp->GetNext();
                }
            }

            //draw flippers
            for(int player_index=0;player_index<4;player_index++)
            {
                if( m_player_ingame[player_index] ) //draw flipper 0
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_p_flipper_body[ player_index ]->GetUserData() );
                    glPushMatrix();

                    b2Vec2 center=m_p_flipper_body[ player_index ]->GetWorldCenter();
                    b2Vec2 mass_center=m_p_flipper_body[ player_index ]->GetLocalCenter();
                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(m_p_flipper_body[ player_index ]->GetAngle()*180/3.14159,0,0,1);

                    //update colors
                    switch(player_index)
                    {
                        case 0:
                        {
                            data->col_color.R=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.G=0.1+cosf( m_time_this_cycle*2.0 )/20.0;
                            data->col_color.B=0.1+cosf( m_time_this_cycle*3.0 )/20.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+1)*cosf(m_time_this_cycle*1*data->col_update_speed+86);
                        }break;

                        case 1:
                        {
                            data->col_color.R=0.2+cosf( m_time_this_cycle*1.0 )/10.0;
                            data->col_color.G=0.2+cosf( m_time_this_cycle*2.0 )/10.0;
                            data->col_color.B=0.8+cosf( m_time_this_cycle*3.0 )/5.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+16)*cosf(m_time_this_cycle*1*data->col_update_speed+110);
                        }break;

                        case 2:
                        {
                            data->col_color.R=0.2+cosf( m_time_this_cycle*1.0 )/10.0;
                            data->col_color.G=0.8+cosf( m_time_this_cycle*2.0 )/5.0;
                            data->col_color.B=0.2+cosf( m_time_this_cycle*3.0 )/10.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+24)*cosf(m_time_this_cycle*1*data->col_update_speed+432);
                        }break;

                        case 3:
                        {
                            data->col_color.R=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.G=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.B=0.2+cosf( m_time_this_cycle*3.0 )/10.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+123)*cosf(m_time_this_cycle*1*data->col_update_speed+3);
                        }break;
                    }

                    glColor3f( data->col_color.R*data->col_color.brightness,
                               data->col_color.G*data->col_color.brightness,
                               data->col_color.B*data->col_color.brightness );

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_texture);

                    glBegin(GL_TRIANGLE_STRIP);
                    for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                    {
                        glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                        glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                    }
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    glPopMatrix();
                }
            }

            //draw letters
            if(m_letter_F!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_letter_F->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_letter_F->GetWorldCenter();
                b2Vec2 mass_center=m_letter_F->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_letter_F->GetAngle()*180/3.14159,0,0,1);
                glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                //color
                data->col_color.R=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+5 )/2.0;
                data->col_color.G=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+10 )/2.0;
                data->col_color.B=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+20 )/2.0;
                data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+123)*cosf(m_time_this_cycle*1*data->col_update_speed+3);
                if(data->col_update_speed>1) data->col_update_speed-=m_time_this_cycle*0.01;
                glColor3f( data->col_color.R*data->col_color.brightness,
                           data->col_color.G*data->col_color.brightness,
                           data->col_color.B*data->col_color.brightness );

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }
            if(m_letter_L!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_letter_L->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_letter_L->GetWorldCenter();
                b2Vec2 mass_center=m_letter_L->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_letter_L->GetAngle()*180/3.14159,0,0,1);
                glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                //color
                data->col_color.R=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+10 )/2.0;
                data->col_color.G=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+1 )/2.0;
                data->col_color.B=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+30 )/2.0;
                data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+13)*cosf(m_time_this_cycle*1*data->col_update_speed+80);
                if(data->col_update_speed>1) data->col_update_speed-=m_time_this_cycle*0.01;
                glColor3f( data->col_color.R*data->col_color.brightness,
                           data->col_color.G*data->col_color.brightness,
                           data->col_color.B*data->col_color.brightness );

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }
            if(m_letter_I!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_letter_I->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_letter_I->GetWorldCenter();
                b2Vec2 mass_center=m_letter_I->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_letter_I->GetAngle()*180/3.14159,0,0,1);
                glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                //color
                data->col_color.R=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+90 )/2.0;
                data->col_color.G=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+40 )/2.0;
                data->col_color.B=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+80 )/2.0;
                data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+61)*cosf(m_time_this_cycle*1*data->col_update_speed+101);
                if(data->col_update_speed>1) data->col_update_speed-=m_time_this_cycle*0.01;
                glColor3f( data->col_color.R*data->col_color.brightness,
                           data->col_color.G*data->col_color.brightness,
                           data->col_color.B*data->col_color.brightness );

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }
            if(m_letter_Idot!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_letter_Idot->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_letter_Idot->GetWorldCenter();
                b2Vec2 mass_center=m_letter_Idot->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_letter_Idot->GetAngle()*180/3.14159,0,0,1);
                glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                //color
                data->col_color.R=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+10 )/2.0;
                data->col_color.G=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+80 )/2.0;
                data->col_color.B=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+1 )/2.0;
                data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+58)*cosf(m_time_this_cycle*1*data->col_update_speed+4);
                if(data->col_update_speed>1) data->col_update_speed-=m_time_this_cycle*0.01;
                glColor3f( data->col_color.R*data->col_color.brightness,
                           data->col_color.G*data->col_color.brightness,
                           data->col_color.B*data->col_color.brightness );

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }
            if(m_letter_P!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_letter_P->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_letter_P->GetWorldCenter();
                b2Vec2 mass_center=m_letter_P->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_letter_P->GetAngle()*180/3.14159,0,0,1);
                glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                //color
                data->col_color.R=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+60 )/2.0;
                data->col_color.G=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+50 )/2.0;
                data->col_color.B=0.5+cosf( m_time_this_cycle*data->col_update_speed*0.3+100 )/2.0;
                data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+37)*cosf(m_time_this_cycle*1*data->col_update_speed+73);
                if(data->col_update_speed>1) data->col_update_speed-=m_time_this_cycle*0.01;
                glColor3f( data->col_color.R*data->col_color.brightness,
                           data->col_color.G*data->col_color.brightness,
                           data->col_color.B*data->col_color.brightness );

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }

            //draw text info
            for( int i=0; i<4; i++)
            {
                if(m_info_bodies[i]!=NULL)
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_info_bodies[i]->GetUserData() );
                    glPushMatrix();

                    b2Vec2 center=m_info_bodies[i]->GetWorldCenter();
                    b2Vec2 mass_center=m_info_bodies[i]->GetLocalCenter();
                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(m_info_bodies[i]->GetAngle()*180/3.14159,0,0,1);
                    glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                    //color
                    if( m_info_box_shade_tick>0 && !m_game_start_requested )
                    {
                        switch(i)
                        {
                            case 0: data->col_color.brightness=1.0-m_info_box_shade_tick/(_info_box_shade_delay-0.0); break;
                            case 1: data->col_color.brightness=1.0-m_info_box_shade_tick/(_info_box_shade_delay-1.0); break;
                            case 2: data->col_color.brightness=1.0-m_info_box_shade_tick/(_info_box_shade_delay-2.0); break;
                            case 3: data->col_color.brightness=1.0-m_info_box_shade_tick/(_info_box_shade_delay-3.0); break;
                        }
                    }
                    if( data->col_color.brightness>1.0 ) data->col_color.brightness=1.0;//roof
                    glColor3f( data->col_color.R*data->col_color.brightness,
                               data->col_color.G*data->col_color.brightness,
                               data->col_color.B*data->col_color.brightness );

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_texture);

                    glBegin(GL_TRIANGLE_STRIP);
                    for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                    {
                        glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                        glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                    }
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    glPopMatrix();
                }
            }

            //draw countdown timer
            if(m_game_start_requested)
            {
                m_countdown_timer.draw();
            }

            //draw credits decal
            m_dec_credits.draw();

            //intro shade
            if(m_intro_shade_tick>0)
            {
                //draw transparent black square
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0,0,0,m_intro_shade_progress);//black
                glBegin(GL_QUADS);
                glVertex2f(0,0);
                glVertex2f(0,m_window_height);
                glVertex2f(m_window_width,m_window_height);
                glVertex2f(m_window_width,0);
                glEnd();
                glDisable(GL_BLEND);
            }


        }break;

        case state_running:
        {

            if(m_debug_mode)//debug drawing
            {
                glColor3f(1,1,1);
                b2Body* tmp=m_World->GetBodyList();
                while(tmp)
                {
                    glPushMatrix();
                    b2Vec2 center=tmp->GetWorldCenter();
                    b2Vec2 mass_center=tmp->GetLocalCenter();

                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(tmp->GetAngle()*180/3.14159,0,0,1);
                    //dont shift circles
                    if( ((b2Shape*)tmp->GetFixtureList()->GetShape())->m_type != 0 )
                    {
                        glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);//shift to rotation center
                    }

                    for( b2Fixture* fixture=tmp->GetFixtureList(); fixture; fixture=fixture->GetNext() )
                    {
                        //test if sensor
                        if( fixture->IsSensor() )
                        {
                            //red
                            glColor3f(1,0.1,0.1);
                        }
                        else
                        {
                            //white
                            glColor3f(1,1,1);
                        }


                        //test if circle
                        if( ((b2Shape*)fixture->GetShape())->m_type == 0 )
                        {
                            float radius=((b2Shape*)fixture->GetShape())->m_radius;
                            glBegin(GL_TRIANGLE_STRIP);
                            //glVertex2f(0,0);
                            for(float i=0.0;i<=360;i+=36)
                            {
                                glVertex2f( cosf( i*3.14159/180 )*radius*_Met2Pix*m_screen_ratio_x, sinf( i*3.14159/180 )*radius*_Met2Pix*m_screen_ratio_y );
                            }
                            glEnd();
                        }
                        else //not circle
                        {
                            //int vertex_count=((b2PolygonShape*)tmp->GetFixtureList()->GetShape())->GetVertexCount();
                            int vertex_count=((b2PolygonShape*)fixture->GetShape())->GetVertexCount();
                            b2Vec2 points[vertex_count];

                            for(int i=0;i<vertex_count;i++)
                                points[i]=((b2PolygonShape*)fixture->GetShape())->GetVertex(i);

                            glBegin(GL_LINE_STRIP);
                            for(int i=0;i<vertex_count;i++)
                                glVertex2f(points[i].x*_Met2Pix*m_screen_ratio_x,points[i].y*_Met2Pix*m_screen_ratio_y);

                            glVertex2f(points[0].x*_Met2Pix*m_screen_ratio_x,points[0].y*_Met2Pix*m_screen_ratio_y);

                            glEnd();
                        }

                        //fixture=fixture->GetNext();
                    }

                    glPopMatrix();

                    tmp=tmp->GetNext();
                }
            }

            //draw walls
            for(int wall=0;wall<8;wall++)
            {
                if( m_walls[wall]!=NULL ) //draw wall
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_walls[wall]->GetUserData() );
                    glPushMatrix();

                    b2Vec2 center=m_walls[wall]->GetWorldCenter();
                    b2Vec2 mass_center=m_walls[wall]->GetLocalCenter();
                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(m_walls[wall]->GetAngle()*180/3.14159,0,0,1);
                    glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                    //get wall color
                    st_color col_first,col_second,col_third;
                    int current_color_index=wall+m_wall_color_shift;
                    while( current_color_index>=8 ) current_color_index-=8;
                    col_first=m_wall_colors[ current_color_index ];
                    int next_color_index=wall+m_wall_color_shift+1;
                    while( next_color_index>=8 ) next_color_index-=8;
                    col_second=m_wall_colors[ next_color_index ];
                    int third_color_index=wall+m_wall_color_shift+2;
                    while( third_color_index>=8 ) third_color_index-=8;
                    col_third=m_wall_colors[ third_color_index ];
                    //update current colors
                    float dR=col_second.R-col_first.R;
                    float dG=col_second.G-col_first.G;
                    float dB=col_second.B-col_first.B;
                    col_first.R+=dR*m_wall_color_progress;
                    col_first.G+=dG*m_wall_color_progress;
                    col_first.B+=dB*m_wall_color_progress;
                    //update second color
                    dR=col_third.R-col_second.R;
                    dG=col_third.G-col_second.G;
                    dB=col_third.B-col_second.B;
                    col_second.R+=dR*m_wall_color_progress;
                    col_second.G+=dG*m_wall_color_progress;
                    col_second.B+=dB*m_wall_color_progress;

                    glColor4f(1,1,1,1);//temp

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_texture);

                    glBegin(GL_QUADS);
                    for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                    {
                        if(i<2)//first color
                        {
                            glColor3f( col_first.R*col_first.brightness,
                                       col_first.G*col_first.brightness,
                                       col_first.B*col_first.brightness );
                        }
                        else//second color
                        {
                            glColor3f( col_second.R*col_second.brightness,
                                       col_second.G*col_second.brightness,
                                       col_second.B*col_second.brightness );
                        }

                        glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                        glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                    }
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    glPopMatrix();
                }
            }

            //draw flippers
            for(int player_index=0;player_index<4;player_index++)
            {
                if( m_player_ingame[player_index] ) //draw flippers
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_p_flipper_body[ player_index ]->GetUserData() );
                    glPushMatrix();

                    b2Vec2 center=m_p_flipper_body[ player_index ]->GetWorldCenter();
                    b2Vec2 mass_center=m_p_flipper_body[ player_index ]->GetLocalCenter();
                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(m_p_flipper_body[ player_index ]->GetAngle()*180/3.14159,0,0,1);

                    //update colors
                    switch(player_index)
                    {
                        case 0:
                        {
                            data->col_color.R=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.G=0.1+cosf( m_time_this_cycle*2.0 )/20.0;
                            data->col_color.B=0.1+cosf( m_time_this_cycle*3.0 )/20.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+1)*cosf(m_time_this_cycle*1*data->col_update_speed+86);
                        }break;

                        case 1:
                        {
                            data->col_color.R=0.2+cosf( m_time_this_cycle*1.0 )/10.0;
                            data->col_color.G=0.2+cosf( m_time_this_cycle*2.0 )/10.0;
                            data->col_color.B=0.8+cosf( m_time_this_cycle*3.0 )/5.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+16)*cosf(m_time_this_cycle*1*data->col_update_speed+110);
                        }break;

                        case 2:
                        {
                            data->col_color.R=0.2+cosf( m_time_this_cycle*1.0 )/10.0;
                            data->col_color.G=0.8+cosf( m_time_this_cycle*2.0 )/5.0;
                            data->col_color.B=0.2+cosf( m_time_this_cycle*3.0 )/10.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+24)*cosf(m_time_this_cycle*1*data->col_update_speed+432);
                        }break;

                        case 3:
                        {
                            data->col_color.R=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.G=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.B=0.2+cosf( m_time_this_cycle*3.0 )/10.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+123)*cosf(m_time_this_cycle*1*data->col_update_speed+3);
                        }break;
                    }

                    glColor3f( data->col_color.R*data->col_color.brightness,
                               data->col_color.G*data->col_color.brightness,
                               data->col_color.B*data->col_color.brightness );

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_texture);

                    glBegin(GL_TRIANGLE_STRIP);
                    for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                    {
                        glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                        glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                    }
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    glPopMatrix();
                }
            }

            //draw ball
            if(m_ball!=NULL)
            {
                st_body_user_data* data_ball=static_cast<st_body_user_data*>( m_ball->GetUserData() );
                glPushMatrix();
                b2Vec2 center=m_ball->GetWorldCenter();
                b2Vec2 mass_center=m_ball->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);
                glRotatef(m_ball->GetAngle()*180/3.14159,0,0,1);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                //update color
                data_ball->col_color.R=0.6+cosf( m_time_this_cycle*1 )/5;
                data_ball->col_color.G=0.6+cosf( m_time_this_cycle*2 )/5;
                data_ball->col_color.B=0.6+cosf( m_time_this_cycle*3 )/5;

                glColor3f(data_ball->col_color.R,data_ball->col_color.G,data_ball->col_color.B);

                glBegin(GL_TRIANGLE_FAN);
                for( int i=0;i<(int)data_ball->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data_ball->vec_texture_points[i].x, data_ball->vec_texture_points[i].y );
                    glVertex2f( data_ball->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data_ball->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }

            //draw goals
            if(m_goal_left!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_goal_left->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_goal_left->GetWorldCenter();
                b2Vec2 mass_center=m_goal_left->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_goal_left->GetAngle()*180/3.14159,0,0,1);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glColor3f(data->col_color.R,data->col_color.G,data->col_color.B);

                //update color
                data->col_color.R=0.8+cosf( m_time_this_cycle*1 )/5;
                data->col_color.G=0.1+cosf( m_time_this_cycle*2 )/10;
                data->col_color.B=0.1+cosf( m_time_this_cycle*3 )/10;

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }

            if(m_goal_right!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_goal_right->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_goal_right->GetWorldCenter();
                b2Vec2 mass_center=m_goal_right->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_goal_right->GetAngle()*180/3.14159,0,0,1);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glColor3f(data->col_color.R,data->col_color.G,data->col_color.B);

                //update color
                data->col_color.R=0.1+cosf( m_time_this_cycle*2 )/10;
                data->col_color.G=0.1+cosf( m_time_this_cycle*4 )/10;
                data->col_color.B=0.8+cosf( m_time_this_cycle*3 )/5;

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }


            //display
            m_score_display_left.draw();
            m_score_display_right.draw();

        }break;

        case state_game_over:
        {
            if(m_debug_mode)//debug drawing
            {
                glColor3f(1,1,1);
                b2Body* tmp=m_World->GetBodyList();
                while(tmp)
                {
                    glPushMatrix();
                    b2Vec2 center=tmp->GetWorldCenter();
                    b2Vec2 mass_center=tmp->GetLocalCenter();

                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(tmp->GetAngle()*180/3.14159,0,0,1);
                    //dont shift circles
                    if( ((b2Shape*)tmp->GetFixtureList()->GetShape())->m_type != 0 )
                    {
                        glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);//shift to rotation center
                    }

                    for( b2Fixture* fixture=tmp->GetFixtureList(); fixture; fixture=fixture->GetNext() )
                    {
                        //test if sensor
                        if( fixture->IsSensor() )
                        {
                            //red
                            glColor3f(1,0.1,0.1);
                        }
                        else
                        {
                            //white
                            glColor3f(1,1,1);
                        }


                        //test if circle
                        if( ((b2Shape*)fixture->GetShape())->m_type == 0 )
                        {
                            float radius=((b2Shape*)fixture->GetShape())->m_radius;
                            glBegin(GL_TRIANGLE_STRIP);
                            //glVertex2f(0,0);
                            for(float i=0.0;i<=360;i+=36)
                            {
                                glVertex2f( cosf( i*3.14159/180 )*radius*_Met2Pix*m_screen_ratio_x, sinf( i*3.14159/180 )*radius*_Met2Pix*m_screen_ratio_y );
                            }
                            glEnd();
                        }
                        else //not circle
                        {
                            //int vertex_count=((b2PolygonShape*)tmp->GetFixtureList()->GetShape())->GetVertexCount();
                            int vertex_count=((b2PolygonShape*)fixture->GetShape())->GetVertexCount();
                            b2Vec2 points[vertex_count];

                            for(int i=0;i<vertex_count;i++)
                                points[i]=((b2PolygonShape*)fixture->GetShape())->GetVertex(i);

                            glBegin(GL_LINE_STRIP);
                            for(int i=0;i<vertex_count;i++)
                                glVertex2f(points[i].x*_Met2Pix*m_screen_ratio_x,points[i].y*_Met2Pix*m_screen_ratio_y);

                            glVertex2f(points[0].x*_Met2Pix*m_screen_ratio_x,points[0].y*_Met2Pix*m_screen_ratio_y);

                            glEnd();
                        }

                        //fixture=fixture->GetNext();
                    }

                    glPopMatrix();

                    tmp=tmp->GetNext();
                }
            }

            //draw walls
            for(int wall=0;wall<8;wall++)
            {
                if( m_walls[wall]!=NULL ) //draw wall
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_walls[wall]->GetUserData() );
                    glPushMatrix();

                    b2Vec2 center=m_walls[wall]->GetWorldCenter();
                    b2Vec2 mass_center=m_walls[wall]->GetLocalCenter();
                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(m_walls[wall]->GetAngle()*180/3.14159,0,0,1);
                    glTranslatef(-mass_center.x*_Met2Pix*m_screen_ratio_x,-mass_center.y*_Met2Pix*m_screen_ratio_y,0);

                    //get wall color
                    st_color col_first,col_second,col_third;
                    int current_color_index=wall+m_wall_color_shift;
                    while( current_color_index>=8 ) current_color_index-=8;
                    col_first=m_wall_colors[ current_color_index ];
                    int next_color_index=wall+m_wall_color_shift+1;
                    while( next_color_index>=8 ) next_color_index-=8;
                    col_second=m_wall_colors[ next_color_index ];
                    int third_color_index=wall+m_wall_color_shift+2;
                    while( third_color_index>=8 ) third_color_index-=8;
                    col_third=m_wall_colors[ third_color_index ];
                    //update current colors
                    float dR=col_second.R-col_first.R;
                    float dG=col_second.G-col_first.G;
                    float dB=col_second.B-col_first.B;
                    col_first.R+=dR*m_wall_color_progress;
                    col_first.G+=dG*m_wall_color_progress;
                    col_first.B+=dB*m_wall_color_progress;
                    //update second color
                    dR=col_third.R-col_second.R;
                    dG=col_third.G-col_second.G;
                    dB=col_third.B-col_second.B;
                    col_second.R+=dR*m_wall_color_progress;
                    col_second.G+=dG*m_wall_color_progress;
                    col_second.B+=dB*m_wall_color_progress;

                    glColor4f(1,1,1,1);//temp

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_texture);

                    glBegin(GL_QUADS);
                    for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                    {
                        if(i<2)//first color
                        {
                            glColor3f( col_first.R*col_first.brightness,
                                       col_first.G*col_first.brightness,
                                       col_first.B*col_first.brightness );
                        }
                        else//second color
                        {
                            glColor3f( col_second.R*col_second.brightness,
                                       col_second.G*col_second.brightness,
                                       col_second.B*col_second.brightness );
                        }

                        glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                        glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                    }
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    glPopMatrix();
                }
            }

            //draw flippers
            for(int player_index=0;player_index<4;player_index++)
            {
                if( m_player_ingame[player_index] ) //draw flippers
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_p_flipper_body[ player_index ]->GetUserData() );
                    glPushMatrix();

                    b2Vec2 center=m_p_flipper_body[ player_index ]->GetWorldCenter();
                    b2Vec2 mass_center=m_p_flipper_body[ player_index ]->GetLocalCenter();
                    glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                    glRotatef(m_p_flipper_body[ player_index ]->GetAngle()*180/3.14159,0,0,1);

                    glColor3f( data->col_color.R,data->col_color.G,data->col_color.B );

                    //update colors
                    switch(player_index)
                    {
                        case 0:
                        {
                            data->col_color.R=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.G=0.1+cosf( m_time_this_cycle*2.0 )/20.0;
                            data->col_color.B=0.1+cosf( m_time_this_cycle*3.0 )/20.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+1)*cosf(m_time_this_cycle*1*data->col_update_speed+86);
                        }break;

                        case 1:
                        {
                            data->col_color.R=0.2+cosf( m_time_this_cycle*1.0 )/10.0;
                            data->col_color.G=0.2+cosf( m_time_this_cycle*2.0 )/10.0;
                            data->col_color.B=0.8+cosf( m_time_this_cycle*3.0 )/5.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+16)*cosf(m_time_this_cycle*1*data->col_update_speed+110);
                        }break;

                        case 2:
                        {
                            data->col_color.R=0.2+cosf( m_time_this_cycle*1.0 )/10.0;
                            data->col_color.G=0.8+cosf( m_time_this_cycle*2.0 )/5.0;
                            data->col_color.B=0.2+cosf( m_time_this_cycle*3.0 )/10.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+24)*cosf(m_time_this_cycle*1*data->col_update_speed+432);
                        }break;

                        case 3:
                        {
                            data->col_color.R=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.G=0.8+cosf( m_time_this_cycle*1.0 )/5.0;
                            data->col_color.B=0.2+cosf( m_time_this_cycle*3.0 )/10.0;
                            data->col_color.brightness=0.8+0.2*cosf(m_time_this_cycle*4*data->col_update_speed+123)*cosf(m_time_this_cycle*1*data->col_update_speed+3);
                        }break;
                    }

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_texture);

                    glBegin(GL_TRIANGLE_STRIP);
                    //glVertex2f(0,0);
                    for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                    {
                        glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                        glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                    }
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    glPopMatrix();
                }
            }

            //draw ball
            if(m_ball!=NULL)
            {
                st_body_user_data* data_ball=static_cast<st_body_user_data*>( m_ball->GetUserData() );
                glPushMatrix();
                b2Vec2 center=m_ball->GetWorldCenter();
                b2Vec2 mass_center=m_ball->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);
                glRotatef(m_ball->GetAngle()*180/3.14159,0,0,1);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                //update color
                data_ball->col_color.R=0.6+cosf( m_time_this_cycle*1 )/5;
                data_ball->col_color.G=0.6+cosf( m_time_this_cycle*2 )/5;
                data_ball->col_color.B=0.6+cosf( m_time_this_cycle*3 )/5;

                glColor3f(data_ball->col_color.R,data_ball->col_color.G,data_ball->col_color.B);

                glBegin(GL_TRIANGLE_FAN);
                for( int i=0;i<(int)data_ball->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data_ball->vec_texture_points[i].x, data_ball->vec_texture_points[i].y );
                    glVertex2f( data_ball->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data_ball->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }

            //draw goals
            if(m_goal_left!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_goal_left->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_goal_left->GetWorldCenter();
                b2Vec2 mass_center=m_goal_left->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_goal_left->GetAngle()*180/3.14159,0,0,1);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glColor3f(data->col_color.R,data->col_color.G,data->col_color.B);

                //update color
                data->col_color.R=0.8+cosf( m_time_this_cycle*1 )/5;
                data->col_color.G=0.1+cosf( m_time_this_cycle*2 )/10;
                data->col_color.B=0.1+cosf( m_time_this_cycle*3 )/10;

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }

            if(m_goal_right!=NULL)
            {
                st_body_user_data* data=static_cast<st_body_user_data*>( m_goal_right->GetUserData() );
                glPushMatrix();

                b2Vec2 center=m_goal_right->GetWorldCenter();
                b2Vec2 mass_center=m_goal_right->GetLocalCenter();
                glTranslatef(center.x*_Met2Pix*m_screen_ratio_x,center.y*_Met2Pix*m_screen_ratio_y,0);//go to body's center
                glRotatef(m_goal_right->GetAngle()*180/3.14159,0,0,1);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_texture);

                glColor3f(data->col_color.R,data->col_color.G,data->col_color.B);

                //update color
                data->col_color.R=0.1+cosf( m_time_this_cycle*2 )/10;
                data->col_color.G=0.1+cosf( m_time_this_cycle*4 )/10;
                data->col_color.B=0.8+cosf( m_time_this_cycle*3 )/5;

                glBegin(GL_TRIANGLE_STRIP);
                for( int i=0;i<(int)data->vec_vertex_points.size();i++ )
                {
                    glTexCoord2f( data->vec_texture_points[i].x, data->vec_texture_points[i].y );
                    glVertex2f( data->vec_vertex_points[i].x*_Met2Pix*m_screen_ratio_x, data->vec_vertex_points[i].y*_Met2Pix*m_screen_ratio_y );
                }
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                glPopMatrix();
            }


            //display
            m_score_display_left.draw();
            m_score_display_right.draw();

            //game over sign
            bool draw_outro_shade=false;
            float progress=1.0-m_game_over_tick/_game_over_delay;
            float brightness=progress;
            if(brightness>0.5)
            {
                draw_outro_shade=true;
                brightness=1.0;
            }
            float color_mask[]={brightness,brightness,brightness};//update brightness
            m_dec_game_over.set_color_mask(color_mask);
            m_dec_game_over.zoom( 1.0+(m_time_this_cycle-m_time_last_cycle)*0.01 );//update size
            m_dec_game_over.draw();
            //draw outro shade
            if(draw_outro_shade)
            {
                //draw transparent black square
                float blackness=(progress-0.5)/0.5;
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0,0,0,blackness);//black
                glBegin(GL_QUADS);
                glVertex2f(0,0);
                glVertex2f(0,m_window_height);
                glVertex2f(m_window_width,m_window_height);
                glVertex2f(m_window_width,0);
                glEnd();
                glDisable(GL_BLEND);
            }

        }break;
    }

    /*//motion blur demo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    frame->draw();
    // q is a suited float (about .90 to .99)
    glAccum(GL_MULT, q);
    glAccum(GL_ACCUM, 1-q);
    glAccum(GL_RETURN, 1.0);
    glFlush();
    glutSwapBuffers();
    */


    return true;
}

bool game::cycle(bool keys[],bool mouse_but[],int mouse_pos[])
{
    m_time_last_cycle=m_time_this_cycle;//store time for last cycle
    m_time_this_cycle=(float)clock()/CLOCKS_PER_SEC;//get time now, in sec

    //sound timer
    if(m_col_sound_cooldown>0) m_col_sound_cooldown-=m_time_this_cycle-m_time_last_cycle;

    //music update
    if(!m_music_loop_started && m_music_enabled)
    {
        if(m_music_source==-1)//if error, start loop
        {
            m_music_source=m_p_SoundEngine->playSimpleSound(wav_music_intro,1.0,20,false);
        }
        else
        {
            //test if intro done, start loop
            if( m_p_SoundEngine->get_source_status(20) )
            {
                m_p_SoundEngine->playSimpleSound(wav_music_loop,1.0,20,true);
                m_music_loop_started=true;
            }
        }
    }

    //toggle music
    if(m_key_delay>0) m_key_delay-=m_time_this_cycle-m_time_last_cycle;
    if( keys[77] && m_key_delay<=0.0)
    {
        m_key_delay=_key_delay;
        //pause/resume sound source 20
        m_music_enabled=!m_music_enabled;
        if(m_music_enabled)
        {
            m_p_SoundEngine->resume_source(20);
        }
        else
        {
            m_p_SoundEngine->pause_source(20);
        }
    }

    switch(m_state)
    {
        case state_lobby:
        {

            //screen blend
            if( m_intro_shade_tick>=0 )
            {
                m_intro_shade_tick-=m_time_this_cycle-m_time_last_cycle;
                m_intro_shade_progress=m_intro_shade_tick/_intro_shade_delay;

                if(m_intro_shade_tick<0)
                {
                    m_intro_shade_progress=1;
                }
            }
            else if( m_info_box_shade_tick>0 ) //start infobox shade after blend done
            {
                m_info_box_shade_tick-=m_time_this_cycle-m_time_last_cycle;
                m_info_box_shade_progress=1.0-m_info_box_shade_tick/_info_box_shade_delay;

                if(m_info_box_shade_tick<0)
                {
                    m_info_box_shade_progress=1;
                }
            }

            //countdown timer
            if( m_game_start_requested && m_game_start_tick>0 )
            {
                m_game_start_tick-=m_time_this_cycle-m_time_last_cycle;

                //update brightness of infoboxes
                for(int i=0;i<4;i++)
                {
                    st_body_user_data* data=static_cast<st_body_user_data*>( m_info_bodies[i]->GetUserData() );
                    data->col_color.brightness=(_game_start_delay-m_game_start_tick) / (_game_start_delay/3);
                    if( data->col_color.brightness>1.0 ) data->col_color.brightness=1.0;//roof
                    data->col_color.brightness= 1.0 - data->col_color.brightness;//invert
                }

                if( m_game_start_tick<=0 )
                {//start game
                    m_game_start_requested=false;
                    //remove all players from menu world
                    for(int p=0;p<4;p++)
                    {
                        if( m_connected_player[p] && m_player_ingame[p] ) remove_player(p);
                    }
                    //add players to game world
                    for(int p=0;p<4;p++)
                    {
                        if( m_connected_player[p] && m_player_ingame[p] ) add_new_player(p,m_World);
                    }
                    m_state=state_running;
                    m_game_start_tick=_game_start_delay;
                    reset_board();
                    break;
                }
                else//update countdown timer
                {
                    if( m_countdown_timer.get_number() != (int)m_game_start_tick+1 )
                    {//update value
                        m_countdown_timer.set_number( (int)m_game_start_tick+1 );
                        //play sound
                        m_p_SoundEngine->playSimpleSound(wav_ping,0.5);
                    }
                }
            }

            //gamepad input
            st_gamepad_data gamepad_data[4];
            for(int i=0;i<4;i++)
            {
                if( m_gamepad[i].IsConnected() )
                {
                    gamepad_data[i]=m_gamepad[i].GetState();
                    if( !m_connected_player[i] )//add new player
                    {
                        cout<<"Controller "<<i<<" connected\n";
                    }
                    m_connected_player[i]=true;
                }
                else
                {
                    if( m_connected_player[i] )//lost player
                    {
                        cout<<"Lost connection to controller "<<i<<endl;
                        if( m_player_ingame[i] )//player needs to be removed
                        {
                            remove_player(i);
                        }
                    }
                    m_connected_player[i]=false;
                    m_player_ingame[i]=false;
                }
            }

            for(int i=0;i<4;i++)
            {
                if( m_connected_player[i] )
                {
                    if( !m_player_ingame[i] )
                    {
                        if( gamepad_data[i].button_A )//join game
                        {
                            add_new_player(i,m_Menu_World);
                            m_player_ingame[i]=true;
                        }
                        else continue;//skip input from player not in game
                    }

                    //move flipper
                    m_p_flipper_body[ i ]->ApplyForce( b2Vec2( gamepad_data[i].thumbstick_left_x/400*_move_speed,
                                                              -gamepad_data[i].thumbstick_left_y/400*_move_speed ),
                                                       m_p_flipper_body[ i ]->GetWorldCenter() );
                    //rotate flipper
                    float target_angle=atan2f( -gamepad_data[i].thumbstick_right_x , -gamepad_data[i].thumbstick_right_y );
                    float force = ( fabs(gamepad_data[i].thumbstick_right_x) + fabs(gamepad_data[i].thumbstick_right_y) ) / 65535 * _rotate_speed;
                    float bodyAngle=m_p_flipper_body[ i ]->GetAngle();
                    float totalRotation = target_angle - bodyAngle;
                    while ( totalRotation < -180 * 0.0174532 ) totalRotation += 360 * 0.0174532;
                    while ( totalRotation >  180 * 0.0174532 ) totalRotation -= 360 * 0.0174532;
                    m_p_flipper_body[ i ]->ApplyTorque( totalRotation < 0 ? -800*force : 800*force );
                    //test buttons
                    if( gamepad_data[i].button_start && !m_game_start_requested && m_intro_shade_tick<_intro_shade_delay-2.0 ) //start game
                    {//can only start if intro is done
                        cout<<"Starting game in "<<_game_start_delay<<" seconds\n";
                        m_game_start_requested=true;
                    }
                    if( gamepad_data[i].button_back ) //leave request
                    {
                        m_leave_timer[i]-=m_time_this_cycle-m_time_last_cycle;
                        if(m_leave_timer[i]<=0)
                        {//leave
                            remove_player(i);
                            m_player_ingame[i]=false;
                            m_connected_player[i]=false;
                            m_leave_timer[i]=_leave_delay;
                        }
                    }
                    else m_leave_timer[i]=_leave_delay;

                }
            }

            m_Menu_World->Step( m_time_this_cycle-m_time_last_cycle,5,5 );

            if(*m_p_event_flag!=0) handle_event();
            if(*m_p_sound_flag!=-1) handle_sound_event();

        }break;

        case state_running:
        {
            //board reset timer
            if(m_reset_board)
            {
                m_reset_board_tick-=m_time_this_cycle-m_time_last_cycle;
                if(m_reset_board_tick<=0)
                {
                    m_reset_board=false;
                    reset_board();
                }
            }

            //update wall colors
            m_wall_color_tick-=m_time_this_cycle-m_time_last_cycle;
            m_wall_color_progress=1.0-m_wall_color_tick/_wall_color_delay;
            if( m_wall_color_tick<=0 )//reset timer
            {
                m_wall_color_tick=_wall_color_delay;
                m_wall_color_progress=0;
                //shift colors
                m_wall_color_shift++;
                if(m_wall_color_shift>=8) m_wall_color_shift=0;//only 8 colors, go back to 0
            }

            //gamepad input
            st_gamepad_data gamepad_data[4];
            for(int i=0;i<4;i++)
            {
                if( m_gamepad[i].IsConnected() )
                {
                    gamepad_data[i]=m_gamepad[i].GetState();
                    if( !m_connected_player[i] )//add new player
                    {
                        cout<<"Controller "<<i<<" connected\n";
                    }
                    m_connected_player[i]=true;
                }
                else
                {
                    if( m_connected_player[i] )//lost player
                    {
                        cout<<"Lost connection to controller "<<i<<endl;
                        if( m_player_ingame[i] )//player needs to be removed
                        {
                            remove_player(i);
                        }
                    }
                    m_connected_player[i]=false;
                    m_player_ingame[i]=false;
                }
            }

            for(int i=0;i<4;i++)
            {
                if( m_connected_player[i] )
                {
                    if( !m_player_ingame[i] )
                    {
                        if( gamepad_data[i].button_A )//join game
                        {
                            add_new_player(i,m_World);
                            m_player_ingame[i]=true;
                        }
                        else continue;//skip input from player not in game
                    }

                    //move flipper
                    m_p_flipper_body[ i ]->ApplyForce( b2Vec2( gamepad_data[i].thumbstick_left_x/400*_move_speed,
                                                              -gamepad_data[i].thumbstick_left_y/400*_move_speed ),
                                                       m_p_flipper_body[ i ]->GetWorldCenter() );
                    //rotate flipper
                    float target_angle=atan2f( -gamepad_data[i].thumbstick_right_x , -gamepad_data[i].thumbstick_right_y );
                    float force = ( fabs(gamepad_data[i].thumbstick_right_x) + fabs(gamepad_data[i].thumbstick_right_y) ) / 65535 * _rotate_speed;
                    float bodyAngle=m_p_flipper_body[ i ]->GetAngle();
                    float totalRotation = target_angle - bodyAngle;
                    while ( totalRotation < -180 * 0.0174532 ) totalRotation += 360 * 0.0174532;
                    while ( totalRotation >  180 * 0.0174532 ) totalRotation -= 360 * 0.0174532;
                    m_p_flipper_body[ i ]->ApplyTorque( totalRotation < 0 ? -800*force : 800*force );
                    //test buttons

                    if( gamepad_data[i].button_back ) //leave request
                    {
                        m_leave_timer[i]-=m_time_this_cycle-m_time_last_cycle;
                        if(m_leave_timer[i]<=0)
                        {//leave
                            remove_player(i);
                            m_player_ingame[i]=false;
                            m_connected_player[i]=false;
                            m_leave_timer[i]=_leave_delay;
                        }
                    }
                    else m_leave_timer[i]=_leave_delay;
                }
            }

            //test if any player ingame
            if( !m_player_ingame[0] && !m_player_ingame[1] && !m_player_ingame[2] && !m_player_ingame[3] && m_state!=state_game_over )
            {//no players ingame -> reset game
                game_over("No");
                break;
            }

            m_World->Step( m_time_this_cycle-m_time_last_cycle,5,5 );

            if(*m_p_event_flag!=0) handle_event();
            if(*m_p_sound_flag!=-1) handle_sound_event();

        }break;

        case state_game_over:
        {
            //game reset timer
            m_game_over_tick-=m_time_this_cycle-m_time_last_cycle;
            if(m_game_over_tick<=0)
            {//clean up
                cout<<"clean up\n";
                m_reset_board=false;
                //reset_board();
                reset_main_menu();
                break;
            }

            //update wall colors
            m_wall_color_tick-=m_time_this_cycle-m_time_last_cycle;
            m_wall_color_progress=1.0-m_wall_color_tick/_wall_color_delay;
            if( m_wall_color_tick<=0 )//reset timer
            {
                m_wall_color_tick=_wall_color_delay;
                m_wall_color_progress=0;
                //shift colors
                m_wall_color_shift++;
                if(m_wall_color_shift>=8) m_wall_color_shift=0;//only 8 colors, go back to 0
            }

            //get gamepad input
            st_gamepad_data gamepad_data[4];
            for(int i=0;i<4;i++)
            {
                if( m_gamepad[i].IsConnected() )
                {
                    gamepad_data[i]=m_gamepad[i].GetState();
                    m_connected_player[i]=true;
                }
                else
                {
                    if( m_connected_player[i] )//lost player
                    {
                        cout<<"Lost connection to controller "<<i<<endl;
                        if( m_player_ingame[i] )//player needs to be removed
                        {
                            remove_player(i);
                        }
                    }
                    m_connected_player[i]=false;
                    m_player_ingame[i]=false;
                }
            }

            //input result
            for(int i=0;i<4;i++)
            {
                if( m_connected_player[i] )
                {
                    //move flipper
                    m_p_flipper_body[ i ]->ApplyForce( b2Vec2( gamepad_data[i].thumbstick_left_x/400*_move_speed,
                                                              -gamepad_data[i].thumbstick_left_y/400*_move_speed ),
                                                       m_p_flipper_body[ i ]->GetWorldCenter() );
                    //rotate flipper
                    float target_angle=atan2f( -gamepad_data[i].thumbstick_right_x , -gamepad_data[i].thumbstick_right_y );
                    float force = ( fabs(gamepad_data[i].thumbstick_right_x) + fabs(gamepad_data[i].thumbstick_right_y) ) / 65535 * _rotate_speed;
                    float bodyAngle=m_p_flipper_body[ i ]->GetAngle();
                    float totalRotation = target_angle - bodyAngle;
                    while ( totalRotation < -180 * 0.0174532 ) totalRotation += 360 * 0.0174532;
                    while ( totalRotation >  180 * 0.0174532 ) totalRotation -= 360 * 0.0174532;
                    m_p_flipper_body[ i ]->ApplyTorque( totalRotation < 0 ? -800*force : 800*force );
                    //test buttons

                }
            }

            m_World->Step( m_time_this_cycle-m_time_last_cycle,5,5 );

        }break;
    }


    draw();

    return true;
}

//PRIVATE FUNC

bool game::init_box2d(void)
{
    //create world
    bool doSleep=true;
    m_World=new b2World( b2Vec2(0.0,0.0),doSleep );

    init_main_menu(35.0,18.0,100.0);

    //In-game world
    float xpos; float ypos;
    float width; float height;
    //create walls 0
    b2BodyDef wallbodydef;
    wallbodydef.position.Set( 960.0*_Pix2Met, 1058.4*_Pix2Met);
    wallbodydef.type=b2_staticBody;
    m_walls[0]=m_World->CreateBody(&wallbodydef);
    b2PolygonShape shape;
    shape.SetAsEdge( b2Vec2( -576.0*_Pix2Met,0 ),b2Vec2( 576.0*_Pix2Met,0 ) );
    b2FixtureDef wallfixturedef;
    wallfixturedef.shape=&shape;
    wallfixturedef.density=0.0;
    m_walls[0]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall0=new st_body_user_data;
    data_wall0->s_info="Wall";
    float extra_x=16.0;
    float extra_y=5.0;
    float extra_xy=4.0;
    data_wall0->vec_vertex_points.push_back( st_point( (-576.0-extra_x)*_Pix2Met, -10.0*_Pix2Met ) );
    data_wall0->vec_vertex_points.push_back( st_point( (-576.0-extra_x)*_Pix2Met, 10.0*_Pix2Met ) );
    data_wall0->vec_vertex_points.push_back( st_point( (576.0+extra_x)*_Pix2Met, 10.0*_Pix2Met ) );
    data_wall0->vec_vertex_points.push_back( st_point( (576.0+extra_x)*_Pix2Met, -10.0*_Pix2Met ) );
    data_wall0->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall0->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall0->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall0->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    m_walls[0]->SetUserData(data_wall0);

    //wall 1 (4)
    wallbodydef.position.Set( 960.0*_Pix2Met, 21.6*_Pix2Met );
    m_walls[4]=m_World->CreateBody(&wallbodydef);
    m_walls[4]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall4=new st_body_user_data;
    data_wall4->s_info="Wall";

    data_wall4->vec_vertex_points.push_back( st_point( (576.0+extra_x)*_Pix2Met, 10.0*_Pix2Met ) );
    data_wall4->vec_vertex_points.push_back( st_point( (576.0+extra_x)*_Pix2Met, -10.0*_Pix2Met ) );
    data_wall4->vec_vertex_points.push_back( st_point( (-576.0-extra_x)*_Pix2Met, -10.0*_Pix2Met ) );
    data_wall4->vec_vertex_points.push_back( st_point( (-576.0-extra_x)*_Pix2Met, 10.0*_Pix2Met ) );

    data_wall4->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall4->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall4->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall4->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    m_walls[4]->SetUserData(data_wall4);

    //wall 2 (6)
    wallbodydef.position.Set( 38.4*_Pix2Met , 540.0*_Pix2Met);
    m_walls[6]=m_World->CreateBody(&wallbodydef);
    shape.SetAsEdge( b2Vec2(0, 237.6*_Pix2Met ),b2Vec2(0, -237.6*_Pix2Met ) );
    m_walls[6]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall6=new st_body_user_data;
    data_wall6->s_info="Wall";
    data_wall6->vec_vertex_points.push_back( st_point( 10.0*_Pix2Met, (-237.6-extra_y)*_Pix2Met ) );
    data_wall6->vec_vertex_points.push_back( st_point( -10.0*_Pix2Met, (-237.6-extra_y)*_Pix2Met ) );
    data_wall6->vec_vertex_points.push_back( st_point( -10.0*_Pix2Met, (237.6+extra_y)*_Pix2Met ) );
    data_wall6->vec_vertex_points.push_back( st_point( 10.0*_Pix2Met, (237.6+extra_y)*_Pix2Met ) );
    data_wall6->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall6->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall6->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall6->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    m_walls[6]->SetUserData(data_wall6);

    //wall 3 (2)
    wallbodydef.position.Set( 1881.6*_Pix2Met , 540.0*_Pix2Met );
    m_walls[2]=m_World->CreateBody(&wallbodydef);
    m_walls[2]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall2=new st_body_user_data;
    data_wall2->s_info="Wall";

    data_wall2->vec_vertex_points.push_back( st_point( -10.0*_Pix2Met, (237.6+extra_y)*_Pix2Met ) );
    data_wall2->vec_vertex_points.push_back( st_point( 10.0*_Pix2Met, (237.6+extra_y)*_Pix2Met ) );
    data_wall2->vec_vertex_points.push_back( st_point( 10.0*_Pix2Met, (-237.6-extra_y)*_Pix2Met ) );
    data_wall2->vec_vertex_points.push_back( st_point( -10.0*_Pix2Met, (-237.6-extra_y)*_Pix2Met ) );

    data_wall2->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall2->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall2->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall2->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    m_walls[2]->SetUserData(data_wall2);

    //wall 4 (7)
    wallbodydef.position.Set( 211.2*_Pix2Met, 918.0*_Pix2Met );
    m_walls[7]=m_World->CreateBody(&wallbodydef);
    shape.SetAsEdge( b2Vec2( -172.8*_Pix2Met,-140.4*_Pix2Met ),b2Vec2( 172.8*_Pix2Met, 140.4*_Pix2Met ) );
    m_walls[7]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall7=new st_body_user_data;
    data_wall7->s_info="Wall";
    data_wall7->vec_vertex_points.push_back( st_point( (-172.8+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (-140.4-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall7->vec_vertex_points.push_back( st_point( (-172.8-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (-140.4+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall7->vec_vertex_points.push_back( st_point( (172.8-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (140.4+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall7->vec_vertex_points.push_back( st_point( (172.8+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (140.4-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall7->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall7->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall7->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall7->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    m_walls[7]->SetUserData(data_wall7);

    //wall 5 (1)
    wallbodydef.position.Set( 1708.8*_Pix2Met, 918.0*_Pix2Met);
    m_walls[1]=m_World->CreateBody(&wallbodydef);
    shape.SetAsEdge( b2Vec2( -172.8*_Pix2Met,140.4*_Pix2Met ),b2Vec2( 172.8*_Pix2Met, -140.4*_Pix2Met ) );
    m_walls[1]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall1=new st_body_user_data;
    data_wall1->s_info="Wall";
    data_wall1->vec_vertex_points.push_back( st_point( (-172.8-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (140.4-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall1->vec_vertex_points.push_back( st_point( (-172.8+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (140.4+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall1->vec_vertex_points.push_back( st_point( (172.8+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (-140.4+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall1->vec_vertex_points.push_back( st_point( (172.8-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (-140.4-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall1->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall1->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall1->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall1->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    m_walls[1]->SetUserData(data_wall1);

    //wall 6 (3)
    wallbodydef.position.Set( 1708.8*_Pix2Met, 162.0*_Pix2Met );
    m_walls[3]=m_World->CreateBody(&wallbodydef);
    shape.SetAsEdge( b2Vec2( -172.8*_Pix2Met,-140.4*_Pix2Met ),b2Vec2( 172.8*_Pix2Met, 140.4*_Pix2Met ) );
    m_walls[3]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall3=new st_body_user_data;
    data_wall3->s_info="Wall";

    data_wall3->vec_vertex_points.push_back( st_point( (172.8-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (140.4+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall3->vec_vertex_points.push_back( st_point( (172.8+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (140.4-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall3->vec_vertex_points.push_back( st_point( (-172.8+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (-140.4-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall3->vec_vertex_points.push_back( st_point( (-172.8-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (-140.4+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );

    data_wall3->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall3->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall3->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall3->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    m_walls[3]->SetUserData(data_wall3);

    //wall 7 (5)
    wallbodydef.position.Set( 211.2*_Pix2Met, 162.0*_Pix2Met );
    m_walls[5]=m_World->CreateBody(&wallbodydef);
    shape.SetAsEdge( b2Vec2( -172.8*_Pix2Met,140.4*_Pix2Met ),b2Vec2( 172.8*_Pix2Met, -140.4*_Pix2Met ) );
    m_walls[5]->CreateFixture(&wallfixturedef);
    //data
    st_body_user_data* data_wall5=new st_body_user_data;
    data_wall5->s_info="Wall";

    data_wall5->vec_vertex_points.push_back( st_point( (172.8+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (-140.4+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall5->vec_vertex_points.push_back( st_point( (172.8-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met, (-140.4-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met ) );
    data_wall5->vec_vertex_points.push_back( st_point( (-172.8-10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (140.4-10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );
    data_wall5->vec_vertex_points.push_back( st_point( (-172.8+10.0*cosf(45.0*_Deg2Rad)-extra_xy )*_Pix2Met, (140.4+10.0*cosf(45.0*_Deg2Rad)+extra_xy )*_Pix2Met ) );

    data_wall5->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-133.0/1024.0 ) );
    data_wall5->vec_texture_points.push_back( st_point( 1001.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall5->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-106.0/1024.0 ) );
    data_wall5->vec_texture_points.push_back( st_point( 124.0/1024.0,1.0-133.0/1024.0 ) );
    m_walls[5]->SetUserData(data_wall5);

    //create ball
    xpos=960.0*_Pix2Met; ypos=540.0*_Pix2Met;
    b2BodyDef bodydef_ball;
    bodydef_ball.position.Set(0,0);
    bodydef_ball.type=b2_dynamicBody;
    bodydef_ball.linearDamping=0.5;
    bodydef_ball.angularDamping=1;
    m_ball=m_World->CreateBody(&bodydef_ball);
    b2CircleShape shape_ball;
    shape_ball.m_p.Set(xpos,ypos);
    shape_ball.m_radius=0.52;
    b2FixtureDef fixturedef_ball;
    fixturedef_ball.shape=&shape_ball;
    fixturedef_ball.density=0.2;
    fixturedef_ball.restitution=0.5;
    m_ball->CreateFixture(&fixturedef_ball);

    st_body_user_data* data_ball=new st_body_user_data;
    data_ball->i_player_number=-1;
    data_ball->s_info="Ball";

    int numof_corners=10;
    float radius_vertex=0.8;
    float radius_texture=12.0;
    float texture_center_x=39.0/1024.0; float texture_center_y=1.0-125.0/1024.0;
    float step_size=360.0/numof_corners;
    //center pos
    data_ball->vec_vertex_points.push_back( st_point( 0.0, 0.0 ) );
    data_ball->vec_texture_points.push_back( st_point( texture_center_x,texture_center_y ) );
    for(float angle=0;angle<=360;angle+=step_size )
    {
        data_ball->vec_vertex_points.push_back( st_point( cosf(angle*_Deg2Rad)*radius_vertex, sinf(angle*_Deg2Rad)*radius_vertex ) );
        data_ball->vec_texture_points.push_back( st_point( (cosf(angle*_Deg2Rad)*radius_texture)/1024.0+texture_center_x,
                                                           (sinf(angle*_Deg2Rad)*radius_texture)/1024.0+texture_center_y ) );
    }

    data_ball->col_color.R=0.4;
    data_ball->col_color.G=0.6;
    data_ball->col_color.B=0.5;

    m_ball->SetUserData(data_ball);

    //create goals
    xpos=1632.0*_Pix2Met; ypos=540.0*_Pix2Met;
    b2BodyDef bodydef_goal;
    bodydef_goal.position.Set(xpos,ypos);
    bodydef_goal.type=b2_dynamicBody;
    bodydef_goal.linearDamping=2;
    bodydef_goal.angularDamping=2;
    b2Body* goal_body=m_World->CreateBody(&bodydef_goal);
    m_goal_right=goal_body;
    //create fixture
    b2PolygonShape shape_goal1,shape_goal2,shape_goal3;

    float factor=0.7;
    b2Vec2 p1g(-3.9*factor,6.5*factor); b2Vec2 p2g(-3.9*factor,5.7*factor);
    b2Vec2 p3g(1.7*factor,5.7*factor); b2Vec2 p4g(1.7*factor,6.5*factor);
    b2Vec2 p5g(0.8*factor,5.7*factor); b2Vec2 p6g(0.8*factor,-5.7*factor);
    b2Vec2 p7g(1.7*factor,-5.7*factor); b2Vec2 p8g(1.7*factor,5.7*factor);
    b2Vec2 p9g(-3.9*factor,-5.7*factor); b2Vec2 p10g(-3.9*factor,-6.5*factor);
    b2Vec2 p11g(1.7*factor,-6.5*factor); b2Vec2 p12g(1.7*factor,-5.7*factor);

    b2Vec2 arrg1[]={p1g,p2g,p3g,p4g};
    b2Vec2 arrg2[]={p5g,p6g,p7g,p8g};
    b2Vec2 arrg3[]={p9g,p10g,p11g,p12g};
    shape_goal1.Set(arrg1,4);
    b2FixtureDef fixturedef_goal1,fixturedef_goal2,fixturedef_goal3;
    fixturedef_goal1.shape=&shape_goal1;
    fixturedef_goal1.density=_goal_density;
    goal_body->CreateFixture(&fixturedef_goal1);
    shape_goal2.Set(arrg2,4);
    fixturedef_goal2.shape=&shape_goal2;
    fixturedef_goal2.density=_goal_density;
    goal_body->CreateFixture(&fixturedef_goal2);
    shape_goal3.Set(arrg3,4);
    fixturedef_goal3.shape=&shape_goal3;
    fixturedef_goal3.density=_goal_density;
    goal_body->CreateFixture(&fixturedef_goal3);
    //sensor part
    b2PolygonShape shape_goal_sensor;
    float factor_2=1.3;
    b2Vec2 sensor_arr[]={ b2Vec2(-1.5*factor_2,2.7*factor_2),
                          b2Vec2(-1.5*factor_2,-2.7*factor_2),
                          b2Vec2(0.1*factor_2,-2.7*factor_2),
                          b2Vec2(0.1*factor_2,2.7*factor_2) };
    shape_goal_sensor.Set(sensor_arr,4);
    b2FixtureDef fixturedef_goal_sensor;
    fixturedef_goal_sensor.shape=&shape_goal_sensor;
    fixturedef_goal_sensor.isSensor=true;
    m_goal_sensor_right=goal_body->CreateFixture(&fixturedef_goal_sensor);

    st_body_user_data* data_goal1=new st_body_user_data;
    data_goal1->i_player_number=-1;
    data_goal1->s_info="Goal";
    data_goal1->vec_vertex_points.push_back( st_point(-4.5*factor,7.2*factor) );
    data_goal1->vec_vertex_points.push_back( st_point(-4.5*factor,-7.1*factor) );
    data_goal1->vec_vertex_points.push_back( st_point(2.4*factor,7.2*factor) );
    data_goal1->vec_vertex_points.push_back( st_point(2.4*factor,-7.1*factor) );
    data_goal1->vec_texture_points.push_back( st_point(11.0/1024.0, 1.0-160.0/1024.0) );
    data_goal1->vec_texture_points.push_back( st_point(11.0/1024.0, 1.0-303.0/1024.0) );
    data_goal1->vec_texture_points.push_back( st_point(80.0/1024.0, 1.0-160.0/1024.0) );
    data_goal1->vec_texture_points.push_back( st_point(80.0/1024.0, 1.0-303.0/1024.0) );
    data_goal1->col_color.R=0.6;
    data_goal1->col_color.G=0.2;
    data_goal1->col_color.B=0.1;
    m_goal_right->SetUserData(data_goal1);

    //next goal
    xpos=288.0*_Pix2Met;
    bodydef_goal.position.Set(xpos,ypos);
    b2Body* goal_body2=m_World->CreateBody(&bodydef_goal);
    m_goal_left=goal_body2;
    //create fixture
    goal_body2->CreateFixture(&fixturedef_goal1);
    goal_body2->CreateFixture(&fixturedef_goal2);
    goal_body2->CreateFixture(&fixturedef_goal3);
    goal_body2->SetTransform( b2Vec2(xpos,ypos), 3.14159 );
    //create sensor
    b2PolygonShape shape_goal_sensor2;
    shape_goal_sensor2.Set(sensor_arr,4);
    b2FixtureDef fixturedef_goal_sensor2;
    fixturedef_goal_sensor2.shape=&shape_goal_sensor;
    fixturedef_goal_sensor2.isSensor=true;
    m_goal_sensor_left=goal_body2->CreateFixture(&fixturedef_goal_sensor2);

    st_body_user_data* data_goal2=new st_body_user_data;
    data_goal2->i_player_number=-1;
    data_goal2->s_info="Goal";
    data_goal2->vec_vertex_points.push_back( st_point(-4.5*factor,7.2*factor) );
    data_goal2->vec_vertex_points.push_back( st_point(-4.5*factor,-7.1*factor) );
    data_goal2->vec_vertex_points.push_back( st_point(2.4*factor,7.2*factor) );
    data_goal2->vec_vertex_points.push_back( st_point(2.4*factor,-7.1*factor) );
    data_goal2->vec_texture_points.push_back( st_point(11.0/1024.0, 1.0-160.0/1024.0) );
    data_goal2->vec_texture_points.push_back( st_point(11.0/1024.0, 1.0-303.0/1024.0) );
    data_goal2->vec_texture_points.push_back( st_point(80.0/1024.0, 1.0-160.0/1024.0) );
    data_goal2->vec_texture_points.push_back( st_point(80.0/1024.0, 1.0-303.0/1024.0) );
    data_goal2->col_color.R=0.2;
    data_goal2->col_color.G=0.7;
    data_goal2->col_color.B=0.7;
    m_goal_left->SetUserData(data_goal2);

    //init contact listener
    m_World->SetContactListener(&m_myContactListenerInstance);
    m_myContactListenerInstance.init(m_goal_sensor_left,m_goal_sensor_right,m_ball,&func,m_p_event_flag,m_p_sound_flag,m_p_sound_vol,m_body_pointers);
    //m_myContactListenerInstance.func_to_call=func;
    //m_myContactListenerInstance.func_to_call_set=true;

    return true;
}

bool game::init_main_menu(float start_xpos,float start_ypos, float size)
{
    m_Menu_World=new b2World( b2Vec2(0.0,0.0),true );
    m_Menu_World->SetContactListener(&m_myContactListenerInstance);
    m_myContactListenerInstance.init(m_goal_sensor_left,m_goal_sensor_right,m_ball,&func,m_p_event_flag,m_p_sound_flag,m_p_sound_vol,m_body_pointers);

    //Letters
    //float start_xpos=33; float start_ypos=18;
    float factor=size;//size of text
    float xpos,ypos;
    //F
    xpos=start_xpos+0*(factor); ypos=start_ypos+0*(factor);
    b2BodyDef bodydef_letter;
    bodydef_letter.position.Set(xpos,ypos);
    bodydef_letter.type=b2_dynamicBody;
    bodydef_letter.linearDamping=2;
    bodydef_letter.angularDamping=2;
    m_letter_F=m_Menu_World->CreateBody(&bodydef_letter);
    //create fixture
    b2PolygonShape shape_f_top,shape_f_left,shape_f_center;

    b2Vec2 p1f( (-81.5)/1024.0 *factor, ((-97.5)/1024.0) *factor);
    b2Vec2 p2f( (81.5)/1024.0 *factor, ((-97.5)/1024.0) *factor);
    b2Vec2 p3f( (75.5)/1024.0 *factor, ((-74.5)/1024.0) *factor);
    b2Vec2 p4f( (-34.5)/1024.0 *factor, ((-74.5)/1024.0) *factor);
    b2Vec2 p5f( (-12.5)/1024.0 *factor, ((-35.5)/1024.0) *factor);
    b2Vec2 p6f( (64.5)/1024.0 *factor, ((-35.5)/1024.0) *factor);
    b2Vec2 p7f( (54.5)/1024.0 *factor, ((-0.5)/1024.0) *factor);
    b2Vec2 p8f( (8.5)/1024.0 *factor, ((-0.5)/1024.0) *factor);
    b2Vec2 p9f( (41.5)/1024.0 *factor, ((57.5)/1024.0) *factor);
    b2Vec2 p10f( (30.5)/1024.0 *factor, ((97.5)/1024.0) *factor);

    b2Vec2 f_top[]={p1f,p2f,p3f,p4f};
    b2Vec2 f_left[]={p1f,p4f,p9f,p10f};
    b2Vec2 f_center[]={p5f,p6f,p7f,p8f};
    shape_f_top.Set(f_top,4);
    b2FixtureDef fixturedef_f_top,fixturedef_f_left,fixturedef_f_center;
    fixturedef_f_top.shape=&shape_f_top;
    fixturedef_f_top.density=1.0;
    m_letter_F->CreateFixture(&fixturedef_f_top);
    shape_f_left.Set(f_left,4);
    fixturedef_f_left.shape=&shape_f_left;
    fixturedef_f_left.density=1.0;
    m_letter_F->CreateFixture(&fixturedef_f_left);
    shape_f_center.Set(f_center,4);
    fixturedef_f_center.shape=&shape_f_center;
    fixturedef_f_center.density=1.0;
    m_letter_F->CreateFixture(&fixturedef_f_center);

    //data
    st_body_user_data* data_f=new st_body_user_data;
    data_f->i_player_number=-1;
    data_f->s_info="Letter";
    data_f->vec_vertex_points.push_back( st_point( -95.5/1024.0*factor,-110.5/1024.0*factor ) );
    data_f->vec_vertex_points.push_back( st_point( -95.5/1024.0*factor,115.5/1024.0*factor ) );
    data_f->vec_vertex_points.push_back( st_point( 101.5/1024.0*factor,-110.5/1024.0*factor ) );
    data_f->vec_vertex_points.push_back( st_point( 101.5/1024.0*factor,115.5/1024.0*factor ) );
    data_f->vec_texture_points.push_back( st_point( 17.0/1024.0,1.0-329.0/1024.0 ) );
    data_f->vec_texture_points.push_back( st_point( 17.0/1024.0,1.0-555.0/1024.0 ) );
    data_f->vec_texture_points.push_back( st_point( 214.0/1024.0,1.0-329.0/1024.0 ) );
    data_f->vec_texture_points.push_back( st_point( 214.0/1024.0,1.0-555.0/1024.0 ) );
    data_f->col_color.R=0.9;
    data_f->col_color.G=0.3;
    data_f->col_color.B=0.3;
    data_f->col_update_speed=1;
    m_letter_F->SetUserData(data_f);

    //L
    xpos=start_xpos+0.10*(factor); ypos=start_ypos+0.016*(factor);
    bodydef_letter.position.Set(xpos,ypos);
    m_letter_L=m_Menu_World->CreateBody(&bodydef_letter);
    //create fixture
    b2PolygonShape shape_l_left,shape_l_bottom;

    b2Vec2 p1l( (-7.5)/1024.0 *factor, ((-114.5)/1024.0) *factor);
    b2Vec2 p2l( (23.5)/1024.0 *factor, ((-114.5)/1024.0) *factor);
    b2Vec2 p3l( (-23.5)/1024.0 *factor, ((61.5)/1024.0) *factor);
    b2Vec2 p4l( (47.5)/1024.0 *factor, ((81.5)/1024.0) *factor);
    b2Vec2 p5l( (60.5)/1024.0 *factor, ((114.5)/1024.0) *factor);
    b2Vec2 p6l( (-60.5)/1024.0 *factor, ((81.5)/1024.0) *factor);

    b2Vec2 l_left[]={p1l,p2l,p3l,p6l};
    b2Vec2 l_bottom[]={p3l,p4l,p5l,p6l};
    shape_l_left.Set(l_left,4);
    b2FixtureDef fixturedef_l_left,fixturedef_l_bottom;
    fixturedef_l_left.shape=&shape_l_left;
    fixturedef_l_left.density=1.0;
    m_letter_L->CreateFixture(&fixturedef_l_left);
    shape_l_bottom.Set(l_bottom,4);
    fixturedef_l_bottom.shape=&shape_l_bottom;
    fixturedef_l_bottom.density=1.0;
    m_letter_L->CreateFixture(&fixturedef_l_bottom);

    //data
    st_body_user_data* data_l=new st_body_user_data;
    data_l->i_player_number=-1;
    data_l->s_info="Letter";
    data_l->vec_vertex_points.push_back( st_point( -84.5/1024.0*factor,-127.5/1024.0*factor ) );
    data_l->vec_vertex_points.push_back( st_point( -84.5/1024.0*factor,132.5/1024.0*factor ) );
    data_l->vec_vertex_points.push_back( st_point( 82.5/1024.0*factor,-127.5/1024.0*factor ) );
    data_l->vec_vertex_points.push_back( st_point( 82.5/1024.0*factor,132.5/1024.0*factor ) );
    data_l->vec_texture_points.push_back( st_point( 216.0/1024.0,1.0-329.0/1024.0 ) );
    data_l->vec_texture_points.push_back( st_point( 216.0/1024.0,1.0-589.0/1024.0 ) );
    data_l->vec_texture_points.push_back( st_point( 383.0/1024.0,1.0-329.0/1024.0 ) );
    data_l->vec_texture_points.push_back( st_point( 383.0/1024.0,1.0-589.0/1024.0 ) );
    data_l->col_color.R=0.2;
    data_l->col_color.G=0.9;
    data_l->col_color.B=0.9;
    data_l->col_update_speed=1;
    m_letter_L->SetUserData(data_l);

    //I
    xpos=start_xpos+0.14*(factor); ypos=start_ypos-0.007*(factor);
    bodydef_letter.position.Set(xpos,ypos);
    m_letter_I=m_Menu_World->CreateBody(&bodydef_letter);
    //create fixture
    b2PolygonShape shape_i_center;

    b2Vec2 p1i( (-6.5)/1024.0 *factor, ((-91.0)/1024.0) *factor);
    b2Vec2 p2i( (51.5)/1024.0 *factor, ((-92.0)/1024.0) *factor);
    b2Vec2 p3i( (1.5)/1024.0 *factor, ((92.0)/1024.0) *factor);
    b2Vec2 p4i( (-51.5)/1024.0 *factor, ((77.0)/1024.0) *factor);

    b2Vec2 i_center[]={p1i,p2i,p3i,p4i};
    shape_i_center.Set(i_center,4);
    b2FixtureDef fixturedef_i_center;
    fixturedef_i_center.shape=&shape_i_center;
    fixturedef_i_center.density=1.0;
    m_letter_I->CreateFixture(&fixturedef_i_center);

    //data
    st_body_user_data* data_i=new st_body_user_data;
    data_i->i_player_number=-1;
    data_i->s_info="Letter";
    data_i->vec_vertex_points.push_back( st_point( -73.5/1024.0*factor,-105.0/1024.0*factor ) );
    data_i->vec_vertex_points.push_back( st_point( -73.5/1024.0*factor,108.0/1024.0*factor ) );
    data_i->vec_vertex_points.push_back( st_point( 71.5/1024.0*factor,-105.0/1024.0*factor ) );
    data_i->vec_vertex_points.push_back( st_point( 71.5/1024.0*factor,108.0/1024.0*factor ) );
    data_i->vec_texture_points.push_back( st_point( 385.0/1024.0,1.0-329.0/1024.0 ) );
    data_i->vec_texture_points.push_back( st_point( 385.0/1024.0,1.0-542.0/1024.0 ) );
    data_i->vec_texture_points.push_back( st_point( 530.0/1024.0,1.0-329.0/1024.0 ) );
    data_i->vec_texture_points.push_back( st_point( 530.0/1024.0,1.0-542.0/1024.0 ) );
    data_i->col_color.R=0.9;
    data_i->col_color.G=0.9;
    data_i->col_color.B=0.2;
    data_i->col_update_speed=1;
    m_letter_I->SetUserData(data_i);

    //I dot
    xpos=start_xpos+0.17*(factor); ypos=start_ypos-0.13*(factor);
    bodydef_letter.position.Set(xpos,ypos);
    m_letter_Idot=m_Menu_World->CreateBody(&bodydef_letter);
    //create fixture
    b2PolygonShape shape_idot_center;

    b2Vec2 p1id( (-24.5)/1024.0 *factor, ((-17.5)/1024.0) *factor);
    b2Vec2 p2id( (33.5)/1024.0 *factor, ((-17.5)/1024.0) *factor);
    b2Vec2 p3id( (24.5)/1024.0 *factor, ((17.5)/1024.0) *factor);
    b2Vec2 p4id( (-33.5)/1024.0 *factor, ((17.5)/1024.0) *factor);

    b2Vec2 idot_center[]={p1id,p2id,p3id,p4id};
    shape_idot_center.Set(idot_center,4);
    b2FixtureDef fixturedef_idot_center;
    fixturedef_idot_center.shape=&shape_idot_center;
    fixturedef_idot_center.density=1.0;
    m_letter_Idot->CreateFixture(&fixturedef_idot_center);

    //data
    st_body_user_data* data_idot=new st_body_user_data;
    data_idot->i_player_number=-1;
    data_idot->s_info="Letter";
    data_idot->vec_vertex_points.push_back( st_point( -54.5/1024.0*factor,-35.5/1024.0*factor ) );
    data_idot->vec_vertex_points.push_back( st_point( -54.5/1024.0*factor,31.5/1024.0*factor ) );
    data_idot->vec_vertex_points.push_back( st_point( 45.5/1024.0*factor,-35.5/1024.0*factor ) );
    data_idot->vec_vertex_points.push_back( st_point( 45.5/1024.0*factor,31.5/1024.0*factor ) );
    data_idot->vec_texture_points.push_back( st_point( 434.0/1024.0,1.0-258.0/1024.0 ) );
    data_idot->vec_texture_points.push_back( st_point( 434.0/1024.0,1.0-325.0/1024.0 ) );
    data_idot->vec_texture_points.push_back( st_point( 534.0/1024.0,1.0-258.0/1024.0 ) );
    data_idot->vec_texture_points.push_back( st_point( 534.0/1024.0,1.0-325.0/1024.0 ) );
    data_idot->col_color.R=0.7;
    data_idot->col_color.G=0.7;
    data_idot->col_color.B=0.4;
    data_idot->col_update_speed=1;
    m_letter_Idot->SetUserData(data_idot);

    //P
    xpos=start_xpos+0.264*(factor); ypos=start_ypos+0.018*(factor);
    bodydef_letter.position.Set(xpos,ypos);
    m_letter_P=m_Menu_World->CreateBody(&bodydef_letter);
    //create fixture
    b2PolygonShape shape_p_left,shape_p_top,shape_p_center;

    b2Vec2 p1p( (-61.0)/1024.0 *factor, ((-115.5)/1024.0) *factor);
    b2Vec2 p2p( (111.0)/1024.0 *factor, ((-115.5)/1024.0) *factor);
    b2Vec2 p3p( (22.0)/1024.0 *factor, ((-92.5)/1024.0) *factor);
    b2Vec2 p4p( (-39.0)/1024.0 *factor, ((-92.5)/1024.0) *factor);
    b2Vec2 p5p( (-49.0)/1024.0 *factor, ((-51.5)/1024.0) *factor);
    b2Vec2 p6p( (-59.0)/1024.0 *factor, ((-16.5)/1024.0) *factor);
    b2Vec2 p7p( (-95.0)/1024.0 *factor, ((115.5)/1024.0) *factor);
    b2Vec2 p8p( (-111.0)/1024.0 *factor, ((70.5)/1024.0) *factor);

    b2Vec2 p_left[]={p1p,p4p,p7p,p8p};
    b2Vec2 p_top[]={p1p,p2p,p3p,p4p};
    b2Vec2 p_center[]={p3p,p2p,p6p,p5p};
    shape_p_left.Set(p_top,4);
    b2FixtureDef fixturedef_p_left,fixturedef_p_top,fixturedef_p_center;
    fixturedef_p_left.shape=&shape_p_left;
    fixturedef_p_left.density=1.0;
    m_letter_P->CreateFixture(&fixturedef_p_left);
    shape_p_top.Set(p_left,4);
    fixturedef_p_top.shape=&shape_p_top;
    fixturedef_p_top.density=1.0;
    m_letter_P->CreateFixture(&fixturedef_p_top);
    shape_p_center.Set(p_center,4);
    fixturedef_p_center.shape=&shape_p_center;
    fixturedef_p_center.density=1.0;
    m_letter_P->CreateFixture(&fixturedef_p_center);

    //data
    st_body_user_data* data_p=new st_body_user_data;
    data_p->i_player_number=-1;
    data_p->s_info="Letter";
    data_p->vec_vertex_points.push_back( st_point( -127.0/1024.0*factor,-128.5/1024.0*factor ) );
    data_p->vec_vertex_points.push_back( st_point( -127.0/1024.0*factor,131.5/1024.0*factor ) );
    data_p->vec_vertex_points.push_back( st_point( 139.0/1024.0*factor,-128.5/1024.0*factor ) );
    data_p->vec_vertex_points.push_back( st_point( 139.0/1024.0*factor,131.5/1024.0*factor ) );
    data_p->vec_texture_points.push_back( st_point( 534.0/1024.0,1.0-329.0/1024.0 ) );
    data_p->vec_texture_points.push_back( st_point( 534.0/1024.0,1.0-589.0/1024.0 ) );
    data_p->vec_texture_points.push_back( st_point( 800.0/1024.0,1.0-329.0/1024.0 ) );
    data_p->vec_texture_points.push_back( st_point( 800.0/1024.0,1.0-589.0/1024.0 ) );
    data_p->col_color.R=0.2;
    data_p->col_color.G=0.2;
    data_p->col_color.B=0.9;
    data_p->col_update_speed=1;
    m_letter_P->SetUserData(data_p);

    //Create info boxes
    float info_size=0.03;
    float start_xpos_info=46;
    float start_ypos_info=35;
    //0
    xpos=start_xpos_info+0.0*(info_size); ypos=start_ypos_info+0.0*(info_size);
    b2BodyDef bodydef_box;
    bodydef_box.position.Set(xpos,ypos);
    bodydef_box.type=b2_dynamicBody;
    bodydef_box.linearDamping=2;
    bodydef_box.angularDamping=2;
    m_info_bodies[0]=m_Menu_World->CreateBody(&bodydef_box);
    //create fixture
    b2PolygonShape shape_b0_left;
    shape_b0_left.SetAsBox(385.0/2.0*info_size,66.0/2.0*info_size);
    b2FixtureDef fixturedef_b0;
    fixturedef_b0.density=1.0;
    fixturedef_b0.shape=&shape_b0_left;
    m_info_bodies[0]->CreateFixture(&fixturedef_b0);

    //data
    st_body_user_data* data_b0=new st_body_user_data;
    data_b0->i_player_number=-1;
    data_b0->s_info="Text_box";
    data_b0->vec_vertex_points.push_back( st_point( -206.5*info_size, -50.0*info_size ) );
    data_b0->vec_vertex_points.push_back( st_point( -206.5*info_size, 47.0*info_size ) );
    data_b0->vec_vertex_points.push_back( st_point( 206.5*info_size, -50.0*info_size ) );
    data_b0->vec_vertex_points.push_back( st_point( 206.5*info_size, 47.0*info_size ) );
    data_b0->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(619.0/1024.0) ) );
    data_b0->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(716.0/1024.0) ) );
    data_b0->vec_texture_points.push_back( st_point( 426.0/1024.0, 1.0-(619.0/1024.0) ) );
    data_b0->vec_texture_points.push_back( st_point( 426.0/1024.0, 1.0-(716.0/1024.0) ) );
    data_b0->col_color.R=0.4;
    data_b0->col_color.G=0.4;
    data_b0->col_color.B=0.4;
    data_b0->col_update_speed=1.0;
    data_b0->col_color.brightness=0.0;
    m_info_bodies[0]->SetUserData(data_b0);

    //1
    xpos=start_xpos_info+0.0*(info_size); ypos=start_ypos_info+130.0*(info_size);
    bodydef_box.position.Set(xpos,ypos);
    m_info_bodies[1]=m_Menu_World->CreateBody(&bodydef_box);
    //create fixture
    b2PolygonShape shape_b1;
    shape_b1.SetAsBox(294.0*info_size,31.5*info_size);
    b2FixtureDef fixturedef_b1;
    fixturedef_b1.density=1.0;
    fixturedef_b1.shape=&shape_b1;
    m_info_bodies[1]->CreateFixture(&fixturedef_b1);

    //data
    st_body_user_data* data_b1=new st_body_user_data;
    data_b1->i_player_number=-1;
    data_b1->s_info="Text_box";
    data_b1->vec_vertex_points.push_back( st_point( -308.0*info_size, -46.5*info_size ) );
    data_b1->vec_vertex_points.push_back( st_point( -308.0*info_size, 43.5*info_size ) );
    data_b1->vec_vertex_points.push_back( st_point( 307.0*info_size, -46.5*info_size ) );
    data_b1->vec_vertex_points.push_back( st_point( 307.0*info_size, 43.5*info_size ) );
    data_b1->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(717.0/1024.0) ) );
    data_b1->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(807.0/1024.0) ) );
    data_b1->vec_texture_points.push_back( st_point( 628.0/1024.0, 1.0-(717.0/1024.0) ) );
    data_b1->vec_texture_points.push_back( st_point( 628.0/1024.0, 1.0-(807.0/1024.0) ) );
    data_b1->col_color.R=0.4;
    data_b1->col_color.G=0.4;
    data_b1->col_color.B=0.4;
    data_b1->col_update_speed=1.0;
    data_b1->col_color.brightness=0.0;
    m_info_bodies[1]->SetUserData(data_b1);

    //2
    xpos=start_xpos_info+0.0*(info_size); ypos=start_ypos_info+260.0*(info_size);
    bodydef_box.position.Set(xpos,ypos);
    m_info_bodies[2]=m_Menu_World->CreateBody(&bodydef_box);
    //create fixture
    b2PolygonShape shape_b2;
    shape_b2.SetAsBox(272.5*info_size,30.5*info_size);
    b2FixtureDef fixturedef_b2;
    fixturedef_b2.density=1.0;
    fixturedef_b2.shape=&shape_b2;
    m_info_bodies[2]->CreateFixture(&fixturedef_b2);

    //data
    st_body_user_data* data_b2=new st_body_user_data;
    data_b2->i_player_number=-1;
    data_b2->s_info="Text_box";
    data_b2->vec_vertex_points.push_back( st_point( -286.5*info_size, -45.5*info_size ) );
    data_b2->vec_vertex_points.push_back( st_point( -286.5*info_size, 44.5*info_size ) );
    data_b2->vec_vertex_points.push_back( st_point( 286.5*info_size, -45.5*info_size ) );
    data_b2->vec_vertex_points.push_back( st_point( 286.5*info_size, 44.5*info_size ) );
    data_b2->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(808.0/1024.0) ) );
    data_b2->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(898.0/1024.0) ) );
    data_b2->vec_texture_points.push_back( st_point( 586.0/1024.0, 1.0-(808.0/1024.0) ) );
    data_b2->vec_texture_points.push_back( st_point( 586.0/1024.0, 1.0-(898.0/1024.0) ) );
    data_b2->col_color.R=0.4;
    data_b2->col_color.G=0.4;
    data_b2->col_color.B=0.4;
    data_b2->col_update_speed=1.0;
    data_b2->col_color.brightness=0.0;
    m_info_bodies[2]->SetUserData(data_b2);

    //3
    xpos=start_xpos_info+0.0*(info_size); ypos=start_ypos_info+390.0*(info_size);
    bodydef_box.position.Set(xpos,ypos);
    m_info_bodies[3]=m_Menu_World->CreateBody(&bodydef_box);
    //create fixture
    b2PolygonShape shape_b3;
    shape_b3.SetAsBox(285.0*info_size,34.0*info_size);
    b2FixtureDef fixturedef_b3;
    fixturedef_b3.density=1.0;
    fixturedef_b3.shape=&shape_b3;
    m_info_bodies[3]->CreateFixture(&fixturedef_b3);

    //data
    st_body_user_data* data_b3=new st_body_user_data;
    data_b3->i_player_number=-1;
    data_b3->s_info="Text_box";
    data_b3->vec_vertex_points.push_back( st_point( -299.0*info_size, -50.0*info_size ) );
    data_b3->vec_vertex_points.push_back( st_point( -299.0*info_size, 48.0*info_size ) );
    data_b3->vec_vertex_points.push_back( st_point( 298.0*info_size, -50.0*info_size ) );
    data_b3->vec_vertex_points.push_back( st_point( 298.0*info_size, 48.0*info_size ) );
    data_b3->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(899.0/1024.0) ) );
    data_b3->vec_texture_points.push_back( st_point( 13.0/1024.0, 1.0-(997.0/1024.0) ) );
    data_b3->vec_texture_points.push_back( st_point( 610.0/1024.0, 1.0-(899.0/1024.0) ) );
    data_b3->vec_texture_points.push_back( st_point( 610.0/1024.0, 1.0-(997.0/1024.0) ) );
    data_b3->col_color.R=0.4;
    data_b3->col_color.G=0.4;
    data_b3->col_color.B=0.4;
    data_b3->col_update_speed=1.0;
    data_b3->col_color.brightness=0.0;
    m_info_bodies[3]->SetUserData(data_b3);

}

bool game::reset_main_menu(void)
{
    m_state=state_lobby;//go to lobby

    //reset score
    m_score_display_left.reset();
    m_score_display_right.reset();

    //reset game over sign
    m_dec_game_over.reset_zoom();

    //remove players from ingame world
    for(int p=0;p<4;p++)
    {
        if( m_connected_player[p] && m_player_ingame[p] ) remove_player(p);
        m_player_ingame[p]=false;
        m_connected_player[p]=false;
    }
    //add players to menu world
    /*for(int p=0;p<4;p++)
    {
        if( m_connected_player[p] && m_player_ingame[p] ) add_new_player(p,m_Menu_World);
        m_player_ingame[p]=true;
        m_connected_player[p]=true;
    }*/

    //reset letters
    float start_xpos=35.0; float start_ypos=18.0; float factor=100.0;
    //F
    m_letter_F->SetLinearVelocity(b2Vec2(0,0));
    m_letter_F->SetAngularVelocity(0);
    m_letter_F->SetTransform( b2Vec2( start_xpos+0*(factor), start_ypos+0*(factor) ) , 0 );
    //L
    m_letter_L->SetLinearVelocity(b2Vec2(0,0));
    m_letter_L->SetAngularVelocity(0);
    m_letter_L->SetTransform( b2Vec2( start_xpos+0.10*(factor), start_ypos+0.016*(factor) ) , 0 );
    //I
    m_letter_I->SetLinearVelocity(b2Vec2(0,0));
    m_letter_I->SetAngularVelocity(0);
    m_letter_I->SetTransform( b2Vec2( start_xpos+0.14*(factor), start_ypos-0.007*(factor) ) , 0 );
    //Idot
    m_letter_Idot->SetLinearVelocity(b2Vec2(0,0));
    m_letter_Idot->SetAngularVelocity(0);
    m_letter_Idot->SetTransform( b2Vec2( start_xpos+0.17*(factor), start_ypos-0.13*(factor) ) , 0 );
    //P
    m_letter_P->SetLinearVelocity(b2Vec2(0,0));
    m_letter_P->SetAngularVelocity(0);
    m_letter_P->SetTransform( b2Vec2( start_xpos+0.264*(factor), start_ypos+0.018*(factor) ) , 0 );

    //info bodies
    start_xpos=46.0; start_ypos=35.0; factor=0.03;
    //0
    st_body_user_data* data0=static_cast<st_body_user_data*>( m_info_bodies[0]->GetUserData() );
    data0->col_color.brightness=0.0;
    m_info_bodies[0]->SetLinearVelocity(b2Vec2(0,0));
    m_info_bodies[0]->SetAngularVelocity(0);
    m_info_bodies[0]->SetTransform( b2Vec2( start_xpos+0.0*(factor), start_ypos+0.0*(factor) ) , 0 );
    //1
    st_body_user_data* data1=static_cast<st_body_user_data*>( m_info_bodies[1]->GetUserData() );
    data1->col_color.brightness=0.0;
    m_info_bodies[1]->SetLinearVelocity(b2Vec2(0,0));
    m_info_bodies[1]->SetAngularVelocity(0);
    m_info_bodies[1]->SetTransform( b2Vec2( start_xpos+0.0*(factor), start_ypos+130.0*(factor) ) , 0 );
    //2
    st_body_user_data* data2=static_cast<st_body_user_data*>( m_info_bodies[2]->GetUserData() );
    data2->col_color.brightness=0.0;
    m_info_bodies[2]->SetLinearVelocity(b2Vec2(0,0));
    m_info_bodies[2]->SetAngularVelocity(0);
    m_info_bodies[2]->SetTransform( b2Vec2( start_xpos+0.0*(factor), start_ypos+260.0*(factor) ) , 0 );
    //3
    st_body_user_data* data3=static_cast<st_body_user_data*>( m_info_bodies[3]->GetUserData() );
    data3->col_color.brightness=0.0;
    m_info_bodies[3]->SetLinearVelocity(b2Vec2(0,0));
    m_info_bodies[3]->SetAngularVelocity(0);
    m_info_bodies[3]->SetTransform( b2Vec2( start_xpos+0.0*(factor), start_ypos+390.0*(factor) ) , 0 );

    //enable intro shade
    m_intro_shade_tick=_intro_shade_delay;
    m_info_box_shade_tick=_info_box_shade_delay;

    //play sound
    if(!m_music_enabled) m_p_SoundEngine->playSimpleSound(wav_intro,1.0);

    return true;
}

bool game::add_new_player(int player_number,b2World* world)
{
    cout<<"Added new player "<<player_number<<endl;

    int xpos=0; int ypos=15;
    switch(player_number)//decide position
    {
        case 0:
        {
            if(m_state==state_lobby)
            {
                xpos=15; ypos=15;
            }
            else
            {
                xpos=26; ypos=15;
            }

        }break;

        case 1:
        {
            if(m_state==state_lobby)
            {
                xpos=75; ypos=15;
            }
            else
            {
                xpos=70; ypos=15;
            }

        }break;

        case 2:
        {
            if(m_state==state_lobby)
            {
                xpos=25; ypos=35;
            }
            else
            {
                xpos=26; ypos=38;
            }

        }break;

        case 3:
        {
            if(m_state==state_lobby)
            {
                xpos=65; ypos=35;
            }
            else
            {
                xpos=70; ypos=38;
            }

        }break;
    }
    b2BodyDef bodydef;
    bodydef.position.Set(xpos,ypos);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=_move_damping;
    bodydef.angularDamping=_rotate_damping;
    m_p_flipper_body[ player_number ]=world->CreateBody(&bodydef);
    //create fixture
    b2PolygonShape shape2;
    float factor=0.7;
    b2Vec2 p1(factor*-1.3,factor*0.0);
    b2Vec2 p2(factor*-0.8,factor*-0.8);
    b2Vec2 p3(factor*0.0,factor*-1.1);
    b2Vec2 p4(factor*0.8,factor*-0.8);
    b2Vec2 p5(factor*1.3,factor*0.0);
    b2Vec2 p6(factor*0.4,factor*3.4);
    b2Vec2 p7(factor*0.0,factor*3.6);
    b2Vec2 p8(factor*-0.4,factor*3.4);

    b2Vec2 arr[]={p1,p2,p3,p4,p5,p6,p7,p8};
    shape2.Set(arr,8);
    b2FixtureDef fixturedef;
    fixturedef.shape=&shape2;
    fixturedef.density=1.0;
    m_p_flipper_body[ player_number ]->CreateFixture(&fixturedef);
    //set center of rotation
    b2MassData massD;
    m_p_flipper_body[ player_number ]->GetMassData(&massD);
    b2Vec2 centerV;
    centerV.x = 0.0;
    centerV.y = 0.0;
    massD.center = centerV;
    m_p_flipper_body[ player_number ]->SetMassData(&massD);

    st_body_user_data* data=new st_body_user_data;
    data->i_player_number=player_number;
    data->s_info="Flipper";

    data->vec_vertex_points.push_back( st_point( factor*0.0,factor*4.2 ) );
    data->vec_vertex_points.push_back( st_point( factor*-0.8,factor*3.6 ) );
    data->vec_vertex_points.push_back( st_point( factor*0.8,factor*3.6 ) );
    data->vec_vertex_points.push_back( st_point( factor*-1.7,factor*0.0 ) );
    data->vec_vertex_points.push_back( st_point( factor*1.7,factor*0.0 ) );
    data->vec_vertex_points.push_back( st_point( factor*-1.2,factor*-1.1 ) );
    data->vec_vertex_points.push_back( st_point( factor*1.2,factor*-1.1 ) );
    data->vec_vertex_points.push_back( st_point( factor*0.0,factor*-1.7 ) );
    data->vec_texture_points.push_back( st_point( 37.0/1024.0,1-12.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 27.0/1024.0,1-19.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 47.0/1024.0,1-19.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 16.0/1024.0,1-57.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 58.0/1024.0,1-57.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 23.0/1024.0,1-70.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 51.0/1024.0,1-70.0/1024.0 ) );
    data->vec_texture_points.push_back( st_point( 37.0/1024.0,1-76.0/1024.0 ) );

    switch(player_number)
    {
        case 0:
        {
            data->col_color.R=0.8;
            data->col_color.G=0.2;
            data->col_color.B=0.2;
        }break;

        case 1:
        {
            data->col_color.R=0.2;
            data->col_color.G=0.2;
            data->col_color.B=0.8;
        }break;

        case 2:
        {
            data->col_color.R=0.2;
            data->col_color.G=0.8;
            data->col_color.B=0.2;
        }break;

        case 3:
        {
            data->col_color.R=0.8;
            data->col_color.G=0.8;
            data->col_color.B=0.2;
        }break;
    }

    data->col_color.brightness=1.0;
    data->col_update_speed=1.0;

    m_p_flipper_body[player_number]->SetUserData(data);

    //play sound
    m_p_SoundEngine->playSimpleSound(wav_join,0.5);

    return true;
}

bool game::remove_player(int player_number)
{
    st_body_user_data* data=static_cast<st_body_user_data*>( m_p_flipper_body[ player_number ]->GetUserData() );
    int number=data->i_player_number;
    cout<<"Removing player "<<number<<endl;
    delete m_p_flipper_body[ player_number ]->GetUserData();
    m_p_flipper_body[ player_number ]->GetWorld()->DestroyBody( m_p_flipper_body[ player_number ] );

    return true;
}

bool game::load_textures(void)
{
    //load file and decode
    string s_decode=base64_decode( load_base64_file(file_texture) );

    const unsigned char* texture_data=(const unsigned char*)s_decode.c_str();

    //load texture from memory
    m_texture = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	if(m_texture == 0)
	{
	    return false;
	}

    return true;
}

bool game::load_sounds(void)
{
    m_p_SoundEngine=new sound();

    //load files from text and decode
    m_p_SoundEngine->load_WAVE_from_string(wav_beep1, base64_decode( load_base64_file(file_sound_beep1) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_goal, base64_decode( load_base64_file(file_sound_goal) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_intro, base64_decode( load_base64_file(file_sound_intro) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_gameover, base64_decode( load_base64_file(file_sound_gameover) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_ping, base64_decode( load_base64_file(file_sound_ping) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_dong, base64_decode( load_base64_file(file_sound_dong) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_join, base64_decode( load_base64_file(file_sound_join) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_music_intro, base64_decode( load_base64_file(file_sound_music_intro) ) );
    m_p_SoundEngine->load_WAVE_from_string(wav_music_loop, base64_decode( load_base64_file(file_sound_music_loop) ) );

    if(!m_p_SoundEngine->m_ready || m_p_SoundEngine->get_error()!=0)
    {
        cout<<"ERROR problem loading sound, error id: "<<m_p_SoundEngine->get_error()<<endl;
        return false;
    }

    return true;
}

bool game::handle_event(void)
{

    cout<<"Event: "<<*m_p_event_flag<<endl;

    switch(*m_p_event_flag)
    {
        case ev_error://
        {
            ;
        }break;

        case ev_goal_left://
        {
            if(m_reset_board) break;//no goals during goal delay
            m_score_display_right.set_number( m_score_display_right.get_number()+1 );

            if(m_score_display_right.get_number()>=_goal_limit)//game over test
            {
                game_over("Right");
                break;
            }
            else//play goal sound
            {
                m_p_SoundEngine->playSimpleSound(wav_goal,1.0);
            }

            m_reset_board=true;
            m_reset_board_tick=_goal_delay;
        }break;

        case ev_goal_right://
        {
            if(m_reset_board) break;//no goals during goal delay
            m_score_display_left.set_number( m_score_display_left.get_number()+1 );

            if(m_score_display_left.get_number()>=_goal_limit)//game over test
            {
                game_over("Left");
                break;
            }
            else//play goal sound
            {
                m_p_SoundEngine->playSimpleSound(wav_goal,1.0);
            }

            m_reset_board=true;
            m_reset_board_tick=_goal_delay;
        }break;

        case ev_collision_color://change color update speed
        {
            ;
        }break;
    }

    //reset event flag
    *m_p_event_flag=0;

    return true;
}

bool game::handle_sound_event(void)
{
    if( m_col_sound_cooldown<=0 )
    {
        m_p_SoundEngine->playSimpleSound( *m_p_sound_flag,*m_p_sound_vol );
        //cout<<*m_p_sound_vol<<endl;
        m_col_sound_cooldown=_col_sound_cooldown;
    }

    *m_p_sound_flag=-1;
    *m_p_sound_vol=0;


    return true;
}

bool game::reset_board(void)
{
    //ball to center
    m_ball->SetLinearVelocity(b2Vec2(0,0));
    m_ball->SetAngularVelocity(0);
    m_ball->SetTransform( b2Vec2( 0, 0 ) , 0 );

    //reset flipper pos
    for(int i=0;i<4;i++)
    {
        if( m_p_flipper_body[i]==NULL ) continue;
        m_p_flipper_body[i]->SetLinearVelocity(b2Vec2(0,0));
        m_p_flipper_body[i]->SetAngularVelocity(0);
        switch(i)
        {
            case 0:
            {
                m_p_flipper_body[i]->SetTransform( b2Vec2( 26, 15 ) , 0 );
            }break;

            case 1:
            {
                m_p_flipper_body[i]->SetTransform( b2Vec2( 70, 15 ) , 0 );
            }break;

            case 2:
            {
                m_p_flipper_body[i]->SetTransform( b2Vec2( 26, 38 ) , 0 );
            }break;

            case 3:
            {
                m_p_flipper_body[i]->SetTransform( b2Vec2( 70, 38 ) , 0 );
            }break;
        }

    }

    //reset goals
    m_goal_left->SetLinearVelocity(b2Vec2(0,0));
    m_goal_left->SetAngularVelocity(0);
    m_goal_left->SetTransform( b2Vec2( 288.0*_Pix2Met, 540.0*_Pix2Met ) , 3.14159 );

    m_goal_right->SetLinearVelocity(b2Vec2(0,0));
    m_goal_right->SetAngularVelocity(0);
    m_goal_right->SetTransform( b2Vec2( 1632.0*_Pix2Met, 540.0*_Pix2Met ) , 0 );

    //play sound
    m_p_SoundEngine->playSimpleSound(wav_dong,0.5);

    return true;
}

bool game::game_over(string winner)
{
    cout<<"GAME OVER\n";
    cout<<winner<<" side won\n";

    m_s_winner=winner;

    m_reset_board=true;//stopps further goals

    m_game_over_tick=_game_over_delay;

    m_state=state_game_over;

    //play sound
    if(!m_music_enabled) m_p_SoundEngine->playSimpleSound(wav_gameover,0.5);

    return true;
}

//-----------------

void func(b2Contact* contact)
{
    cout<<"func called\n";
}

/*void game::fixture_test(b2Fixture* fixA,b2Fixture* fixB)
{
    if( fixA==m_goal_sensor || fixB==m_goal_sensor )
    {
        cout<<"XXX\n";
    }
}

b2Contact* game::func(b2Contact* contact)//test
{
    cout<<"func called\n";

    b2Fixture* fixA=contact->GetFixtureA();
    b2Fixture* fixB=contact->GetFixtureB();

    //fixture_test(fixA,fixB);

    return contact;
}*/
