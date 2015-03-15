#include "tesseract\baseapi.h"
#include <iostream>
#include "leptonica\allheaders.h"
#include "opencv2\world.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <chrono>
#include <sapi.h>
using namespace std;
using namespace cv;

tesseract::TessBaseAPI Init_tesseract();
STRING readText(tesseract::TessBaseAPI *api, Mat binary);
Mat segment(Mat );
int confidence;
bool flag = false;
int match_method;
Mat img , templ, result,dst;
void MatchingMethod(Mat, void*);
Point center;
Point line_up;
Point line_up_end;
Point line_bot;
Point line_bot_end;
int id;
Mat perspective(Rect , Mat );
int l = 0;
int main(void){

	//initalize camera
	VideoCapture cap(0);
	if (!cap.isOpened()){
		cout << "Cannot open Camera!" << endl;
		system("pause");
		return -1;
	}
	//Points for plotting line
	line_up = Point(0, 0);
	line_up_end = Point(0,0);
	line_bot = Point(0,0);
	line_bot_end = Point(0,0);
	//Mat for processing
	Mat frame;
	Mat gray;
	Mat binary;
	
	namedWindow("Finger",WINDOW_AUTOSIZE);
	namedWindow("ROI", WINDOW_KEEPRATIO);

	//define TTS variables
	STRING text_out;
	std::string count_text[8];
	std::fill_n(count_text,8,"");
	ISpVoice * pVoice = NULL;
	std::wstring tts;
	if (FAILED(::CoInitialize(NULL)))
		cout << "Couldn't configure text to speech" << endl;
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	string temp = "";

	//Initialize tesseract OCR
	tesseract::TessBaseAPI api = Init_tesseract();

	//Template
	templ = imread("template.jpg", 1);
	
	while (1){
		
		//Read frames
		bool read = cap.read(frame);
		if (!read){
			cout << endl<<"Cannot read frame" << endl;
			system("pause");
			return -1;
		}

		Mat copy1;
		frame.copyTo(copy1);
		//Segment function outputs ROI
		Mat detect = segment(copy1);
		imshow("ROI", detect);
		
		//Increasing detection accuracy...
		double minVal; double maxVal;
		Mat m = detect.clone();
		minMaxLoc(m, &minVal, &maxVal);
		if (maxVal > 1){

				cvtColor(m, gray, COLOR_BGR2GRAY);
				threshold(gray, binary, 128, 255, THRESH_BINARY | THRESH_OTSU);
				bitwise_not(binary, dst);
				text_out = readText(&api, dst);
				if (text_out != "" && l < 8){
					//cout << "Text output:" << text_out.string() << endl;
					count_text[l] = text_out.string();
					l++;
				}
				if (l == 7){
					if (temp != count_text[(l + 1) / 2]){
						//TTS done here...
						tts.assign(count_text[(l + 1) / 2].begin(), count_text[(l + 1) / 2].end());
						hr = pVoice->Speak((LPCWSTR)tts.c_str(), SPF_ASYNC, NULL);
						cout << "Text output:" << count_text[(l + 1) / 2] << endl;
					}
						
						temp = count_text[(l + 1) / 2];
				}	

			}
				
			else
			{
				imshow("ROI", frame);
				l = 0;
			}
		

		if (waitKey(30) == 27){
			cout << endl<<"Escape key pressed" << endl;
			system("pause");
			pVoice->Release();
			pVoice = NULL;
			::CoUninitialize();
			break;
		}

		
	}

	return 0;
}

//Functions
tesseract::TessBaseAPI Init_tesseract(){
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	api->Init("", "eng", tesseract::OEM_TESSERACT_ONLY);
	return *api;
}

STRING readText(tesseract::TessBaseAPI *api, Mat binary){
	STRING text_out;
	api->SetImage((uchar*)binary.data, binary.cols, binary.rows, 1, binary.cols);
	text_out = api->GetUTF8Text();
	confidence = api->MeanTextConf();
	if (confidence > 80)
		return text_out;
	return "";
}



void MatchingMethod(Mat src, void*)
{
	/// Source image to display
	Mat img_display;
	src.copyTo(img_display);

	/// Create the result matrix
	int result_cols = src.cols - templ.cols + 1;
	int result_rows = src.rows - templ.rows + 1;

	result.create(result_cols, result_rows, CV_32FC1);

	/// Do the Matching and Normalize
	matchTemplate(src, templ, result, match_method);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
	if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}
	center.x = (2 * matchLoc.x + templ.cols) / 2;
	center.y = (2 * matchLoc.y + templ.rows) / 2;
	if (matchLoc.y - 10 > line_bot.y)
		cout << "Going down" << endl;
	if (matchLoc.y < line_up.y)
		cout << "Going Up" << endl;

	rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	line(img_display, center , Point(((2 * matchLoc.x + templ.cols) / 2), 0), Scalar::all(0), 2, 8, 0);
	imshow("Finger", img_display);
	
	return;
}



Mat segment(Mat large)
{
	
	Mat rgb = large.clone();
	Mat output = rgb.clone();
	GaussianBlur(rgb, rgb,Size(17,17),0,0);
	Mat small;
	cvtColor(rgb, small, COLOR_BGR2GRAY);

	// morphological gradient
	Mat grad;
	Mat morphKernel = getStructuringElement(MORPH_ERODE, Size(3, 3));
	morphologyEx(small, grad, MORPH_GRADIENT, morphKernel);
	// binarize
	Mat bw;
	threshold(grad, bw, 128, 255.0, THRESH_BINARY | THRESH_OTSU);

	// connect horizontally oriented regions
	Mat connected;
	morphKernel = getStructuringElement(MORPH_RECT, Size(9, 1));
	morphologyEx(bw, connected, MORPH_CLOSE, morphKernel);
	// find contours
	Mat mask = Mat::zeros(bw.size(), CV_8UC1);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(connected, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));
	
	Mat roi ,roi1;
	roi = Mat::zeros(bw.size(), CV_32FC1);
	roi1 = Mat::zeros(bw.size(), CV_32FC1);
	 for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
	 {
		 Rect rect = boundingRect(contours[idx]);
		 Point2f top_left = rect.tl();
		 Point2f bottom_right = rect.br();
		 if (bottom_right.y<output.rows*0.3 || top_left.y > output.rows*0.8)
			 continue;
		 
		 float x = (bottom_right.x);
		 Mat maskROI(mask, rect);
		 maskROI = Scalar(0, 0, 0);

		 // fill the contour
		 drawContours(mask, contours, idx, Scalar(255, 255, 255), FILLED);
		 // ratio of non-zero pixels in the filled region
		 double r = (double)countNonZero(maskROI) / (rect.width*rect.height);
		// roi1.create(rect.width, rect.height, CV_32FC1);
		 if (r > .45 /* assume at least 45% of the area is filled if it contains text */
			 &&
			 (rect.height > 25 && rect.width > 25) /* constraints on region size */
			 /* these two conditions alone are not very robust. better to use something
			 like the number of significant peaks in a horizontal projection as a third condition */
			 )
		 {
			 rectangle(output, rect, Scalar(0, 255, 0), 2);
			 line_up.y = top_left.y - 20;
			 line_up.x = 0;
			 line_up_end.x = output.cols;
			 line_up_end.y = line_up.y;
			 line_bot.y = bottom_right.y + 20;
			 line_bot.x = 0;
			 line_bot_end.x = output.cols;
			 line_bot_end.y = line_bot.y;
			 if (center.x > top_left.x && center.x < x)
			 {
				 
				 roi = output(rect);
				 roi1 = roi.clone();
				 break;
			 }
			 else
			 {
				 roi1 = Scalar::all(0);
			 }

		 }
		 
	 }
	
		 line(output, line_up, line_up_end, Scalar(0, 0, 255), 2);
		 line(output, line_bot, line_bot_end, Scalar(255, 0, 0), 2);
	 
	 MatchingMethod(output, 0);

	 return roi1;
}



