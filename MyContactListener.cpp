#include "MyContactListener.h"

bool MyContactListener::init(b2Fixture* goal_sensor_left,b2Fixture* goal_sensor_right,
                             b2Body* ball,
                             void (*func_to_call)(b2Contact*),
                             int* event_flag,
                             int* sound_flag,
                             float* sound_vol,
                             b2Body* body_pointers[2] )
{
    func_to_call_set=true;

    m_goal_sensor_left=goal_sensor_left;
    m_goal_sensor_right=goal_sensor_right;
    m_ball=ball;

    func=func_to_call;
    m_p_event_flag=event_flag;
    m_p_sound_flag=sound_flag;
    m_p_sound_vol=sound_vol;

    m_body_pointers[0]=body_pointers[0];
    m_body_pointers[1]=body_pointers[1];

    return true;
}

void MyContactListener::BeginContact(b2Contact* contact)
{
    //if(func_to_call_set) func(contact);

    //get colliders fixtures
    b2Fixture* fixA=contact->GetFixtureA();
    b2Fixture* fixB=contact->GetFixtureB();

    //get ball fixture
    b2Fixture* ball_fix=m_ball->GetFixtureList();

    bool goal_involved_left=false;
    bool goal_involved_right=false;
    if(fixA==m_goal_sensor_left || fixB==m_goal_sensor_left) goal_involved_left=true;
    if(fixA==m_goal_sensor_right || fixB==m_goal_sensor_right) goal_involved_right=true;

    if(goal_involved_left)//test ball
    {
        if(fixA==ball_fix || fixB==ball_fix)
        {
            std::cout<<"Goal, left\n";
            *m_p_event_flag=ev_goal_left;
            return;
        }
    }
    if(goal_involved_right)//test ball
    {
        if(fixA==ball_fix || fixB==ball_fix)
        {
            std::cout<<"Goal, right\n";
            *m_p_event_flag=ev_goal_right;
            return;
        }
    }

    //collision -> play sound
    //*m_p_event_flag=ev_collision;
}

void MyContactListener::EndContact(b2Contact* contact)
{
    ;
}

void MyContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    b2WorldManifold worldManifold;

    contact->GetWorldManifold(&worldManifold);

    b2PointState state1[2], state2[2];

    b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
    //NSLog(@"Presolving");

    if (state2[0] == b2_addState)
    {
        const b2Body* bodyA = contact->GetFixtureA()->GetBody();

        const b2Body* bodyB = contact->GetFixtureB()->GetBody();

        b2Vec2 point = worldManifold.points[0];

        b2Vec2 vA = bodyA->GetLinearVelocityFromWorldPoint(point);

        b2Vec2 vB = bodyB->GetLinearVelocityFromWorldPoint(point);

        b2Vec2 rV = vB - vA;

        float32 approachVelocity = b2Dot(rV, worldManifold.normal);

        if (-5.0f < approachVelocity && approachVelocity < 1.0f)
        {

            //MyPlayCollisionSound();
            //NSLog(@"Not Playing Sound");
            //*m_p_sound_flag=0;


        }
        else
        {
            //NSLog(@"playing the sound");
            *m_p_event_flag=ev_collision_color;
            *m_p_sound_flag=0;
            *m_p_sound_vol=fabs( approachVelocity*approachVelocity*0.002) ;
        }

    }
}

void MyContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    ;
}
