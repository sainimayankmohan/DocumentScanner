#include <opencv2/opencv.hpp>
#include <iostream>
#include "dataPath.hpp"

using namespace cv;
using namespace std;

void getOrderedPoints(vector<Point> input,vector<Point2f>& output)
{
	//The top left point of the document will be the point whose sum of x and y is the minimum
	//and the bottom right point will be having the largest sum of x and y
	//The vector stores the data in descending order i.e. larger first
	vector<Point> sum = input;
	sort(sum.begin(), sum.end(),
	    [](Point p1, Point p2) {return (p1.x  + p1.y) > (p2.x + p2.y);});

	//As we already found the 0th and 2nd pint i.e. top left and bottom right, in the sum vector on position 0th and 3rd
	//Hence we can now eliminate these 2 points
	vector<Point> diff;
	diff.push_back(sum[1]);
	diff.push_back(sum[2]);

	//Now the top right point will be having the larger value of x than the bottom left point
	//The vector stores the data in descending order i.e. larger first
	sort(diff.begin(), diff.end(),
	    [](Point p1, Point p2) {return p1.x > p2.x ;});

	output.push_back(sum[3]);//Smallest sum for top left
	output.push_back(diff[0]);//Larger X than bottom left for top right
	output.push_back(sum[0]);//Largest sum for bottom right
	output.push_back(diff[1]);////Smaller X than top right for bottom left
}


int main()
{
	Mat image = imread(DATA_PATH + "images/scanned-form.jpg");

	resize(image,image,Size(500,700));

	Mat imgCopy = image.clone();

	Mat gray;

	cvtColor(imgCopy,gray,COLOR_BGR2GRAY);

	Mat blurred;

	GaussianBlur(gray,blurred,Size(5,5),0);

	Mat edges;

	Canny( blurred, edges, 30, 50);

	// Find all contours in the image
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(edges, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

	sort(contours.begin(), contours.end(), [](const vector<Point>& c1, const vector<Point>& c2){
	    return contourArea(c1, false) > contourArea(c2, false);	});

	//Now the largest contour will be the document that is placed on 0th position in the vector after sorting

	vector<Point> docContour = contours[0];

	double perimeter = arcLength(docContour,true);

	approxPolyDP(docContour,docContour, 0.02*perimeter, true);

	vector<Point2f> doc;

	getOrderedPoints(docContour,doc);

	// Create a vector of destination points.
	vector<Point2f> pts_dst;

	pts_dst.push_back(Point2f(0,0));
	pts_dst.push_back(Point2f(500, 0));
	pts_dst.push_back(Point2f(500, 700));
	pts_dst.push_back(Point2f(0, 700 ));

	Mat pers = getPerspectiveTransform(doc, pts_dst);

	warpPerspective(image, image, pers, Size(500,700));

	imshow("Original",imgCopy);
	imshow("Scanned",image);
	waitKey(0);
	return 0;
}
