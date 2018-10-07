#ifndef MYCONTACTLISTENER_H
#define MYCONTACTLISTENER_H

#include <iostream>
#include <Box2D/Box2D.h>

enum events
{
    ev_error=0,
    ev_goal_left,
    ev_goal_right,
    ev_collision_color
};

class MyContactListener : public b2ContactListener
{
    public:

        bool init(b2Fixture* goal_sensor_left, b2Fixture* goal_sensor_right,
                  b2Body* ball, void (*func_to_call)(b2Contact*),
                  int* event_flag, int* sound_flag, float* sound_vol, b2Body* body_pointer[2] );

        void (*func)(b2Contact*);
        bool func_to_call_set;

    private:

        b2Fixture* m_goal_sensor_left;
        b2Fixture* m_goal_sensor_right;
        b2Body*    m_ball;
        b2Body*    m_body_pointers[2];
        int*       m_p_event_flag;
        int*       m_p_sound_flag;
        float*     m_p_sound_vol;

        void BeginContact(b2Contact* contact);
        void EndContact(b2Contact* contact);
        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};

#endif // MYCONTACTLISTENER_H
