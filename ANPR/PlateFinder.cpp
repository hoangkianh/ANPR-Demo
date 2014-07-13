#include "PlateFinder.h"


PlateFinder::PlateFinder(void)
{
	S1 = cvCreateStructuringElementEx(3, 1, 1, 0, CV_SHAPE_RECT, NULL);
	S2 = cvCreateStructuringElementEx(6, 1, 3, 0, CV_SHAPE_RECT, NULL);
	plate = NULL;
}


PlateFinder::~PlateFinder(void)
{
	if (plate)
	{
		S1 = NULL;
		S2 = NULL;
		plate = NULL;
	}
}

void PlateFinder::ImageRestoration(IplImage *src)
{
	int w = src->width;
	int h = src->height;

	IplImage *mImg = cvCreateImage(cvSize(w/2, h/2), IPL_DEPTH_8U, 1);		// Anh su dung cho bien doi hinh thai hoc
	IplImage *src_pyrdown = cvCreateImage (cvSize(w/2, h/2), IPL_DEPTH_8U, 1);
	IplImage *tmp = cvCreateImage (cvSize(w/2, h/2), IPL_DEPTH_8U, 1);
	IplImage *thresholed = cvCreateImage (cvSize(w/2, h/2), IPL_DEPTH_8U, 1);	// Anh nhi phan voi nguong
	IplImage *mini_thresh = cvCreateImage (cvSize(w/2, h/2), IPL_DEPTH_8U, 1);
	IplImage *dst = cvCreateImage (cvSize(w/2, h/2), IPL_DEPTH_8U, 1);			// Anh lam ro vung bien so

	cvPyrDown (src, src_pyrdown);

	cvMorphologyEx(src_pyrdown, mImg, tmp, S2, CV_MOP_BLACKHAT);
	cvNormalize(mImg, mImg, 0, 255, CV_MINMAX);


	// Nhi phan hoa anh mImg
	cvThreshold(mImg, thresholed, (int)10*cvAvg(mImg).val[0], 255, CV_THRESH_BINARY);
	cvZero(dst);
	cvCopy(thresholed, mini_thresh);

	// Su dung hinh chu nhat co size = 8x16 truot tren toan bo anh
	
	int cnt;
	int nonZero1, nonZero2, nonZero3, nonZero4;
	CvRect rect;

	for (int i = 0; i < mini_thresh->width-32; i+=4)
	{
		for (int j = 0; j  < mini_thresh->height-16; j+=4)
		{
			rect = cvRect(i, j, 16, 8);
			cvSetImageROI (mini_thresh, rect);	//ROI = Region of Interest
			nonZero1 = cvCountNonZero(mini_thresh);
			cvResetImageROI(mini_thresh);

			rect = cvRect(i+16, j, 16, 8);
			cvSetImageROI (mini_thresh, rect);	//ROI = Region of Interest
			nonZero2 = cvCountNonZero(mini_thresh);
			cvResetImageROI(mini_thresh);

			rect = cvRect(i, j+8, 16, 8);
			cvSetImageROI (mini_thresh, rect);	//ROI = Region of Interest
			nonZero3 = cvCountNonZero(mini_thresh);
			cvResetImageROI(mini_thresh);

			rect = cvRect(i+16, j+8, 16, 8);
			cvSetImageROI (mini_thresh, rect);	//ROI = Region of Interest
			nonZero4 = cvCountNonZero(mini_thresh);
			cvResetImageROI(mini_thresh);

			cnt = 0;
			if (nonZero1 > 15) { cnt++; }
			if (nonZero2 > 15) { cnt++; }
			if (nonZero3 > 15) { cnt++; }
			if (nonZero4 > 15) { cnt++; }

			if (cnt > 2)
			{
				rect = cvRect (i, j, 32, 16);
				cvSetImageROI(dst, rect);
				cvSetImageROI(mini_thresh, rect);
				cvCopy(mini_thresh, dst);
				cvResetImageROI(dst);
				cvResetImageROI(mini_thresh);
			}
		}
	}

	IplImage* dst_clone = cvCloneImage(dst);

	cvDilate(dst, dst, NULL, 2);
	cvErode(dst, dst, NULL, 2);
	cvDilate(dst, dst, S1, 9);
	cvErode(dst, dst, S1, 10);
	cvDilate(dst, dst);

	cvShowImage("Source" , src);
	cvShowImage("mImg", mImg);	
	cvShowImage("mini_thresh", mini_thresh);	
	cvShowImage("dst_clone", dst_clone);	
	cvShowImage("dst", dst);

	cvPyrUp(dst, src);

	cvReleaseImage(&mini_thresh);
	cvReleaseImage(&mImg);
	cvReleaseImage(&tmp);
	cvReleaseImage(&dst);
	cvReleaseImage(&src_pyrdown);
	cvReleaseImage(&thresholed);
	cvReleaseImage(&dst_clone);
}
