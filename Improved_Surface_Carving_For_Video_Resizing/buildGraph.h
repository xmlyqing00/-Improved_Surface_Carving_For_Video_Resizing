﻿/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : buildGraph.h
*	Creation Time : 2015-3-19
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef BUILD_GRAPH
#define BUILD_GRAPH

#include "baseFunction.h"

void buildGraph( vector<Mat> &frames, vector<Mat> &pixelEnergy, vector<Mat> &edgeProtect, 
				 int bandLeft, int bandWidth, vector<int> &num2pos, vector<int> &edgeHead, vector<typeEdge> &edge ) {

	int wEdgeProtect = 2;
	int frameCount = frames.size();
	Size frameSize = frames[0].size();

	int N = frameCount * frameSize.width * frameSize.height + 2;
	int M = 1;
	bandLeft = min( bandLeft, frameSize.width - bandWidth );
	int bandRight = bandLeft + bandWidth;

	num2pos.push_back( 0 );
	vector<int> pos2num( N, -1 );
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = bandLeft; x < bandRight; x++ ) {

				int p0 = txy2pos( t, x, y, frameCount, frameSize );
				pos2num[p0] = M++;
				num2pos.push_back( p0 );
			}
		}
	}

	N = M + 1;
	int S = 0, T = N - 1;

	edgeHead.clear();
	edge.clear();
	edgeHead = vector<int>( N, -1 );
	//cout << "  > NodeSize : " << N << endl;

	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {

			int p = pos2num[txy2pos( t, bandLeft, y, frameCount, frameSize )];
			buildEdge( edgeHead, edge, S, p, INF );
			buildEdge( edgeHead, edge, p, S, 0 );

			p = pos2num[txy2pos( t, bandRight - 1, y, frameCount, frameSize )];
			int w = pixelEnergy[t].ptr<uchar>( y )[bandRight - 1] + edgeProtect[t].ptr<uchar>( y )[bandRight - 1];
			buildEdge( edgeHead, edge, p, T, w, true );
			buildEdge( edgeHead, edge, T, p, 0 );
		}
	}
	//cout << "After S & T edge.size() = " << edge.size() << endl;

	
	// Pixel Energy
	for ( int t = 0; t < frameCount; t++ ) {

		for ( int y = 0; y < frameSize.height; y++ ) {

			uchar *rowDataEnergy = pixelEnergy[t].ptr<uchar>( y );
			uchar *rowDataEdgeProtect = edgeProtect[t].ptr<uchar>( y );
			for ( int x = bandLeft; x < bandRight - 1; x++ ) {

				int p0 = pos2num[txy2pos( t, x, y, frameCount, frameSize )];
				int p1 = pos2num[txy2pos( t, x + 1, y, frameCount, frameSize )];

				int w = rowDataEnergy[x] + rowDataEdgeProtect[x] * wEdgeProtect;
				buildEdge( edgeHead, edge, p0, p1, w, true );
				buildEdge( edgeHead, edge, p1, p0, INF );
			}
		}
	}

	// XY-Plane
	for ( int t = 0; t < frameCount; t++ ) {
		
		for ( int y = 1; y < frameSize.height; y++ ) {

			for ( int x = bandLeft; x < bandRight; x++ ) {

				int pAbove = pos2num[txy2pos( t, x, y, frameCount, frameSize )];
				int pBelow = pos2num[txy2pos( t, x, y - 1, frameCount, frameSize )];
				
				int wDown = edgeProtect[t].ptr<uchar>( y - 1 )[x] * wEdgeProtect;
				if ( x > 0 ) {
					wDown += abs( min( 0, pixelEnergy[t].ptr<uchar>( y - 1 )[x] - pixelEnergy[t].ptr<uchar>( y - 1 )[x - 1] ) );
				}
				
				int wUp = edgeProtect[t].ptr<uchar>( y )[x] * wEdgeProtect;
				if ( x > 0 ) {
					wUp += abs( min( 0, pixelEnergy[t].ptr<uchar>( y )[x - 1] - pixelEnergy[t].ptr<uchar>( y )[x] ) );
				}

				if ( wDown > 0 || wUp > 0 ) {
					buildEdge( edgeHead, edge, pAbove, pBelow, wDown );
					buildEdge( edgeHead, edge, pBelow, pAbove, wUp );
				}
			}
		}
	}
	
	
	// XT-Plane
	for ( int y = 0; y < frameSize.height; y++ ) {

		for ( int t = 1; t < frameCount; t++ ) {
			
			for ( int x = bandLeft; x < bandRight; x++ ) {

				int pFront = pos2num[txy2pos( t, x, y, frameCount, frameSize )];
				int pRear = pos2num[txy2pos( t - 1, x, y, frameCount, frameSize )];
				
				int wBack = edgeProtect[t - 1].ptr<uchar>( y )[x] * wEdgeProtect;
				if ( x > 0 ) {
					wBack += abs( min( 0, pixelEnergy[t - 1].ptr<uchar>( y )[x] - pixelEnergy[t - 1].ptr<uchar>( y )[x - 1] ) );
				}

				int wFor = edgeProtect[t].ptr<uchar>( y )[x] * wEdgeProtect;
				if ( x > 0 ) {
					wFor += abs( min( 0, pixelEnergy[t].ptr<uchar>( y )[x - 1] - pixelEnergy[t].ptr<uchar>( y )[x] ) );
				}

				if ( wBack > 0 || wFor > 0 ) {
					buildEdge( edgeHead, edge, pFront, pRear, wBack );
					buildEdge( edgeHead, edge, pRear, pFront, wFor );
				}
			}
		}
	}
	
	//cout << "  > edgeSize : " << edge.size() << endl;

#ifdef SHOW_GRAPH
	if ( bandWidth == frameSize.width ) {
		for ( int i = 0; i < num2pos.size(); i++ ) {
			if ( num2pos[i] != i ) cout << "6666  " << i << endl;
		}
		showGraph( edgeHead, edge, N, frameCount, frameSize );
	}
#endif
}

void settleBand( vector< vector<int> > &removePts, int &bandLeft, int bandWidth, Size frameSize, int frameCount ) {

	int avgX = 0;
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			avgX += removePts[t][y];
		}
	}
	avgX = (avgX << 1) / (frameCount * frameSize.height);
	
	bandLeft = max( 0, avgX - bandWidth / 2 );
	//bandLeft = min( bandLeft, frameSize.width * 2 + 1 - bandWidth );

	/*
	Mat bandMat;
	char matName[100];
	for ( int t = 0; t < frameCount; t++ ) {

		sprintf( matName, "midBandMap//%d.png", t );
		bandMat = Mat::zeros( frameSize, CV_8UC1 );
		for ( int y = 0; y < frameSize.height; y++ ) {
			bandMat.ptr<uchar>( y )[removePts[t][y]] = 255;
		}
		imwrite( matName, bandMat );
	}
	cout << "avgX : " << avgX << endl;
	//system( "pause" );
	*/
}

#endif