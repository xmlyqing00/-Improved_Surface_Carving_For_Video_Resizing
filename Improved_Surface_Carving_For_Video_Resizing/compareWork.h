/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : compareWork.h
*	Creation Time : 2015-5-22
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef COMPARE_WORK
#define COMPARE_WORK

#include "baseFunction.h"

int generalCrop( vector<Mat> &pixelEnergy, vector<Mat> &edgeProtect, int widthDeleted ) {

	int frameCount = pixelEnergy.size();
	Size frameSize = pixelEnergy[0].size();

	vector<int> evaluate( frameSize.width, 0 );
	int step = frameSize.width / widthDeleted;
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x ++ ) {
				evaluate[x] += pixelEnergy[t].ptr<uchar>( y )[x] + edgeProtect[t].ptr<uchar>( y )[x];
			}
		}
	}

	sort( evaluate.begin(), evaluate.end() );
	
	int avgEvaluate = 0;
	for ( int i = 0; i < widthDeleted; i++ ) avgEvaluate += evaluate[i];
	avgEvaluate /= widthDeleted;
	int midEvaluate = evaluate[widthDeleted / 2];
	int maxEvaluate = evaluate[widthDeleted - 1];
	int terminalEvaluate = cvRound( evaluate[0] + 0.8 * (maxEvaluate - evaluate[0]) );

	cout << " Crop Avg Destroy : " << avgEvaluate << endl;
	cout << " Crop Mid Destroy : " << midEvaluate << endl;
	cout << " Crop Max Destroy : " << maxEvaluate << endl;
	cout << " Terminate Evaluate : " << terminalEvaluate << endl;

	return terminalEvaluate;

}

void generalScale( vector<Mat> &pixelEnergy, vector<Mat> &edgeProtect, int widthDeleted ) {

	int frameCount = pixelEnergy.size();
	Size frameSize = pixelEnergy[0].size();

	long long sum = 0;
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {
				sum += pixelEnergy[t].ptr<uchar>( y )[x] + edgeProtect[t].ptr<uchar>( y )[x];
			}
		}
	}

	Mat resizeEnergy, resizeEdge;
	long long newSum = 0;
	for ( int t = 0; t < frameCount; t++ ) {

		resize( pixelEnergy[t], resizeEnergy, Size( frameSize.width - widthDeleted, frameSize.height ) );
		resize( edgeProtect[t], resizeEdge, Size( frameSize.width - widthDeleted, frameSize.height ) );

		int newWidth = frameSize.width - widthDeleted;
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < newWidth; x++ ) {
				newSum += resizeEnergy.ptr<uchar>( y )[x] + resizeEdge.ptr<uchar>( y )[x];
			}
		}

	}

	sum -= newSum;
	sum /= widthDeleted;
	cout << sum << endl;
}

#endif