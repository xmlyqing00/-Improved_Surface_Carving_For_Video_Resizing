/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : main.cpp
*	Creation Time : 2015-3-19
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#include "baseFunction.h"
#include "io.h"
#include "buildGraph.h"
#include "surfaceCarving.h"
#include "pixelTag.h"
#include "pixelEnergy.h"
#include "compareWork.h"
#include "buildPyramid.h"
#include "resizeVideo.h"
#include "timeCompress.h"
#include "shotCut.h"

int main( void ) {

	help();
	
	// func type
	int funcType = 1;
	int readType = 1;
	int processId = 5;

	// paramater
	int layerLimit = 3;
	int widthDeleted = 100;
	int colorDiffThred = 5;
	int elePerTagThred = 300;
	int bandWidthDefault = 50;
	int keyFrameNumLimit = 50;
	int badCutLimit = (int)(0.1 * widthDeleted);

	switch ( funcType ) {

		case 0:{
			
			bool state;
			state = video2Frames( processId );
			if ( !state ) return -2;
			break;
		}
		case 1:{

			int globalTime = clock();

			// shot Cut
			cout << " Shot Cut --ING" << endl;
			vector<int> shotArr;
			segShotCut( processId, shotArr );
			
			// resize shot
			for ( int shotId = 1; shotId < (int)shotArr.size(); shotId++ ) {

				vector<Mat> frames;
				bool state;
				state = readFrameStream( processId, shotArr[shotId - 1], shotArr[shotId], frames );
				if ( !state ) return -2;

				// calculate Energy
				vector<Mat> pixelEnergy;
				vector<Mat> edgeProtect;
				vector<int> pixelTag;
				int tagNum;

				cout << " Calculate Pixel Tag --ING" << endl;
				calcPixelTag( frames, pixelTag, tagNum, colorDiffThred, elePerTagThred );
				cout << " Calculate Pixel Energy --ING" << endl;
				calcPixelEnergy( frames, pixelTag, tagNum, pixelEnergy );
				pixelTag.clear();

				// temporal compress
				cout << " Temporal Compress --ING" << endl;
				vector<int> keyFrame;
				timeCompress( pixelEnergy, keyFrame, keyFrameNumLimit );
				preserveKeyData( keyFrame, frames, pixelEnergy );

				// resize Video
				calcEdgeProtect( frames, edgeProtect );
				resizeVideo( keyFrame, frames, pixelEnergy, edgeProtect, layerLimit, widthDeleted, bandWidthDefault, badCutLimit, 0 );
				scaleVideo( keyFrame, frames, pixelEnergy, widthDeleted );
				rotateVideo( keyFrame, frames, pixelEnergy, 0 );
				calcEdgeProtect( frames, edgeProtect );
				resizeVideo( keyFrame, frames, pixelEnergy, edgeProtect, layerLimit, widthDeleted, bandWidthDefault, badCutLimit, 1 );
				rotateVideo( keyFrame, frames, pixelEnergy, 1 );
			}
			// finished
			cout << " Well done !!" << endl;
			globalTime = clock() - globalTime;
			globalTime = (globalTime + 500) / 1000;
			printf( " Global Time Used : %d min %d sec\n", globalTime / 60, globalTime % 60 );

			break;
		}
		case 2:{
			// write video
			bool state = writeVideo( processId );
			if ( !state ) return -2;
			break;
		}
		default:
			break;
	}

	system( "pause" );

	return 0;
}


