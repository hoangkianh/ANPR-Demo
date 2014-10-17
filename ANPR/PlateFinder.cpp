#include "PlateFinder.h"

const CvScalar RED = CV_RGB(255,0,0);
const CvScalar GREEN = CV_RGB(0,255,0);
const CvScalar BLUE = CV_RGB(0,0,255);

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

	/*cvShowImage("Source" , src);
	cvShowImage("mImg", mImg);	
	cvShowImage("mini_thresh", mini_thresh);	
	cvShowImage("dst_clone", dst_clone);	
	cvShowImage("dst", dst);*/

	cvPyrUp(dst, src);

	cvReleaseImage(&mini_thresh);
	cvReleaseImage(&mImg);
	cvReleaseImage(&tmp);
	cvReleaseImage(&dst);
	cvReleaseImage(&src_pyrdown);
	cvReleaseImage(&thresholed);
	cvReleaseImage(&dst_clone);
}

IplImage* PlateFinder::FindPlate (IplImage *src) {
	IplImage* plate;
	IplImage* contourImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);	// anh tim contour
	IplImage* grayImg =  cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);	// anh xam
	cvCvtColor(src, grayImg, CV_RGB2GRAY);

	IplImage* cloneImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cloneImg = cvCloneImage(src);
	
	// tien xu ly anh
	cvCopy(grayImg, contourImg);
	cvNormalize(contourImg, contourImg, 0, 255, CV_MINMAX);
	ImageRestoration(contourImg);
	
	IplImage* rectImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvMerge(contourImg, contourImg, contourImg, NULL, rectImg); // tron anh

	// tim contour cua buc anh
	CvMemStorage *storagePlate = cvCreateMemStorage(0);
	CvSeq *contours = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), storagePlate);
	cvFindContours(contourImg, storagePlate, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	//cvShowImage("contourImg", contourImg);
	

	int xmin, ymin, xmax, ymax, w, h, s, r;
	int count;
	double ratio;	// ty le chieu rong tren chieu cao
	CvRect rectPlate; 

	// luu lai cac anh co kha nang la bien so
	IplImage** plateArr = new IplImage *[5];
	int j = 0;
	for (int i = 0; i < 5; i++)
	{
		plateArr[i] = NULL;
	}

	while (contours) {
		count = contours->total;
		CvPoint *PointArray = new CvPoint[count];
		cvCvtSeqToArray (contours, PointArray, CV_WHOLE_SEQ);

		for (int i = 0; i < count; i++)
		{
			if (i == 0)
			{
				xmin = xmax = PointArray[i].x;
				ymin = ymax = PointArray[i].y;
			}

			if (PointArray[i].x > xmax) {
				xmax = PointArray[i].x;
			}
			if (PointArray[i].x < xmin)  {
				xmin = PointArray[i].x;
			}

			if (PointArray[i].y > ymax) {
				ymax = PointArray[i].y;
			}
			if (PointArray[i].y < ymin)  {
				ymin = PointArray[i].y;
			}
		}

		w = xmax - xmin;
		h = ymax - ymin;
		s = w * h;

		cvRectangle (rectImg, cvPoint(xmin, ymin), cvPoint(xmax, ymax), RED);

		// loai bo nhung hinh chu nhat co ti le khong dung
		if (s != 0) {
			r = (contourImg->height * contourImg->width) / s;
		} else {
			r = 1000;
		}

		if (w == 0 && h == 0) {
			ratio = 0;
		} else {
			ratio = (double)w/h;
		}

		if (r > 30 && r < 270) {
			// ve ra hcn mau xanh la
			cvRectangle (rectImg, cvPoint(xmin, ymin), cvPoint(xmax, ymax), GREEN);

			if (ratio > 2.6 && ratio < 7) {
				cvRectangle (rectImg, cvPoint(xmin, ymin), cvPoint(xmax, ymax), BLUE);

				if (w > 80 && w < 250 && h > 25 && h < 150) {
					rectPlate = cvRect (xmin, ymin, w, h);

					cvRectangle (cloneImg, cvPoint(rectPlate.x, rectPlate.y),
						cvPoint(rectPlate.x + rectPlate.width, rectPlate.y + rectPlate.height), RED, 3);

					// cat bien so
					plate = cvCreateImage(cvSize(rectPlate.width, rectPlate.height), IPL_DEPTH_8U, 3);
					cvSetImageROI(src, rectPlate);
					cvCopy(src, plate, NULL);
					cvResetImageROI(src);

					// luu vao mang cac bien so plateArr
					int cnt = CountCharacter(plate);
					if (cnt >= 5) {
						plateArr[j] = plate;
						j++;
					}
				}
			}
		}

		delete []PointArray;

		contours = contours->h_next;
	}

	// sap xep
	if (plateArr[0]) 
	{
		int w = plateArr[0]->width;

		int flag;
		for (int i = 1; i < 4; i++)
		{
			if (plateArr[i] && plateArr[i]->width < w)
			{
				flag = i;
			}
		}

		plateArr[0] = plateArr[flag];
	}

	cvShowImage("cloneImg", cloneImg);
	cvShowImage("rectImg", rectImg);
	cvShowImage("plate", plateArr[0]);

	cvReleaseImage(&contourImg);
	cvReleaseImage(&rectImg);
	cvReleaseImage(&plate);

	return plateArr[0];
}


int PlateFinder::CountCharacter(IplImage *plate) {
	int cnt = 0;
	IplImage *resizeImg, *grayImg, *binaryImg;

	resizeImg = cvCreateImage (cvSize(408, 70), IPL_DEPTH_8U, 3);
	grayImg = cvCreateImage (cvSize(408, 70), IPL_DEPTH_8U,  1);
	binaryImg = cvCreateImage (cvSize(408, 70), IPL_DEPTH_8U,  1);

	cvResize(plate, resizeImg);
	cvCvtColor(resizeImg, grayImg, CV_RGB2GRAY);
	cvAdaptiveThreshold(grayImg, binaryImg, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 13, 2);

	//cvShowImage("binaryImg", binaryImg);

	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), storage);
	cvFindContours(binaryImg, storage, &contours);
	
	//cvShowImage("contours", binaryImg);

	//CvSeq *contours = 0;
	//cvFindContours(binaryImg, storage, &contours);

	while (contours) {
		CvRect rect = cvBoundingRect(contours);

		if (rect.width > 15 && rect.width < 50
			&& rect.height > 40 && rect.height < 65
			&& rect.width * rect.height > 1000) 
		{
			cvRectangle (resizeImg, cvPoint(rect.x, rect.y), 
				cvPoint(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

			cnt++;
		}
		contours = contours->h_next;
	}

	cvShowImage("resizeImg", resizeImg);

	return cnt;
}