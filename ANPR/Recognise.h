#pragma once

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv\cxcore.h>
#include <iostream>

using namespace std;

class Recognise {
public:
	Recognise();
	vector<IplImage *> FindCharacter (IplImage *plate);
};