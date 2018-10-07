#include "main_menu.h"

main_menu::main_menu()
{
    m_ready=false;
}

bool main_menu::init(int window_width,int window_height,int texture)
{
    m_window_width=window_width;
    m_window_height=window_height;
    m_texture=texture;

    m_f_size_ratio=(float)m_window_width/1920*1.3;

    m_dec_title=decal(  );
    m_dec_player_icon[0]=decal(  );
    m_dec_player_icon[1]=decal(  );
    m_dec_player_icon[2]=decal(  );
    m_dec_player_icon[3]=decal(  );

    //reset gamepads
    for(int i=0;i<4;i++)
    {
        m_gamepad[i]=gamepad(i);
        m_connected_player[i]=false;
    }

    m_ready=true;

    return true;
}

bool main_menu::draw(void)
{
    m_dec_title.draw();
    for(int i=0;i<4;i++) m_dec_player_icon[i].draw();

    /*glColor3f(1,1,1);

    b2Body* tmp=m_World->GetBodyList();

    while(tmp)
    {
        glPushMatrix();
        b2Vec2 center=tmp->GetWorldCenter();
        b2Vec2 mass_center=tmp->GetLocalCenter();

        glTranslatef(center.x*_Met2Pix,center.y*_Met2Pix,0);//go to body's center
        glRotatef(tmp->GetAngle()*180/3.14159,0,0,1);
        //dont shift circles
        if( ((b2Shape*)tmp->GetFixtureList()->GetShape())->m_type != 0 )
        {
            glTranslatef(-mass_center.x*_Met2Pix,-mass_center.y*_Met2Pix,0);//shift to rotation center
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
                    glVertex2f( cosf( i*3.14159/180 )*radius*_Met2Pix, sinf( i*3.14159/180 )*radius*_Met2Pix );
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
                    glVertex2f(points[i].x*_Met2Pix,points[i].y*_Met2Pix);

                glVertex2f(points[0].x*_Met2Pix,points[0].y*_Met2Pix);

                glEnd();
            }

            //fixture=fixture->GetNext();
        }

        glPopMatrix();

        tmp=tmp->GetNext();
    }
*/
    return true;
}

bool main_menu::update(void)
{
    //test for new players
    //gamepad input
    st_gamepad_data gamepad_data[4];
    for(int i=0;i<4;i++)
    {
        if( m_gamepad[i].IsConnected() )
        {
            gamepad_data[i]=m_gamepad[i].GetState();
            if( !m_connected_player[i] )//add new player
            {
                add_new_player(i);
            }
            m_connected_player[i]=true;
        }
        else
        {
            if( m_connected_player[i] )//lost player
            {
                remove_player(i);
            }
            m_connected_player[i]=false;
        }
    }

    for(int i=0;i<4;i++)
    {
        if( m_connected_player[i] )
        {
            //move flipper
            m_p_flipper_body[ i ]->ApplyForce( b2Vec2( gamepad_data[i].thumbstick_left_x/400*m_f_size_ratio, -gamepad_data[i].thumbstick_left_y/400*m_f_size_ratio ), m_p_flipper_body[ i ]->GetWorldCenter() );
            //rotate flipper
            float target_angle=atan2f( -gamepad_data[i].thumbstick_right_x , -gamepad_data[i].thumbstick_right_y );
            float force = ( fabs(gamepad_data[i].thumbstick_right_x) + fabs(gamepad_data[i].thumbstick_right_y) ) / 65535 * m_f_size_ratio;
            float bodyAngle=m_p_flipper_body[ i ]->GetAngle();
            float totalRotation = target_angle - bodyAngle;
            while ( totalRotation < -180 * 0.0174532 ) totalRotation += 360 * 0.0174532;
            while ( totalRotation >  180 * 0.0174532 ) totalRotation -= 360 * 0.0174532;
            m_p_flipper_body[ i ]->ApplyTorque( totalRotation < 0 ? -800*force : 800*force );

        }
    }

    return true;
}

//-----------------------

bool main_menu::init_box2d(void)
{
    //create world
    bool doSleep=true;
    m_World=new b2World( b2Vec2(0.0,0.0),doSleep );

    return true;
}

bool main_menu::add_new_player(int player_number)
{
    cout<<"Added new player "<<player_number<<endl;

    int xpos=0; int ypos=15;
    switch(player_number)//decide position
    {
        case 0:
        {
            xpos=26; ypos=15;
        }break;

        case 1:
        {
            xpos=26; ypos=38;
        }break;

        case 2:
        {
            xpos=70; ypos=15;
        }break;

        case 3:
        {
            xpos=70; ypos=38;
        }break;
    }

    b2BodyDef bodydef;
    bodydef.position.Set(xpos,ypos);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=2;
    bodydef.angularDamping=15;
    cout<<"innan\n";
    m_p_flipper_body[ player_number ]=m_World->CreateBody(&bodydef);
    //create fixture
    cout<<"efter\n";
    b2PolygonShape shape2;
    b2Vec2 p1(-0.7*m_f_size_ratio,0*m_f_size_ratio); b2Vec2 p2(-0.58*m_f_size_ratio,-0.4*m_f_size_ratio);
    b2Vec2 p3(0*m_f_size_ratio,-0.64*m_f_size_ratio); b2Vec2 p4(0.58*m_f_size_ratio,-0.4*m_f_size_ratio);
    b2Vec2 p5(0.7*m_f_size_ratio,0*m_f_size_ratio); b2Vec2 p6(0.22*m_f_size_ratio,1.82*m_f_size_ratio);
    b2Vec2 p7(0*m_f_size_ratio,1.94*m_f_size_ratio); b2Vec2 p8(-0.22*m_f_size_ratio,1.84*m_f_size_ratio);
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
    m_p_flipper_body[player_number]->SetUserData(data);



    return true;
}

bool main_menu::remove_player(int player_number)
{
    st_body_user_data* data=static_cast<st_body_user_data*>( m_p_flipper_body[ player_number ]->GetUserData() );
    int nummer=data->i_player_number;
    cout<<"Removing player "<<nummer<<endl;
    delete m_p_flipper_body[ player_number ]->GetUserData();
    m_p_flipper_body[ player_number ]->GetWorld()->DestroyBody( m_p_flipper_body[ player_number ] );

    return true;
}
