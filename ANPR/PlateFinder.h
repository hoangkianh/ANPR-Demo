#pragma once

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv\cxcore.h>
#include <iostream>


class PlateFinder
{
private:
	IplConvKernel* S1;
	IplConvKernel* S2;
	IplImage *plate;

public:
	PlateFinder(void);
	virtual ~PlateFinder(void);
	void ImageRestoration(IplImage *src);	// tien xu ly anh goc
	IplImage* FindPlate (IplImage *src);	// tim va cat bien so
	int CountCharacter(IplImage *plate);	// dem so vung co kha nang la ki tu
};