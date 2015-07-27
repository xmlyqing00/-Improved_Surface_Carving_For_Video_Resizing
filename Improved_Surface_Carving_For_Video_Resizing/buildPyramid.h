/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : buildPyramid.h
*	Creation Time : 2015-5-28
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef BUILD_PYRAMID
#define BUILD_PYRAMID

#include "baseFunction.h"

void buildPyramid( vector< vector<Mat> > &pyramidFrames, vector< vector<Mat> > &pyramidPixelEnergy, vector< vector<Mat> > &pyramidEdgeProtect,
				   vector<Mat> &frames, vector<Mat> &pixelEnergy, vector<Mat> &edgeProtect, int layerLimit, bool isBuildPyramid ) {

	pyramidFrames.clear();
	pyramidPixelEnergy.clear();
	pyramidEdgeProtect.clear();

	for ( int pyramidIndex = 0; pyramidIndex < layerLimit; pyramidIndex++ ) {
	
		bool temporalRescale = false;

		//if ( (pyramidIndex + 1) % 2 == 0 ) temporalRescale = true;

		if ( pyramidIndex != 0 ) {

			int countTemp = 0;
			int step = temporalRescale ? 2 : 1;

			for ( int i = 0; i < (int)frames.size(); i += step ) {

				resize( frames[i], frames[i], Size(), 0.5, 0.5 );
				resize( pixelEnergy[i], pixelEnergy[i], Size(), 0.5, 0.5 );
				resize( edgeProtect[i], edgeProtect[i], Size(), 0.5, 0.5 );

				if ( temporalRescale ) {
					frames[i / 2] = frames[i];
					pixelEnergy[i / 2] = pixelEnergy[i];
					edgeProtect[i / 2] = edgeProtect[i];
					countTemp++;
				}
			}
			if ( temporalRescale ) {
				frames.resize( countTemp );
				pixelEnergy.resize( countTemp );
				edgeProtect.resize( countTemp );
			}
		}
		
		//cout << frames.size() << " " << frames[0].size().height << " " << frames[0].size().width << endl;
		pyramidFrames.push_back( frames );
		pyramidPixelEnergy.push_back( pixelEnergy );
		pyramidEdgeProtect.push_back( edgeProtect );

		if ( !isBuildPyramid ) break;
	}

	frames = pyramidFrames[0];
	pixelEnergy = pyramidPixelEnergy[0];
	edgeProtect = pyramidEdgeProtect[0];
}
#endif