#include <string>
#include <iostream>

#ifndef FILES_IN_TEXT_H
#define FILES_IN_TEXT_H



using namespace std;

//Data files converted with Base64 to text

enum data_files//list of all files
{
    file_texture,
    file_sound_beep1,
    file_sound_dong,
    file_sound_gameover,
    file_sound_goal,
    file_sound_intro,
    file_sound_join,
    file_sound_ping,
    file_sound_music_intro,
    file_sound_music_loop
};

string load_base64_file(int file_id);//will return encoded text

#endif
