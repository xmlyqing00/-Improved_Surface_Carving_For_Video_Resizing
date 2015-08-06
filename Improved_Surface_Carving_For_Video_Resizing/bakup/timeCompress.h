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

void timeCompress( vector<Mat> &pixelEnergy, vector<int> &keyFrame, int keyFrameNumLimit ) {

	cout << " Temporal Compress --ING" << endl;

	int frameCount = pixelEnergy.size();
	Size frameSize = pixelEnergy[0].size();

	vector<Mat> rotFrames;
	Mat rotFrame = Mat( frameSize.width, frameCount, CV_8UC1 );

	for ( int i = 0; i < frameSize.height; i++ ) {
		for ( int y = 0; y < rotFrame.rows; y++ ) {
			uchar *rowData = rotFrame.ptr<uchar>( y );
			for ( int x = 0; x < rotFrame.cols; x++ ) {
				rowData[x] = pixelEnergy[x].ptr<uchar>( i )[y];
			}
		}
		rotFrames.push_back( rotFrame.clone() );
	}

	Size rotSize = rotFrames[0].size();
	int rotCount = rotFrames.size();
	Mat gradX, absGradX, tempMat;
	vector<int> sum( rotSize.width, 0 );

	for ( int i = 0; i < rotCount; i++ ) {

		Sobel( rotFrames[i], gradX, CV_16S, 1, 0, 3 );
		convertScaleAbs( gradX, absGradX );
		absGradX = 0.5*absGradX;
		absGradX.convertTo( tempMat, CV_8UC1 );

		for ( int y = 0; y < rotSize.height; y++ ) {
			for ( int x = 0; x < rotSize.width; x++ ) {
				sum[x] += tempMat.ptr<uchar>( y )[x];
			}
		}
	}

	vector<double> v( rotSize.width, 0 );
	int n = rotSize.width;
	int m = min( (int)(0.1*n), keyFrameNumLimit );
	//cout << m << endl;
	for ( int x = 0; x < rotSize.width; x++ ) {

		double temp = sum[x];
		temp = temp / (double)(rotCount * rotSize.height);
		v[x] = temp + 0.01;
	}

	v[0] = v[1];
	vector< pair<double, int> > sortV;
	for ( int i = n - 1; i > 0; i-- ) {

		pair<double, int> oneV;
		oneV.first = v[i] / v[i - 1];
		oneV.second = i;
		sortV.push_back( oneV );
	}

	sort( sortV.begin(), sortV.end(), cmp_v );
	//for ( int i = 0; i < m; i++ ) cout << i << " " << sortV[i].first << " " << sortV[i].second << endl;
	sort( sortV.begin(), sortV.begin() + m - 1, cmp_index );

	keyFrame.clear();
	keyFrame.push_back( 0 );
	for ( int i = 0; i < m - 1; i++ ) keyFrame.push_back( sortV[i].second );

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