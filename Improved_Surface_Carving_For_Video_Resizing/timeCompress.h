/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : timeCompress.h
*	Creation Time : 2015-5-31
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef TIME_COMPRESS
#define TIME_COMPRESS

#include "baseFunction.h"

bool cmp_v( const pair<double, int> &p0, const pair<double, int> &p1 ) {

	return p0.first > p1.first;
}
bool cmp_index( const pair<double, int> &p0, const pair<double, int> &p1 ) {

	return p0.second < p1.second;
}

void timeCompress( vector<Mat> &pixelEnergy, vector<int> &keyFrame, double keyFrameNumLimit ) {

	cout << " Temporal Compress --ING" << endl;

	Mat diffFrame;
	vector<int> diffArr;
	vector< pair<int, int> > candidateArr;
	int keyFrameNum = (int)(keyFrameNumLimit * pixelEnergy.size() + 1);
	int normalFrameNum = 0;

	for ( int i = 1; i < (int)pixelEnergy.size(); i++ ) {

		absdiff( pixelEnergy[i], pixelEnergy[i - 1], diffFrame );
		Scalar m = mean( diffFrame );

		diffArr.push_back( (int)(m.val[0] + 0.5) );

		if ( i >= 7 ) {

			vector< pair<int, int> > sortArr;
			pair<int, int> x;
			for ( int j = i - 7; j < i; j++ ) {
				x.first = diffArr[j];
				x.second = j + 1;
				sortArr.push_back( x );
			}

			std::sort( sortArr.begin(), sortArr.end() );

			if ( sortArr[5].first != 0 ) {

				if ( candidateArr.size() > 0 ) {
					if ( candidateArr[candidateArr.size() - 1].second != sortArr[6].second ) {
						sortArr[6].first = sortArr[6].first / sortArr[5].first;
						candidateArr.push_back( sortArr[6] );
						normalFrameNum++;
					}
				} else {
					sortArr[6].first = sortArr[6].first / sortArr[5].first;
					candidateArr.push_back( sortArr[6] );
					normalFrameNum++;
				}
			}
		}
	}

	std::sort( candidateArr.begin(), candidateArr.end() );
	
	keyFrame.push_back( 0 ); 

	int n = candidateArr.size();
	if ( n == 0 ) return;

	for ( int i = 1; i < keyFrameNum; i++ ) {
		
		if ( n - i < 0 ) break;

		if ( candidateArr[n - i].first == 1 ) {
			double gap = (double)normalFrameNum / ( keyFrameNum - i );
			for ( double j = n - i; j > 0; j -= gap ) {
				int k = (int)j;
				if ( candidateArr[k].first == 0 ) break;
				if ( keyFrame.size() > 0 && keyFrame[keyFrame.size() - 1] != candidateArr[k].second ) {
					keyFrame.push_back( candidateArr[k].second );
				}
				//_ASSERT( _CrtCheckMemory() );
			}
			break;
		}
		keyFrame.push_back( candidateArr[n - i].second );
	}
	std::sort( keyFrame.begin(), keyFrame.end() );
}

void preserveKeyData( vector<int> &keyFrame, vector<Mat> &frames, vector<Mat> &pixelEnergy ) {

	int n = keyFrame.size();

	cout << " Key Frame Num : " << n << endl;

	for ( int i = 0; i < n; i++ ) {
		//cout << keyFrame[i] << endl;
		frames[i] = frames[keyFrame[i]];
		pixelEnergy[i] = pixelEnergy[keyFrame[i]];
	}

	frames.resize( n );
	pixelEnergy.resize( n );

	//remove( "rmdir .\\keyEnergy" );
	//remove( "rmdir .\\keyEdge" );
	/*
	for ( int i = 0; i < n; i++ ) {

	sprintf( pngName, "keyFrame//%d.png", keyFrame[i] );
	imwrite( pngName, frames[i] );
	sprintf( pngName, "keyEnergy//%d.png", keyFrame[i] );
	imwrite( pngName, pixelEnergy[i] );
	}*/
}

#endif