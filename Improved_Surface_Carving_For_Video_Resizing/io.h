/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : io.h
*	Creation Time : 2015-5-29
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef IO
#define IO

#include "baseFunction.h"

bool video2Frames( int processId ) {
	
	cout << " Convert Video To Frames --ING" << endl;

	VideoCapture cap;
	char videoName[100];
	strcpy( videoName, "test" );
	videoName[4] = char( processId + 48 );
	videoName[5] = '\0';
	strcat( videoName, "In.mkv" );

	if ( !cap.open( videoName ) ) {
		cout << "!!! Could not open the output video for reading!" << endl;
		return false;
	}

	Mat inputFrame;
	int count = 0;
	char matName[100];

	while ( cap.read( inputFrame ) ) {

		resize( inputFrame, inputFrame, Size( 384, 216 ) );

		sprintf( matName, "frameStream//%d.png", count );
		imwrite( matName, inputFrame );

		count++;
	}

	return true;
	
}
bool readVideo( int processId, vector<Mat> &frames ) {

	VideoCapture cap;
	char videoName[100];
	strcpy( videoName, "test" );
	videoName[4] = char( processId + 48 );
	videoName[5] = '\0';
	strcat( videoName, "In.mkv" );

	if ( !cap.open( videoName ) ) {
		cout << "!!! Could not open the output video for reading!" << endl;
		return false;
	}

	Mat inputFrame;
	int count = 0;
	int countTemp = 0;
	char matName[100];

	while ( cap.read( inputFrame ) ) {

		countTemp++;
		//if ( countTemp < 30 ) continue;
		//if ( countTemp % 2 != 0 ) continue;
		resize( inputFrame, inputFrame, Size( 384, 216 ) );

		sprintf( matName, "input//%d.png", count );
		imwrite( matName, inputFrame );

		//medianBlur( inputFrame, inputFrame, 3 );
		//frames.push_back( inputFrame.clone() );

		count++;

		//if ( count == 200 ) break;
	}

	/*
	union {
	int value;
	char code[4];
	} getFourCC;

	getFourCC.value = static_cast<int>(cap.get( CV_CAP_PROP_FOURCC ));
	for ( int i = 0; i < 4; i++ ) cout << getFourCC.code[i] << endl;
	*/

	return true;
}

bool readFrameStream( int streamIndex, int streamEnd, vector<Mat> &frames ) {

	cout << " Read Frames --ING" << endl;

	char matName[100];
	Mat inputFrame;

	for ( int i = streamIndex; i < streamEnd; i++ ) {

		sprintf( matName, "frameStream//%d.png", i );
		inputFrame = imread( matName );
		if ( inputFrame.empty() ) return false;
		medianBlur( inputFrame, inputFrame, 3 );
		frames.push_back( inputFrame.clone() );
	}
	return true;
}

void saveFrame( vector<int> &keyFrame, int funcType, int len, vector<Mat> &frames, 
				vector<vector<int>> &linkHead, vector<typeLink> &link, int frameStId, int frameEdId ) {
	
	int n = keyFrame.size();
	int t = 0;
	char pngName[100];

	switch ( funcType ) {

		case 0:
			
			for ( int i = frameStId; i < frameEdId; i++ ) {
			
				sprintf( pngName, "frameStream//%d.png", i );
				Mat originFrame = imread( pngName );
				
				if ( t < n && (i-frameStId) == keyFrame[t + 1] ) t++;
				Size frameSize = originFrame.size();
				Mat surfaceFrame = originFrame.clone();
				Mat resultFrame = Mat( frameSize.height, frameSize.width - len, CV_8UC3 );

				for ( int y = 0; y < frameSize.height; y++ ) {

					int newX = 0;
					int p = linkHead[t][y];
					for ( int x = 0; x < frameSize.width; x++ ) {

						if ( p != -1 && x == link[p].y ) {
							surfaceFrame.ptr<Vec3b>( y )[x] = Vec3b( 0, 0, 255 );
							p = link[p].next;
							continue;
						} else {
							resultFrame.ptr<Vec3b>( y )[newX] = originFrame.ptr<Vec3b>( y )[x];
							newX++;
						}

					}
				}
				
				if ( (i-frameStId) == keyFrame[t] ) frames[t] = resultFrame.clone();
				
				sprintf( pngName, "shrinkCut//%d.png", i );
				imwrite( pngName, surfaceFrame );

				sprintf( pngName, "shrinkResult//%d.png", i );
				imwrite( pngName, resultFrame );

			}

			break;

		case 1:

			for ( int i = frameStId; i < frameEdId; i++ ) {

				sprintf( pngName, "tempMat//%d.png", i );
				Mat originFrame = imread( pngName );

				if ( t < n && (i - frameStId) == keyFrame[t + 1] ) t++;
				Size frameSize = originFrame.size();
				Mat surfaceFrame = originFrame.clone();
				Mat resultFrame = Mat( frameSize.height, frameSize.width + len * 2, CV_8UC3 );

				for ( int y = 0; y < frameSize.height; y++ ) {

					int newX = 0;
					int p = linkHead[t][y];
					for ( int x = 0; x < frameSize.width; x++ ) {

						if ( x == link[p].y ) {

							int x1 = (x == 0) ? x : x - 1;
							int x2 = (x == frameSize.width - 1) ? x : x + 1;
							Vec3b p0 = originFrame.ptr<Vec3b>( y )[x1];
							Vec3b p1 = originFrame.ptr<Vec3b>( y )[x];
							resultFrame.ptr<Vec3b>( y )[newX++] = colorMid( p0, p1 );

							resultFrame.ptr<Vec3b>( y )[newX++] = p1;

							p0 = originFrame.ptr<Vec3b>( y )[x];
							p1 = originFrame.ptr<Vec3b>( y )[x2];
							resultFrame.ptr<Vec3b>( y )[newX++] = colorMid( p0, p1 );

							surfaceFrame.ptr<Vec3b>( y )[x] = Vec3b( 0, 0, 255 );
							p = link[p].next;
							continue; 
						} else {
							
							resultFrame.ptr<Vec3b>( y )[newX++] = originFrame.ptr<Vec3b>( y )[x];
						}

					}
				}

				if ( (i - frameStId) ) frames[t] = resultFrame.clone( );

				frameSize = surfaceFrame.size();
				Mat rotateFrame = Mat( frameSize.width, frameSize.height, CV_8UC3 );

				for ( int y = 0; y < frameSize.height; y++ ) {
					for ( int x = 0; x < frameSize.width; x++ ) {
						rotateFrame.ptr<Vec3b>( x )[y] = surfaceFrame.ptr<Vec3b>( y )[x];
					}
				}
				sprintf( pngName, "extendCut//%d.png", i );
				imwrite( pngName, rotateFrame );

				sprintf( pngName, "extendResult//%d.png", i );
				imwrite( pngName, resultFrame );
				
			}

			break;

		default:
			break;
	}
}

bool writeVideo( int processId ) {

	char videoName[100] = "test";
	videoName[4] = char( processId + 48 );
	videoName[5] = '\0';
	strcat( videoName, "Out.avi" );

	char pngName[100];
	Mat inputMat, tagMat, energyMat, shrinkMat, extendMat, outputMat;
	Vec3b *rowDataMat0, *rowDataMat1;
	char *windowName = "combineVideo";
	namedWindow( windowName );
	int seamWidth = 5;

	sprintf( pngName, "frameStream//%d.png", 0 );
	inputMat = imread( pngName );
	if ( inputMat.empty() ) return false;

	Size singleSize = inputMat.size();
	Size combineSize;
	combineSize.width = singleSize.width * 3 + seamWidth * 4;
	combineSize.height = singleSize.height * 2 + seamWidth * 3;

	VideoWriter outputVideo;
	outputVideo.open( videoName, CV_FOURCC( 'M', 'J', 'P', 'G' ), 25, combineSize );

	if ( !outputVideo.isOpened() ) {

		cout << "!!! Could not open the output video for writing!" << endl;
		return false;
	}

	for ( int i = 0; i < INF; i++ ) {

		sprintf( pngName, "pixelTag//%d.png", i );
		tagMat = imread( pngName );
		if ( tagMat.empty() ) break;

		sprintf( pngName, "pixelEnergy//%d.png", i );
		energyMat = imread( pngName );
		if ( energyMat.empty() ) break;

		sprintf( pngName, "shrinkCut//%d.png", i );
		shrinkMat = imread( pngName );
		if ( shrinkMat.empty() ) break;
		
		sprintf( pngName, "extendCut//%d.png", i );
		extendMat = imread( pngName );
		if ( extendMat.empty() ) break;

		sprintf( pngName, "output//%d.png", i );
		outputMat = imread( pngName );
		if ( outputMat.empty() ) break;
		
		Mat combineMat = Mat::zeros( combineSize, CV_8UC3 );
		Vec3b borderColor = Vec3b( 255, 255, 255 );
		for ( int y = 4; y < combineSize.height - 4; y++ ) {
			combineMat.ptr<Vec3b>( y )[seamWidth - 1] = borderColor;
			combineMat.ptr<Vec3b>( y )[seamWidth + singleSize.width] = borderColor;
			combineMat.ptr<Vec3b>( y )[seamWidth * 2 - 1 + singleSize.width] = borderColor;
			combineMat.ptr<Vec3b>( y )[seamWidth * 2 + singleSize.width * 2] = borderColor;
			combineMat.ptr<Vec3b>( y )[seamWidth * 3 - 1 + singleSize.width * 2] = borderColor;
			combineMat.ptr<Vec3b>( y )[seamWidth * 3 + singleSize.width * 3] = borderColor;
		}
		for ( int x = 4; x < combineSize.width - 4; x++ ) {
			combineMat.ptr<Vec3b>( seamWidth - 1 )[x] = borderColor;
			combineMat.ptr<Vec3b>( seamWidth + singleSize.height )[x] = borderColor;
			combineMat.ptr<Vec3b>( seamWidth * 2 - 1 + singleSize.height )[x] = borderColor;
			combineMat.ptr<Vec3b>( seamWidth * 2 + singleSize.height * 2 )[x] = borderColor;
		}

		
		for ( int y = 0; y < singleSize.height; y++ ) {

			rowDataMat0 = combineMat.ptr<Vec3b>( y + seamWidth );

			rowDataMat1 = inputMat.ptr<Vec3b>( y );
			for ( int x = 0; x < singleSize.width; x++ ) rowDataMat0[x + seamWidth] = rowDataMat1[x];
			
			rowDataMat1 = tagMat.ptr<Vec3b>( y );
			for ( int x = 0; x < singleSize.width; x++ ) rowDataMat0[x + singleSize.width + seamWidth * 2] = rowDataMat1[x];

			rowDataMat1 = energyMat.ptr<Vec3b>( y );
			for ( int x = 0; x < singleSize.width; x++ ) rowDataMat0[x + singleSize.width * 2 + seamWidth * 3] = rowDataMat1[x];
		}

		for ( int y = 0; y < outputMat.rows; y++ ) {

			rowDataMat0 = combineMat.ptr<Vec3b>( y + singleSize.height + seamWidth * 2 );
			rowDataMat1 = outputMat.ptr<Vec3b>( y );
			for ( int x = 0; x < outputMat.cols; x++ ) rowDataMat0[x + seamWidth] = rowDataMat1[x];
		}

		for ( int y = 0; y < shrinkMat.rows; y++ ) {

			rowDataMat0 = combineMat.ptr<Vec3b>( y + singleSize.height + seamWidth * 2 );
			rowDataMat1 = shrinkMat.ptr<Vec3b>( y );
			for ( int x = 0; x < shrinkMat.cols; x++ ) rowDataMat0[x + singleSize.width + seamWidth * 2] = rowDataMat1[x];
		}

		for ( int y = 0; y < extendMat.rows; y++ ) {

			rowDataMat0 = combineMat.ptr<Vec3b>( y + singleSize.height + seamWidth * 2 );
			rowDataMat1 = extendMat.ptr<Vec3b>( y );
			for ( int x = 0; x < extendMat.cols; x++ ) rowDataMat0[x + singleSize.width * 2 + seamWidth * 3] = rowDataMat1[x];
		}


		sprintf( pngName, "combine//%d.png", i );
		imwrite( pngName, combineMat );
		outputVideo << combineMat;
		imshow( windowName, combineMat );
		waitKey( 30 );

		sprintf( pngName, "frameStream//%d.png", i );
		inputMat = imread( pngName );
		if ( inputMat.empty() ) break;
	}

	destroyAllWindows();

	return true;
}

#endif