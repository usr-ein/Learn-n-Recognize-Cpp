//
//  main.cpp
//  learn-n-recognize
//
//  Created by Samuel Prevost on 11/11/2016.
//  Copyright © 2016 Samuel Prevost. All rights reserved.
//

#include <iostream>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include "Database.hpp"
#include "Message.hpp"
#include "ArgumentManager.hpp"
#include "HaarCascade.hpp"
#include "LBPRecognizer.hpp"

// Set default mode to scan
#define DEFAULT_MODE SCAN
// Set the number of frame before learning the stack
#define FRAMES_BEFORE_LEARNING 30
// Used namespaces
using cv::VideoCapture;
using cv::waitKey;

void SaveAndExit(LBPRecognizer* rec);

/** Global variables **/

/* Modes:
 *  - SCAN  : Scanning mode, detect faces and write their name
 *  - LEARN : Learning mode, learn new faces by associating them with their name
 */
enum Mode { SCAN, LEARN };
// The selected mode
Mode currentMode = DEFAULT_MODE;

int main(int argc, const char * argv[]){
    // Clear the console
    ClearMessage();
    // Display credits
    CreditsMessage();
    // Display current OpenCV version
    // (3.1.0-dev for now)
    VersionMessage(CV_VERSION);
    // Arguments handler, check if every args are provided
    ArgumentManager* am = new ArgumentManager(argc, argv);

    // Database containing subject's name and id
    Database* db;
    if(am->database_path != "")
        db = new Database(am->database_path);
    else
        db = new Database();

    // Haar Cascade detect human faces and get us their positions
    HaarCascade* hc = new HaarCascade(am->face_cascade_path);

    // LBPR recognize and identify the faces, it gives us their id
    LBPRecognizer* rec;
    if(am->recognizer_path.size() > 1)
        rec = new LBPRecognizer(am->recognizer_path);
    else
        rec = new LBPRecognizer();
    
    // Open camera image stream
    VideoCapture cap(am->cameraID);
    // If we failed, exit program
    if(!cap.isOpened()) {
        ErrorOpeningCameraMessage();
        exit(EXIT_FAILURE);
    }
    
    // This will contain the position of every faces
    vector<Rect> faces;
    // This will contain the current frame we will work with
    Mat currentFrame;
    // This will contain the name of the subject we are learning the face
    string subject_name;
    // This will contain the id of the subject we are learning the face
    int subject_id = NULL;
    
    // This will contain some of the last frames to update/train the recognizer later with it
    vector<Mat> framesToLearn;
    
    while (true) {
        switch (currentMode) {
            // Scanning mode
            case SCAN:
                ScanningModeMessage();
                Countdown(3);
                while (true){
                    // Save the current frame
                    cap >> currentFrame;
                    // Detect every faces on it
                    faces = hc->detectFaces(&currentFrame);
                    for(int i = 0; i < faces.size(); i++){
                        // if we've initialized our LBPR
                        if(!rec->isEmpty()){
                            // id of the detected face
                            int id = -1;
                            // confidence of the detection
                            double confidence = 0.0;
                            // currentFrame(faces[i]) crop workingFrame to the Rect faces[i]
                            // Get subject's id from image using LBPH
                            rec->recognize(currentFrame(faces[i]), &id, &confidence);
                            // Draw detected name and confidence on the current frame
                            rec->drawNameAndConf(&currentFrame, faces[i], db->getSubjectName(id), to_string(confidence));
                        }else{
                            // otherwise, we can't recognize the faces
                            rec->drawNameAndConf(&currentFrame, faces[i], " ? ", "-");
                        }
                    }
                    // Draw rectangle around every faces
                    hc->drawRect(&currentFrame, faces);
                    // Display the frame
                    imshow("Learn'n'Recognize", currentFrame);
                    
                    // Wait 1ms for key
                    char key = (char)waitKey(1);
                    // Save and exit if we pressed 'q'
                    if( key == 'q' ) { SaveAndExit(rec);}
                    // If we pressed 'l'
                    if( key == 'l' ) {
                        // Go to learning
                        currentMode = LEARN;
                        break;
                    }
                }
                break;


            // Learning mode
            case LEARN:
                // Ask user if the subject exist in the database
                if (DoesSubjectExist()){
                    /* Subject already exist in database */

                    // Get his name and id
                    AskSubjectNameAndID(&subject_name, &subject_id, db);
                    
                }else{
                    /* New subject */

                    subject_name = AskNewSubjectName(db);
                    // Insert subject into db and get his new id
                    subject_id = db->insertSubject(subject_name);
                    // Exit learning mode if we failed to insert the subject into db
                    if(subject_id == -1){
                        currentMode = SCAN;
                        break;
                    }
                }

                // Tell the user what is happening
                LearningModeMessage();
                // Wait 7 sec
                Countdown(7);
                
                while (true){
                    // Save the current frame
                    cap >> currentFrame;
                    // Detect every faces on it
                    faces = hc->detectFaces(&currentFrame);
                    
                    // Theoretically faces.size() = 1 , but meh
                    for (int i = 0; i < faces.size(); i++) {
                        // Here, we add the faces to a stack to learn them when they'll be
                        // a bunch of them
                        framesToLearn.push_back(currentFrame(faces[i]));
                    }
                    
                    // If we've got enough frames
                    if(framesToLearn.size() > FRAMES_BEFORE_LEARNING){
                        vector<int> labels(framesToLearn.size());
                        for (int i = 0; i < framesToLearn.size(); i++) {
                            labels[i] = subject_id;
                        }
                        if(rec->isEmpty())
                            rec->train(framesToLearn, labels);
                        else
                            rec->update(framesToLearn, labels);
                        // Since we don't want to re-learn the same frame, clear the stack
                        framesToLearn.clear();
                    }
                    
                    // Draw rectangles around faces
                    hc->drawRect(&currentFrame, faces);
                    
                    // Display the frame
                    imshow("Learn'n'Recognize", currentFrame);
                    
                    // Wait 1ms for key
                    char key = (char)waitKey(1);
                    // Save and exit if we pressed 'q'
                    if( key == 'q' ) { SaveAndExit(rec);}
                    // If we pressed 's' or spacebar
                    if( key == 's' || key == ' ') {
                        // Reset name and id
                        subject_id = NULL;
                        subject_name = "";
                        // Go to scanning mode
                        currentMode = SCAN;
                        break;
                    }
                }
                break;
        }
    }
    return 0;
}

void SaveAndExit(LBPRecognizer* rec){
    // Save our recognizer
    rec->save(AskWhereToSaveRecognizer() + "/LBPH_recognizer.xml");
    // Say goodbye
    ExitMessage();
    exit(EXIT_SUCCESS);
    return;
}
