#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <windows.h>
#define __in
#define __out
#define __reserved
#include <D3D/XInput.h>

struct st_gamepad_data
{
    //Buttons
    bool button_A,button_B,button_X,button_Y;
    bool button_back,button_start,button_info;
    bool button_LB,button_RB;
    bool button_right_thumbstick,button_left_thumbstick;
    bool button_right_trigger,button_left_trigger;

    //Dpad
    bool dpad_up,dpad_right,dpad_down,dpad_left;

    //Triggers
    int trigger_right,trigger_left;

    //Thumbsticks
    int thumbstick_left_x,thumbstick_left_y,thumbstick_right_x,thumbstick_right_y;
};

class gamepad
{
    public:

        gamepad(int playerNumber = 0);//0-3

        bool             IsConnected(void);//test if connected
        st_gamepad_data  GetState(void);//get controller data
        void             Vibrate(int leftVal = 0, int rightVal = 0);

    private:

        int           m_controllerNum;
        XINPUT_STATE  m_controllerState;
};

#endif // GAMEPAD_H
