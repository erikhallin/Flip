#ifndef DECAL_H
#define DECAL_H

#include <gl\gl.h>

class decal
{
    public:
        //Constructors
        decal();
          //mask should be located directly beneath texture in same file
          //mask: black will be drawn, not white
          //pic:  black will not be drawn
        decal(float x_pos,float y_pos,float width,float height,int texture,
              float tex_offset_x_min,float tex_offset_x_max,float tex_offset_y_min,float tex_offset_y_max);//send location of texture in pixel count
        //Variables
        bool m_ready;
        //Functions
        bool set_color_mask(float color[3]);
        bool get_color_mask(float color[3]);
        bool set_location(float x_pos,float y_pos,float width,float height);
        bool zoom(float new_size);// 0.0 will not change size
        bool reset_zoom(void);
        bool shift_texture_offset(float x_shift,float y_shift);
        bool draw(void);
        bool masking(bool flag);
        bool show_decal(bool flag);
    private:
        //Variables
        bool  m_show,m_have_mask;
        float m_x_pos,m_y_pos,m_width,m_height;
        float m_x_pos_old,m_y_pos_old,m_width_old,m_height_old;//original values
        int   m_texture;
        float m_color_mask[3];
        float m_x_offset_min,m_y_offset_min,m_x_offset_max,m_y_offset_max;//texture offset values
        //Functions

};

#endif // DECAL_H
