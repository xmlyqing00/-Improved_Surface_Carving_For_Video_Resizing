/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : pixelTag.h
*	Creation Time : 2015-5-25
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef PIXEL_TAG
#define PIXEL_TAG

#include "baseFunction.h"

void calcPixelTag( vector<Mat> &frames, vector<int> &pixelTag, int &tagNum, int colorDiffThred, int elePerTagThred ) {

	int frameCount = frames.size();
	Size frameSize = frames[0].size();
	int N = frameCount * frameSize.height * frameSize.width + 2;
	const int route[10][3] = { 
		{ -1, 0, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 }, { 0, 0, -1 }, 
		{ 0, -1, 0 }, { 0, 1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }, { 0, -1, 1 }};
	Mat edges;
	vector<Vec3b> pixelColor;
	vector<Mat> pixelEdges;
	pixelTag = vector<int>( N, -1 );
	vector<int> tagReplace( N, -1 );
	vector<bool> tagAvailable( N, true );
	//queue<int> que;
	typeQue &que = *(new typeQue);
	char filename[100];
	
	pixelColor.push_back( 0 );
	for ( int t = 0; t < frameCount; t++ ) {

		for ( int y = 0; y < frameSize.height; y++ ) {
			Vec3b *rowData = frames[t].ptr<Vec3b>( y );
			for ( int x = 0; x < frameSize.width; x++ ) {
				pixelColor.push_back(rowData[x]);
			}
		}

		Mat grayFrame;
		cvtColor( frames[t], grayFrame, COLOR_RGB2GRAY );
		Canny( grayFrame, edges, 20, 60, 3, true );
		pixelEdges.push_back(edges.clone());

		sprintf( filename, "contour//%d.png", t );
		imwrite( filename, edges );
		//imshow( "canny", edges );
		//waitKey( 33 );
	}

	int pixelNum, nowPixelNum, nextPixelNum, nowT, nowX, nowY, nextT, nextX, nextY;
	int elePerTagCount;
	//int colorGradualThred = colorDiffThred / 5;
   	tagNum = 0;

	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {

				if ( pixelEdges[t].ptr<uchar>( y )[x] == 255 ) continue;
				pixelNum = txy2pos( t, x, y, frameCount, frameSize );
				if ( pixelTag[pixelNum] != -1 ) continue;

				pixelTag[pixelNum] = tagNum++;
				que.push( pixelNum );
				elePerTagCount = 0;
				int seedPixelNum = pixelNum;

				while ( !que.empty() ) {

					nowPixelNum = que.front();
					que.pop();
					elePerTagCount++;
					pos2txy( nowPixelNum, nowT, nowX, nowY, N, frameCount, frameSize );

					for ( int k = 0; k < 6; k++ ) {

						nextT = nowT + route[k][0];
						nextX = nowX + route[k][1];
						nextY = nowY + route[k][2];

						nextPixelNum = txy2pos( nextT, nextX, nextY, frameCount, frameSize );

						if ( nextPixelNum == -1 ) continue;
						if ( pixelTag[nextPixelNum] != -1 ) continue;
						if ( pixelEdges[nextT].ptr<uchar>( nextY )[nextX] == 255 ) continue;
						
						if ( k > 1 && colorDiff( pixelColor[nowPixelNum], pixelColor[nextPixelNum] ) > colorDiffThred ) continue;
						if ( k < 2 && colorDiff( pixelColor[nowPixelNum], pixelColor[nextPixelNum] ) > 1 ) continue;
							//if ( colorDiff( pixelColor[nowPixelNum], pixelColor[nextPixelNum] ) > colorGradualThred ) continue;

						pixelTag[nextPixelNum] = pixelTag[nowPixelNum];
						que.push( nextPixelNum );

					}
				}

				if ( elePerTagCount < elePerTagThred ) tagAvailable[tagNum - 1] = false;
			}
		}
	}

	int tagNumTemp = 0;
	for ( int i = 0; i < tagNum; i++ ) {
		if ( tagAvailable[i] ) tagReplace[i] = tagNumTemp++;
	}

	tagNum = tagNumTemp;

	for ( int i = 1; i < N-1; i++ ) {
		if ( pixelTag[i] != -1 ) pixelTag[i] = tagReplace[pixelTag[i]];

	}

	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {

				nowPixelNum = txy2pos( t, x, y, frameCount, frameSize );
				if ( pixelTag[nowPixelNum] == -1 ) continue;

				for ( int k = 2; k < 6; k++ ) {

					nextX = x + route[k][1];
					nextY = y + route[k][2];

					nextPixelNum = txy2pos( t, nextX, nextY, frameCount, frameSize );

					if ( nextPixelNum == -1 ) continue;
					if ( pixelTag[nextPixelNum] == -1 ) {
						pixelTag[nextPixelNum] = pixelTag[nowPixelNum];
						que.push( nextPixelNum );
					}
				}
			}
		}
	}
	
	while ( !que.empty() ) {

		nowPixelNum = que.front();
		que.pop();
		pos2txy( nowPixelNum, nowT, nowX, nowY, N, frameCount, frameSize );

		for ( int k = 2; k < 6; k++ ) {

			nextT = nowT + route[k][0];
			nextX = nowX + route[k][1];
			nextY = nowY + route[k][2];

			nextPixelNum = txy2pos( nextT, nextX, nextY, frameCount, frameSize );

			if ( nextPixelNum == -1 ) continue;
			if ( pixelTag[nextPixelNum] != -1 ) continue;

			pixelTag[nextPixelNum] = pixelTag[nowPixelNum];
			que.push( nextPixelNum );
		}
	}
	
	delete &que;

	// show tag region
	//char pixelTagName[100], pixelEdgeName[100];
	srand( clock() );
	Mat pixelTagOut = Mat::zeros( frameSize, CV_8UC3 );
	vector<Vec3b> color;
	for ( int i = 0; i < tagNum; i++ ) {

		uchar t0 = rand() * 255;
		uchar t1 = rand() * 255;
		uchar t2 = rand() * 255;
		color.push_back( Vec3b( t0, t1, t2 ) );
	}

	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {
				int pixelNum = txy2pos( t, x, y, frameCount, frameSize );
				if ( pixelTag[pixelNum] == -1 ) continue;
				pixelTagOut.ptr<Vec3b>( y )[x] = color[pixelTag[pixelNum]];
			}
		}
		//imshow( "tag", output );
		//imshow( "origin", frames[t] );
		sprintf( filename, "pixelTag//%d.png", t );
		imwrite( filename, pixelTagOut );
		//waitKey( 0 );
	}
}

#endif