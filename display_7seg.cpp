#include "display_7seg.h"

display_7seg::display_7seg()
{
    m_ready=false;
}

bool display_7seg::init(float xpos,float ypos,float width,float height)
{
    m_time_last_cycle=m_time_this_cycle=0;

    m_xpos=m_xpos_temp=xpos; m_ypos=m_ypos_temp=ypos;
    m_width=m_width_temp=width; m_height=m_height_temp=height;
    m_color[0]=0.9; m_color[1]=0.0; m_color[2]=0.0;
    m_draw_border=false;
    m_have_texture=false;
    m_pulse_active=false;
    m_value_to_display=0;
    m_ready=pre_display_value_calc();

    pre_display_pos_calc();

    return true;
}

bool display_7seg::set_texture(int texture,float tex_x_pos_min,float tex_x_pos_max,float tex_y_pos_min,float tex_y_pos_max)
{
    m_texture=texture;
    m_have_texture=true;

    m_tex_x_pos_min=tex_x_pos_min/1024.0;
    m_tex_x_pos_max=tex_x_pos_max/1024.0;
    m_tex_y_pos_min=1.0-tex_y_pos_min/1024.0;
    m_tex_y_pos_max=1.0-tex_y_pos_max/1024.0;

    pre_display_pos_calc();

    return true;
}

bool display_7seg::set_number(int number)
{
    if( number<0||number>9 ) return false;
    m_value_to_display=number;
    pre_display_value_calc();

    //start pulse anim
    m_pulse_active=true;
    m_pulse_expansion=true;
    m_pulse_progress=1.0; //1 - 2

    return true;
}

bool display_7seg::reset(void)
{
    m_value_to_display=0;
    m_xpos_temp=m_xpos;
    m_ypos_temp=m_ypos;
    m_width_temp=m_width;
    m_height_temp=m_height;

    m_pulse_active=false;
    m_pulse_expansion=false;
    m_pulse_progress=1;

    pre_display_value_calc();
    pre_display_pos_calc();

    return true;
}

int display_7seg::get_number(void)
{
    return m_value_to_display;
}

bool display_7seg::draw(void)
{
    if(!m_ready) return false;

    //time
    m_time_last_cycle=m_time_this_cycle;
    m_time_this_cycle=(float)clock()/CLOCKS_PER_SEC;//get time now, in sec

    if(m_pulse_active)//expand size
    {
        bool skip_calc=false;
        if(m_pulse_expansion)
        {
            float speed=25.0;
            m_pulse_progress+=(m_time_this_cycle-m_time_last_cycle)*speed;
            if(m_pulse_progress>2) //end expansion
            {
                m_pulse_expansion=false;
            }
        }
        else//decrease size
        {
            float speed=4.00;
            m_pulse_progress-=(m_time_this_cycle-m_time_last_cycle)*speed;
            if(m_pulse_progress<1)
            {//done
                m_pulse_progress=1.0;
                m_pulse_active=false;
                m_width_temp=m_width;
                m_height_temp=m_height;
                m_xpos_temp=m_xpos;
                m_ypos_temp=m_ypos;
                //color update
                m_color[0]=0.9;
                m_color[1]=0.0;
                m_color[2]=0.0;

                pre_display_pos_calc();

                skip_calc=true;
            }
        }
        if(!skip_calc)
        {
            //update sizes
            m_width_temp=m_width*m_pulse_progress;
            m_height_temp=m_height*m_pulse_progress;
            m_xpos_temp=m_xpos-(m_width/2)*(m_pulse_progress-1);
            m_ypos_temp=m_ypos-(m_height/2)*(m_pulse_progress-1);

            pre_display_pos_calc();

            //color update
            m_color[0]=0.9-(m_pulse_progress-1.0)/4.0;
            m_color[1]=0.0+(m_pulse_progress-1.0)/2.0;
            m_color[2]=0.0+(m_pulse_progress-1.0)/10.0;
        }
    }

    glPushMatrix();

    glTranslatef(m_xpos_temp,m_ypos_temp,0);

    //draw border
    if(m_draw_border)
    {
        glColor3f( 1,1,1 );
        glBegin(GL_LINE_STRIP);
        glVertex2f(0,0);
        glVertex2f(0,m_height_temp);
        glVertex2f(m_width_temp,m_height_temp);
        glVertex2f(m_width_temp,0);
        glVertex2f(0,0);
        glEnd();
    }

    if(m_have_texture)
    {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glBegin(GL_QUADS);
        for(int i=0;i<7;i++)
        {
            if(m_lines_to_draw[i])
            {
                glColor3f( m_color[0],m_color[1],m_color[2] );
                glTexCoord2f( m_tex_x_pos_min,m_tex_y_pos_max );
                glVertex2f( m_lines_to_draw_pos[i][0][0],m_lines_to_draw_pos[i][0][1] );
                glTexCoord2f( m_tex_x_pos_min,m_tex_y_pos_min );
                glVertex2f( m_lines_to_draw_pos[i][1][0],m_lines_to_draw_pos[i][1][1] );
                glTexCoord2f( m_tex_x_pos_max,m_tex_y_pos_min );
                glVertex2f( m_lines_to_draw_pos[i][2][0],m_lines_to_draw_pos[i][2][1] );
                glTexCoord2f( m_tex_x_pos_max,m_tex_y_pos_max );
                glVertex2f( m_lines_to_draw_pos[i][3][0],m_lines_to_draw_pos[i][3][1] );

            }
            else//weak color
            {
                glColor3f( m_color[0]/6,m_color[1]/6,m_color[2]/6 );
                glTexCoord2f( m_tex_x_pos_min,m_tex_y_pos_max );
                glVertex2f( m_lines_to_draw_pos[i][0][0],m_lines_to_draw_pos[i][0][1] );
                glTexCoord2f( m_tex_x_pos_min,m_tex_y_pos_min );
                glVertex2f( m_lines_to_draw_pos[i][1][0],m_lines_to_draw_pos[i][1][1] );
                glTexCoord2f( m_tex_x_pos_max,m_tex_y_pos_min );
                glVertex2f( m_lines_to_draw_pos[i][2][0],m_lines_to_draw_pos[i][2][1] );
                glTexCoord2f( m_tex_x_pos_max,m_tex_y_pos_max );
                glVertex2f( m_lines_to_draw_pos[i][3][0],m_lines_to_draw_pos[i][3][1] );
            }
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
    else
    {
        glColor3f( m_color[0],m_color[1],m_color[2] );
        glBegin(GL_LINES);
        for(int i=0;i<7;i++)
        {
            if(m_lines_to_draw[i])
            {
                glColor3f( m_color[0],m_color[1],m_color[2] );
                glVertex2f( m_lines_to_draw_pos[i][0][0],m_lines_to_draw_pos[i][0][1] );
                glVertex2f( m_lines_to_draw_pos[i][1][0],m_lines_to_draw_pos[i][1][1] );
            }
            else//weak color
            {
                glColor3f( m_color[0]/6,m_color[1]/6,m_color[2]/6 );
                glVertex2f( m_lines_to_draw_pos[i][0][0],m_lines_to_draw_pos[i][0][1] );
                glVertex2f( m_lines_to_draw_pos[i][1][0],m_lines_to_draw_pos[i][1][1] );
            }
        }
        glEnd();
    }

    glPopMatrix();

    return true;
}

//---------------------------
bool display_7seg::pre_display_pos_calc(void)
{
    if(m_have_texture)
    {
        //recalc quad positions
        float width=m_width_temp;
        float height=m_height_temp;
        //a
        m_lines_to_draw_pos[0][0][0]=width*(35.0/159.0);
        m_lines_to_draw_pos[0][0][1]=height*(6.0/215.0);
        m_lines_to_draw_pos[0][1][0]=width*(35.0/159.0);
        m_lines_to_draw_pos[0][1][1]=height*(27.0/215.0);
        m_lines_to_draw_pos[0][2][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[0][2][1]=height*(27.0/215.0);
        m_lines_to_draw_pos[0][3][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[0][3][1]=height*(6.0/215.0);
        //b
        m_lines_to_draw_pos[1][0][0]=width*(146.0/159.0);
        m_lines_to_draw_pos[1][0][1]=height*(22.0/215.0);
        m_lines_to_draw_pos[1][1][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[1][1][1]=height*(22.0/215.0);
        m_lines_to_draw_pos[1][2][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[1][2][1]=height*(108.0/215.0);
        m_lines_to_draw_pos[1][3][0]=width*(146.0/159.0);
        m_lines_to_draw_pos[1][3][1]=height*(108.0/215.0);
        //c
        m_lines_to_draw_pos[2][0][0]=width*(146.0/159.0);
        m_lines_to_draw_pos[2][0][1]=height*(114.0/215.0);
        m_lines_to_draw_pos[2][1][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[2][1][1]=height*(114.0/215.0);
        m_lines_to_draw_pos[2][2][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[2][2][1]=height*(200.0/215.0);
        m_lines_to_draw_pos[2][3][0]=width*(146.0/159.0);
        m_lines_to_draw_pos[2][3][1]=height*(200.0/215.0);
        //d
        m_lines_to_draw_pos[3][0][0]=width*(35.0/159.0);
        m_lines_to_draw_pos[3][0][1]=height*(189.0/215.0);
        m_lines_to_draw_pos[3][1][0]=width*(35.0/159.0);
        m_lines_to_draw_pos[3][1][1]=height*(210.0/215.0);
        m_lines_to_draw_pos[3][2][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[3][2][1]=height*(210.0/215.0);
        m_lines_to_draw_pos[3][3][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[3][3][1]=height*(189.0/215.0);
        //e
        m_lines_to_draw_pos[4][0][0]=width*(31.0/159.0);
        m_lines_to_draw_pos[4][0][1]=height*(114.0/215.0);
        m_lines_to_draw_pos[4][1][0]=width*(10.0/159.0);
        m_lines_to_draw_pos[4][1][1]=height*(114.0/215.0);
        m_lines_to_draw_pos[4][2][0]=width*(10.0/159.0);
        m_lines_to_draw_pos[4][2][1]=height*(200.0/215.0);
        m_lines_to_draw_pos[4][3][0]=width*(31.0/159.0);
        m_lines_to_draw_pos[4][3][1]=height*(200.0/215.0);
        //f
        m_lines_to_draw_pos[5][0][0]=width*(31.0/159.0);
        m_lines_to_draw_pos[5][0][1]=height*(22.0/215.0);
        m_lines_to_draw_pos[5][1][0]=width*(10.0/159.0);
        m_lines_to_draw_pos[5][1][1]=height*(22.0/215.0);
        m_lines_to_draw_pos[5][2][0]=width*(10.0/159.0);
        m_lines_to_draw_pos[5][2][1]=height*(108.0/215.0);
        m_lines_to_draw_pos[5][3][0]=width*(31.0/159.0);
        m_lines_to_draw_pos[5][3][1]=height*(108.0/215.0);
        //g
        m_lines_to_draw_pos[6][0][0]=width*(35.0/159.0);
        m_lines_to_draw_pos[6][0][1]=height*(104.0/215.0);
        m_lines_to_draw_pos[6][1][0]=width*(35.0/159.0);
        m_lines_to_draw_pos[6][1][1]=height*(125.0/215.0);
        m_lines_to_draw_pos[6][2][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[6][2][1]=height*(125.0/215.0);
        m_lines_to_draw_pos[6][3][0]=width*(121.0/159.0);
        m_lines_to_draw_pos[6][3][1]=height*(104.0/215.0);
    }
    else
    {
        //recalc line positions
        float width=m_width_temp;
        float height=m_height_temp;
        //a
        m_lines_to_draw_pos[0][0][0]=width*0.2;
        m_lines_to_draw_pos[0][0][1]=height*0.06;
        m_lines_to_draw_pos[0][1][0]=width*0.743;
        m_lines_to_draw_pos[0][1][1]=height*0.06;
        //b
        m_lines_to_draw_pos[1][0][0]=width*0.857;
        m_lines_to_draw_pos[1][0][1]=height*0.12;
        m_lines_to_draw_pos[1][1][0]=width*0.857;
        m_lines_to_draw_pos[1][1][1]=height*0.42;
        //c
        m_lines_to_draw_pos[2][0][0]=width*0.857;
        m_lines_to_draw_pos[2][0][1]=height*0.56;
        m_lines_to_draw_pos[2][1][0]=width*0.857;
        m_lines_to_draw_pos[2][1][1]=height*0.86;
        //d
        m_lines_to_draw_pos[3][0][0]=width*0.2;
        m_lines_to_draw_pos[3][0][1]=height*0.92;
        m_lines_to_draw_pos[3][1][0]=width*0.743;
        m_lines_to_draw_pos[3][1][1]=height*0.92;
        //e
        m_lines_to_draw_pos[4][0][0]=width*0.114;
        m_lines_to_draw_pos[4][0][1]=height*0.56;
        m_lines_to_draw_pos[4][1][0]=width*0.114;
        m_lines_to_draw_pos[4][1][1]=height*0.86;
        //f
        m_lines_to_draw_pos[5][0][0]=width*0.114;
        m_lines_to_draw_pos[5][0][1]=height*0.12;
        m_lines_to_draw_pos[5][1][0]=width*0.114;
        m_lines_to_draw_pos[5][1][1]=height*0.42;
        //g
        m_lines_to_draw_pos[6][0][0]=width*0.2;
        m_lines_to_draw_pos[6][0][1]=height*0.48;
        m_lines_to_draw_pos[6][1][0]=width*0.743;
        m_lines_to_draw_pos[6][1][1]=height*0.48;
    }


    return true;
}


bool display_7seg::pre_display_value_calc(void)
{
    for(int i=0;i<7;i++) m_lines_to_draw[i]=false;

    //turn on correct lines
    //    0
    //  5   1
    //    6
    //  4   2
    //    3
    switch(m_value_to_display)
    {
        case 0:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=true;
            m_lines_to_draw[5]=true;
            m_lines_to_draw[6]=false;
        }break;

        case 1:
        {
            m_lines_to_draw[0]=false;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=false;
            m_lines_to_draw[4]=false;
            m_lines_to_draw[5]=false;
            m_lines_to_draw[6]=false;
        }break;

        case 2:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=false;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=true;
            m_lines_to_draw[5]=false;
            m_lines_to_draw[6]=true;
        }break;

        case 3:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=false;
            m_lines_to_draw[5]=false;
            m_lines_to_draw[6]=true;
        }break;

        case 4:
        {
            m_lines_to_draw[0]=false;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=false;
            m_lines_to_draw[4]=false;
            m_lines_to_draw[5]=true;
            m_lines_to_draw[6]=true;
        }break;

        case 5:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=false;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=false;
            m_lines_to_draw[5]=true;
            m_lines_to_draw[6]=true;
        }break;

        case 6:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=false;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=true;
            m_lines_to_draw[5]=true;
            m_lines_to_draw[6]=true;
        }break;

        case 7:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=false;
            m_lines_to_draw[4]=false;
            m_lines_to_draw[5]=false;
            m_lines_to_draw[6]=false;
        }break;

        case 8:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=true;
            m_lines_to_draw[5]=true;
            m_lines_to_draw[6]=true;
        }break;

        case 9:
        {
            m_lines_to_draw[0]=true;
            m_lines_to_draw[1]=true;
            m_lines_to_draw[2]=true;
            m_lines_to_draw[3]=true;
            m_lines_to_draw[4]=false;
            m_lines_to_draw[5]=true;
            m_lines_to_draw[6]=true;
        }break;
    }

    return true;
}
