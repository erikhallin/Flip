#ifndef DISPLAY_7SEG_H
#define DISPLAY_7SEG_H

#include <gl/gl.h>
#include <time.h>

class display_7seg
{
    public:

        display_7seg();

        bool m_ready;

        bool init(float xpos,float ypos,float width,float height);
        bool set_texture(int texture,float tex_x_pos_min,float tex_x_pos_max,float tex_y_pos_min,float tex_y_pos_max);
        bool set_number(int number);
        bool reset(void);
        int  get_number(void);
        bool draw(void);

    private:

        float m_xpos,m_ypos,m_width,m_height;//in pixels
        float m_xpos_temp,m_ypos_temp,m_width_temp,m_height_temp;//in pixels
        float m_time_this_cycle,m_time_last_cycle;
        float m_tex_x_pos_min,m_tex_x_pos_max,m_tex_y_pos_min,m_tex_y_pos_max;
        bool  m_draw_border,m_have_texture;
        bool  m_pulse_active,m_pulse_expansion;
        float m_pulse_progress;
        int   m_texture;
        float m_color[3];
        bool  m_lines_to_draw[7];//the segments flags
        float m_lines_to_draw_pos[7][4][2]; // [line number] [point number (2) or 4 for texture] [x,y]
        int   m_value_to_display;

        bool pre_display_pos_calc(void);
        bool pre_display_value_calc(void);
};

#endif // DISPLAY_7SEG_H
