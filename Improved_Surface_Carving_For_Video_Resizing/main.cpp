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
	int funcType = 0;
	int processId = 0;

	// paramater
	int layerLimit = 3;
	int widthDeletedDefault = 70;
	int colorDiffThred = 5;
	int elePerTagThred = 300;
	int bandWidthDefault = 50;
	int keyFrameNumLimit = 30;
	int badCutLimit = (int)(0.08 * widthDeletedDefault);
	vector<int> shotArr;

	switch ( funcType ) {

		case 0:{

			bool state;
			state = video2Frames( processId );
			if ( !state ) return -2;

			// shot Cut
			cout << " Shot Cut --ING" << endl;
			segShotCut( shotArr );

			cout << " Convert Process Finished !! " << endl;
		}
		case 1:{

			int globalTime = clock();

			getShotCut( shotArr );

			// resize shot
			for ( int shotId = 87; shotId < (int)shotArr.size(); shotId++ ) {

				int frameStId = shotArr[shotId - 1];
				int frameEdId = shotArr[shotId];

				cout << " Process Shot From " << frameStId << " To " << frameEdId << " --ING" << endl << endl;

				// read frames
				vector<Mat> frames;
				bool state;
				state = readFrameStream( frameStId, frameEdId, frames );
				if ( !state ) return -2;

				// calculate Energy
				vector<Mat> pixelEnergy;
				vector<int> pixelTag;
				int tagNum;
				calcPixelTag( frames, pixelTag, tagNum, colorDiffThred, elePerTagThred, frameStId );
				calcPixelEnergy( frames, pixelTag, tagNum, pixelEnergy, frameStId );

				// temporal compress
				vector<int> keyFrame;
				timeCompress( pixelEnergy, keyFrame, keyFrameNumLimit );
				preserveKeyData( keyFrame, frames, pixelEnergy );

				// resize Video
				int widthDeleted = widthDeletedDefault;
				vector<Mat> edgeProtect;
				for ( int c = 0; c < 2; c++ ) {

					if ( widthDeleted > 0 ) {
						calcEdgeProtect( frames, edgeProtect );
						resizeVideo( keyFrame, frames, pixelEnergy, edgeProtect, layerLimit, widthDeleted, bandWidthDefault, badCutLimit, frameStId, frameEdId, c );
						scaleVideo( keyFrame, frames, pixelEnergy, widthDeleted, frameStId, frameEdId, c );
						rotateVideo( keyFrame, frames, pixelEnergy, frameStId, frameEdId, c );
					} else {
						copyFrame( frameStId, frameEdId );
					}
				}

				cout << endl;
				break;

			}
			// finished
			cout << " Shot Finished !! Well done !!" << endl;
			globalTime = clock() - globalTime;
			globalTime = (globalTime + 500) / 1000;
			printf( " Global Time Used : %d min %d sec\n", globalTime / 60, globalTime % 60 );

			break;
		}
		case 2:{

			   // write frame stream
			   writeFrameStream();
		}
		case 3:{
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


