/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : shotCut.h
*	Creation Time : 2015-6-9
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef SHOT_CUT
#define SHOT_CUT

#include "baseFunction.h"

void segShotCut( int processId, vector<int> &shotArr ) {

	int frameCount = 0;
	char matName[100];
	Mat inputFrame, oldFrame, diffFrame;
	vector<int> diffArr;
	shotArr.clear();
	shotArr.push_back( 0 );

	while ( true ) {

		sprintf( matName, "frameStream%d//%d.png", processId, frameCount );
		inputFrame = imread( matName );
		if ( inputFrame.empty() ) break;

		frameCount++;
		if ( frameCount > 1 ) {

			absdiff( inputFrame, oldFrame, diffFrame );
			Scalar m = mean( diffFrame );

			int temp = 0;
			for ( int k = 0; k < 3; k++ ) temp += m.val[k];
			temp /= 3;

			diffArr.push_back( temp );
		}

		oldFrame = inputFrame.clone();

		if ( frameCount > 7 ) {

			vector< pair<int, int> > sortArr;
			pair<int, int> x;
			for ( int i = frameCount - 8; i < frameCount - 1; i++ ) {
				x.first = diffArr[i];
				x.second = i;
				sortArr.push_back( x );
			}
			sort( sortArr.begin(), sortArr.end() );

			if ( sortArr[6].first > sortArr[5].first * 3 && sortArr[5].first != 0 ) {
				shotArr.push_back( sortArr[6].second + 1 );
			}
		}
	}
	shotArr.push_back( frameCount );
}
#endif