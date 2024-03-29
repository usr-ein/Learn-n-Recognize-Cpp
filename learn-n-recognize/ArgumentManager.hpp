//
//  ArgumentManager.hpp
//  learn-n-recognize
//
//  Created by Samuel Prevost on 24/12/2016.
//  Copyright © 2016 Samuel Prevost. All rights reserved.
//

#ifndef ArgumentManager_hpp
#define ArgumentManager_hpp

#include <cstdlib>
#include <iostream>
#include <vector>
#include "Message.hpp"
#include "AskUser.hpp"
#include "Network.hpp"
#include "Miscellaneous.hpp"

#define URL_HAAR_CASCADE "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml"

using std::stoi;
using std::stof;

// Must already be included in Message.hpp
using std::cout;
using std::cin;
using std::vector;
using std::endl;
using std::string;

class ArgumentManager {
private:
    bool isGUIFlag(const char* argv[], int argc);
public:
    // On macOS:    absolute path to the program
    // On Linux:    name of the program
    // On Windows:  undefined behaviour
    string program_path;
    // Path to where the SQLite database containing name and id is
    string database_path;
    // Path to where the LBP recognizer archive is
    string recognizer_path;
    // Path to save newly created files
    string save_path = "";
    // Path to haar cascade XML file
    string face_cascade_path;
    
    // Threshold after which the program will consider the
    // guessed name as valid, about 65% - 75% is normal
    int validity_threshold = 65;
    // ID of the used camera,
    // usualy 0 if only one cam is connected
    int cameraID = 0;
    // Scaling factor of the displayed live video stream
    float video_scaling_factor = 1;
    
    ArgumentManager(int argc, const char * argv[]);
    
    bool downloadHaarFaceCascade();
    
    vector<string> AskForArgs(const char* argv_zero);
};

#endif /* ArgumentManager_hpp */
