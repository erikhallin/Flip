#include "decal.h"

decal::decal()
{
    m_ready=false;
}

decal::decal(float x_pos,float y_pos,float width,float height,int texture,float tex_offset_x_min,float tex_offset_x_max,float tex_offset_y_min,float tex_offset_y_max)
{
    m_x_pos=m_x_pos_old=x_pos;
    m_y_pos=m_y_pos_old=y_pos;
    m_width=m_width_old=width;
    m_height=m_height_old=height;
    m_texture=texture;
    float texture_size=1024;
    m_x_offset_min=tex_offset_x_min/texture_size;
    m_y_offset_min=tex_offset_y_min/texture_size;
    m_x_offset_max=tex_offset_x_max/texture_size;
    m_y_offset_max=tex_offset_y_max/texture_size;

    m_color_mask[0]=1.0;
    m_color_mask[1]=1.0;
    m_color_mask[2]=1.0;

    m_show=true;
    m_have_mask=false;
    m_ready=true;
}

bool decal::set_color_mask(float color[3])
{
    m_color_mask[0]=color[0];
    m_color_mask[1]=color[1];
    m_color_mask[2]=color[2];
    return true;
}

bool decal::get_color_mask(float color[3])
{
    color[0]=m_color_mask[0];
    color[1]=m_color_mask[1];
    color[2]=m_color_mask[2];
    return true;
}

bool decal::set_location(float x_pos,float y_pos,float width,float height)
{
    m_x_pos=m_x_pos_old=x_pos;
    m_y_pos=m_y_pos_old=y_pos;
    m_width=m_width_old=width;
    m_height=m_height_old=height;

    return true;
}

bool decal::zoom(float new_size)
{
    if(new_size==0) return false;

    float new_width=m_width*(new_size);
    float new_height=m_height*(new_size);

    m_x_pos=m_x_pos-(new_width-m_width)/2;
    m_y_pos=m_y_pos-(new_height-m_height)/2;

    m_width=new_width;
    m_height=new_height;

    return true;
}

bool decal::reset_zoom(void)
{
    m_x_pos=m_x_pos_old;
    m_y_pos=m_y_pos_old;
    m_width=m_width_old;
    m_height=m_height_old;

    return true;
}

bool decal::shift_texture_offset(float x_shift,float y_shift)
{
    float texture_size=1024;
    float d_y=m_y_offset_max-m_y_offset_min;
    float d_x=m_x_offset_max-m_x_offset_min;
    m_x_offset_min+=x_shift*(d_x+0.5/texture_size);
    m_x_offset_max+=x_shift*(d_x+0.5/texture_size);
    m_y_offset_min+=y_shift*(d_y+0.5/texture_size);
    m_y_offset_max+=y_shift*(d_y+0.5/texture_size);

    return true;
}

bool decal::draw(void)
{
    if(!m_ready||!m_show) return false;

    //Init
    glColor3f(m_color_mask[0],m_color_mask[1],m_color_mask[2]);//white allows all colors
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    float x_offset_min=m_x_offset_min;
    float y_offset_min=1-m_y_offset_min;
    float x_offset_max=m_x_offset_max;
    float y_offset_max=1-m_y_offset_max;
    float texture_size=1024;
    float d_y=m_y_offset_max-m_y_offset_min;
    float d_x=m_x_offset_max-m_x_offset_min;

    if(m_have_mask)
    {
        //Draw mask
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR,GL_ZERO);

        glBegin(GL_QUADS);
        glTexCoord2f(x_offset_min,y_offset_min);
        glVertex2f(m_x_pos,m_y_pos);
        glTexCoord2f(x_offset_min,y_offset_max);
        glVertex2f(m_x_pos,m_y_pos+m_height);
        glTexCoord2f(x_offset_max,y_offset_max);
        glVertex2f(m_x_pos+m_width,m_y_pos+m_height);
        glTexCoord2f(x_offset_max,y_offset_min);
        glVertex2f(m_x_pos+m_width,m_y_pos);
        glEnd();

        //Draw picture
        y_offset_min-=(d_y+1/texture_size);
        y_offset_max-=(d_y+1/texture_size);

        glDepthMask(GL_TRUE);
        glBlendFunc(GL_ONE,GL_ONE);

        glBegin(GL_QUADS);
        glTexCoord2f(x_offset_min,y_offset_min);
        glVertex2f(m_x_pos,m_y_pos);
        glTexCoord2f(x_offset_min,y_offset_max);
        glVertex2f(m_x_pos,m_y_pos+m_height);
        glTexCoord2f(x_offset_max,y_offset_max);
        glVertex2f(m_x_pos+m_width,m_y_pos+m_height);
        glTexCoord2f(x_offset_max,y_offset_min);
        glVertex2f(m_x_pos+m_width,m_y_pos);
        glEnd();

        glDisable(GL_BLEND);
    }
    else
    {
        glBegin(GL_QUADS);
        glTexCoord2f(x_offset_min,y_offset_min);
        glVertex2f(m_x_pos,m_y_pos);
        glTexCoord2f(x_offset_min,y_offset_max);
        glVertex2f(m_x_pos,m_y_pos+m_height);
        glTexCoord2f(x_offset_max,y_offset_max);
        glVertex2f(m_x_pos+m_width,m_y_pos+m_height);
        glTexCoord2f(x_offset_max,y_offset_min);
        glVertex2f(m_x_pos+m_width,m_y_pos);
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    return true;
}

bool decal::masking(bool flag)
{
    m_have_mask=flag;
    return true;
}

bool decal::show_decal(bool flag)
{
    m_show=flag;
    return true;
}
