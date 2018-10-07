#include "gamepad.h"

gamepad::gamepad(int playerNumber)
{
    // Set the Controller Number
	m_controllerNum = playerNumber;
}

bool gamepad::IsConnected(void)
{
    // Zeroise the state
	ZeroMemory(&m_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(m_controllerNum, &m_controllerState);

	if(Result == ERROR_SUCCESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}

st_gamepad_data gamepad::GetState()
{
	// Zeroise the state
	ZeroMemory(&m_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	XInputGetState(m_controllerNum, &m_controllerState);

	// Return the state
	st_gamepad_data gamepad_data;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A)               gamepad_data.button_A=true; else gamepad_data.button_A=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B)               gamepad_data.button_B=true; else gamepad_data.button_B=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X)               gamepad_data.button_X=true; else gamepad_data.button_X=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)               gamepad_data.button_Y=true; else gamepad_data.button_Y=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)            gamepad_data.button_back=true; else gamepad_data.button_back=false;
	gamepad_data.button_info=false;//temp
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START)           gamepad_data.button_start=true; else gamepad_data.button_start=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)   gamepad_data.button_LB=true; else gamepad_data.button_LB=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)  gamepad_data.button_RB=true; else gamepad_data.button_RB=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)      gamepad_data.button_left_thumbstick=true; else gamepad_data.button_left_thumbstick=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)     gamepad_data.button_right_thumbstick=true; else gamepad_data.button_right_thumbstick=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)       gamepad_data.dpad_down=true; else gamepad_data.dpad_down=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)       gamepad_data.dpad_left=true; else gamepad_data.dpad_left=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)      gamepad_data.dpad_right=true; else gamepad_data.dpad_right=false;
	if(m_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)         gamepad_data.dpad_up=true; else gamepad_data.dpad_up=false;

	//Trigger state
	gamepad_data.trigger_left=m_controllerState.Gamepad.bLeftTrigger;
	gamepad_data.trigger_right=m_controllerState.Gamepad.bRightTrigger;
	if(m_controllerState.Gamepad.bRightTrigger && m_controllerState.Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		gamepad_data.button_right_trigger=true;
	}
	else gamepad_data.button_right_trigger=false;

	if(m_controllerState.Gamepad.bLeftTrigger && m_controllerState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		gamepad_data.button_left_trigger=true;
	}
	else gamepad_data.button_left_trigger=false;

    //Check deadzone for thumbsticks
    if( (m_controllerState.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         m_controllerState.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
        (m_controllerState.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         m_controllerState.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
    {
        m_controllerState.Gamepad.sThumbLX = 0;
        m_controllerState.Gamepad.sThumbLY = 0;
    }
    if( (m_controllerState.Gamepad.sThumbRX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         m_controllerState.Gamepad.sThumbRX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
        (m_controllerState.Gamepad.sThumbRY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         m_controllerState.Gamepad.sThumbRY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) )
    {
        m_controllerState.Gamepad.sThumbRX = 0;
        m_controllerState.Gamepad.sThumbRY = 0;
    }
	gamepad_data.thumbstick_left_x=m_controllerState.Gamepad.sThumbLX;
	gamepad_data.thumbstick_left_y=m_controllerState.Gamepad.sThumbLY;
	gamepad_data.thumbstick_right_x=m_controllerState.Gamepad.sThumbRX;
	gamepad_data.thumbstick_right_y=m_controllerState.Gamepad.sThumbRY;

	return gamepad_data;
}

void gamepad::Vibrate(int leftVal, int rightVal)
{
	// Create a Vibraton State
	XINPUT_VIBRATION Vibration;

	// Zeroise the Vibration
	ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

	// Set the Vibration Values
	Vibration.wLeftMotorSpeed = leftVal;
	Vibration.wRightMotorSpeed = rightVal;

	// Vibrate the controller
	XInputSetState(m_controllerNum, &Vibration);
}
