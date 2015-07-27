/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : pixelEnergy.h
*	Creation Time : 2015-5-25
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef PIXEL_ENERGY
#define PIXEL_ENERGY

#include "baseFunction.h"

inline void cartesian2polar( Point2i p0, Point2i p1, double &rho, double &theta ) {

	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	rho = sqrt( sqr( dx ) + sqr( dy ) );
	theta = atan2( dy, dx );
}
void pickSeed( int event, int x, int y, int, void *param ) {

	pair<int, Point> *seed = (pair<int, Point>*)param;

	if ( seed->first == -1 ) return;

	switch ( event ) {
		case CV_EVENT_LBUTTONDOWN:
			seed->first = 1;
			break;
		case CV_EVENT_MBUTTONDOWN:
			seed->first = 2;
			break;
		case CV_EVENT_RBUTTONDOWN:
			seed->first = 3;
			break;
		case CV_EVENT_MBUTTONUP:
		case CV_EVENT_LBUTTONUP:
		case CV_EVENT_RBUTTONUP:
			seed->first = 0;
			break;
		default:
			break;
	}

	seed->second.x = x;
	seed->second.y = y;
}
void drawBgMat( Mat &img, int t, vector<int> &pixelTag, vector<int> &tagLayer, Mat &frame, Size frameSize, int frameCount ) {

	img = Mat::zeros( frameSize, CV_8UC3 );
	for ( int y = 0; y < frameSize.height; y++ ) {
		Vec3b *rowData = img.ptr<Vec3b>( y );
		Vec3b *frameData = frame.ptr<Vec3b>( y );
		for ( int x = 0; x < frameSize.width; x++ ) {

			int pixelNum = txy2pos( t, x, y, frameCount, frameSize );
			int nowTag = pixelTag[pixelNum];
			rowData[x] = frameData[x];
			switch ( tagLayer[nowTag] ) {
				case 0:
					rowData[x].val[0] = 220;
					rowData[x].val[1] = min( rowData[x].val[1], (uchar)100 );
					rowData[x].val[2] = min( rowData[x].val[1], (uchar)100 );
					break;
				case 1:
					break;
				case 2:
					rowData[x].val[1] = 220;
					rowData[x].val[0] = min( rowData[x].val[1], (uchar)100 );
					rowData[x].val[2] = min( rowData[x].val[1], (uchar)100 );
					break;
				default:
					break;
			}
		}
	}
}
void userInteraction( vector<int> &pixelTag, vector<Mat> &frames, vector<int> &tagLayer, 
					  int frameCount, Size frameSize, int frameStId ) {

	char *selectWindowName = "Select Window";
	char *tagWindowName = "Tag Window";

	char pngName[100];
	pair<int, Point> seed = make_pair( 0, Point( -1, -1 ) );
	Mat tagMat, bgMat;
	bool readySelect = false;

	namedWindow( tagWindowName );
	namedWindow( selectWindowName );

	setMouseCallback( selectWindowName, pickSeed, (void*)(&seed) );

	for ( int i = 0; i < frameCount; i++ ) {

		sprintf( pngName, "pixelTag//%d.png", i + frameStId );
		tagMat = imread( pngName );

		drawBgMat( bgMat, i, pixelTag, tagLayer, frames[i], frameSize, frameCount );

		imshow( tagWindowName, tagMat );
		imshow( selectWindowName, bgMat );

		if ( !readySelect ) {
			waitKey( 0 );
			readySelect = true;
		} else {
			if ( waitKey( 33 ) != 32 && i != frameCount - 1 ) continue;
		}

		while ( waitKey( 1 ) != 32 ) {

			if ( seed.first == 0 ) continue;

			int seedStatus = seed.first;
			seed.first = -1;

			int pixelNum = txy2pos( i, seed.second.x, seed.second.y, frameCount, frameSize );
			if ( pixelNum == -1 ) {
				seed.first = seedStatus;
				continue;
			}
			int seedTag = pixelTag[pixelNum];
			bool reDraw = false;
			int newLayer;
			switch ( seedStatus ) {

				case 1:
					newLayer = 0;
					if ( tagLayer[seedTag] != newLayer ) {
						reDraw = true;
						tagLayer[seedTag] = newLayer;
					}
					break;
				case 2:
					newLayer = 1;
					if ( tagLayer[seedTag] != newLayer ) {
						reDraw = true;
						tagLayer[seedTag] = newLayer;
					}
					break;
				case 3:
					newLayer = 2;
					if ( tagLayer[seedTag] != newLayer ) {
						reDraw = true;
						tagLayer[seedTag] = newLayer;
					}
					break;

				default:
					break;
			}

			if ( reDraw ) {
				drawBgMat( bgMat, i, pixelTag, tagLayer, frames[i], frameSize, frameCount );
				imshow( selectWindowName, bgMat );
			}

			seed.first = seedStatus;

		}
	}

	destroyWindow( selectWindowName );
	destroyWindow( tagWindowName );
}

void calcPixelEnergy( vector<Mat> &frames, vector<int> &pixelTag, int tagNum, vector<Mat> &pixelEnergy, int frameStId ) {

	cout << " Calculate Pixel Energy --ING" << endl;

	int frameCount = frames.size();
	Size frameSize = frames[0].size();
	const int layerLimit = 3;

	vector< vector<Point2i> > tagCenterPerFrame( tagNum, vector<Point2i>( frameCount, Point2i( 0, 0 ) ) );
	vector< vector<int> > tagCountPerFrame( tagNum, vector<int>( frameCount, 0 ) );
	vector< vector<double> > tagRhoPerFrame( tagNum, vector<double>( frameCount, 0 ) );
	vector< vector<double> > tagThetaPerFrame( tagNum, vector<double>( frameCount, 0 ) );
	int pixelNum;

	pixelNum = 1;
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {

				int nowTag = pixelTag[pixelNum++];

				tagCenterPerFrame[nowTag][t] += Point2i( x, y );
				tagCountPerFrame[nowTag][t]++;

			}
		}
	}

	for ( int i = 0; i < tagNum; i++ ) {
		for ( int t = 0; t < frameCount; t++ ) {

			if ( tagCountPerFrame[i][t] > 0 ) {
				tagCenterPerFrame[i][t].x = cvRound( (double)tagCenterPerFrame[i][t].x / tagCountPerFrame[i][t] );
				tagCenterPerFrame[i][t].y = cvRound( (double)tagCenterPerFrame[i][t].y / tagCountPerFrame[i][t] );
			}
		}
	}

	pixelNum = 1;
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {

				int nowTag = pixelTag[pixelNum++];
				double rho, theta;

				if ( tagCountPerFrame[nowTag][t] == 0 ) continue;
				cartesian2polar( tagCenterPerFrame[nowTag][t], Point2i( x, y ), rho, theta );
				tagRhoPerFrame[nowTag][t] += rho;
				tagThetaPerFrame[nowTag][t] += theta;
				//tagThetaPerFrame[nowTag][t] += rho * theta;
			}
		}
	}

	for ( int i = 0; i < tagNum; i++ ) {
		for ( int t = 0; t < frameCount; t++ ) {

			if ( tagCountPerFrame[i][t] == 0 ) continue;
			tagThetaPerFrame[i][t] /= tagCountPerFrame[i][t];
			//if ( tagRhoPerFrame[i][t] != 0 ) tagThetaPerFrame[i][t] /= tagRhoPerFrame[i][t];
			tagRhoPerFrame[i][t] /= tagCountPerFrame[i][t];
		}
	}

	Point2i tagCenterAvg;
	int tagCountAvg;
	double tagRhoAvg, tagThetaAvg;

	vector<double> tagCenterDeviation( tagNum, 0 );
	vector<double> tagCountDeviation( tagNum, 0 );
	vector<double> tagRhoDeviation( tagNum, 0 );
	vector<double> tagThetaDeviation( tagNum, 0 );
	vector<double> tagEnergy( tagNum, 0 );

	double maxTagCenterDeviation, maxTagCountDeviation, maxTagRhoDeviation, maxTagThetaDeviation;
	double minTagCenterDeviation, minTagCountDeviation, minTagRhoDeviation, minTagThetaDeviation;

	maxTagCenterDeviation = maxTagCountDeviation = maxTagRhoDeviation = maxTagThetaDeviation = -INF;
	minTagCenterDeviation = minTagCountDeviation = minTagRhoDeviation = minTagThetaDeviation = INF;

	for ( int i = 0; i < tagNum; i++ ) {

		tagCenterAvg = Point2i( 0, 0 );
		tagCountAvg = 0;
		tagRhoAvg = 0;
		tagThetaAvg = 0;

		int tagAvailableFrameCount = 0;
		for ( int t = 0; t < frameCount; t++ ) {

			if ( tagCountPerFrame[i][t] == 0 ) continue;
			tagCenterAvg += tagCenterPerFrame[i][t];
			tagCountAvg += tagCountPerFrame[i][t];
			tagRhoAvg += tagRhoPerFrame[i][t];
			tagThetaAvg += tagThetaPerFrame[i][t];
			tagAvailableFrameCount++;
		}

		tagCenterAvg.x = cvRound( (double)tagCenterAvg.x / tagAvailableFrameCount );
		tagCenterAvg.y = cvRound( (double)tagCenterAvg.y / tagAvailableFrameCount );
		tagCountAvg /= tagAvailableFrameCount;
		tagRhoAvg /= tagAvailableFrameCount;
		tagThetaAvg /= tagAvailableFrameCount;

		for ( int t = 0; t < frameCount; t++ ) {

			if ( tagCountPerFrame[i][t] == 0 ) continue;
			double rho, theta;
			cartesian2polar( tagCenterAvg, tagCenterPerFrame[i][t], rho, theta );
			tagCenterDeviation[i] += sqr( rho );
			tagCountDeviation[i] += sqr( tagCountAvg - tagCountPerFrame[i][t] );
			tagRhoDeviation[i] += sqr( tagRhoAvg - tagRhoPerFrame[i][t] );
			tagThetaDeviation[i] += sqr( tagThetaAvg - tagThetaPerFrame[i][t] );
		}
		if ( tagCountPerFrame[i][0] == 0 ) {
			tagCountDeviation[i] += sqr( tagCountAvg );
			tagRhoDeviation[i] += sqr( tagRhoAvg );
			tagAvailableFrameCount++;
		}
		if ( tagCountPerFrame[i][frameCount - 1] == 0 ) {
			tagCountDeviation[i] += sqr( tagCountAvg );
			tagRhoDeviation[i] += sqr( tagRhoAvg );
			tagAvailableFrameCount++;
		}

		tagCenterDeviation[i] = sqrt( tagCenterDeviation[i] / tagAvailableFrameCount );
		//tagCenterDeviation[i] /= tagRhoAvg;
		tagCountDeviation[i] = sqrt( tagCountDeviation[i] / tagAvailableFrameCount );
		//tagCountDeviation[i] /= tagRhoAvg;
		tagRhoDeviation[i] = sqrt( tagRhoDeviation[i] / tagAvailableFrameCount );
		tagThetaDeviation[i] = sqrt( tagThetaDeviation[i] / tagAvailableFrameCount );


		//maxTagCenterDeviation = max( maxTagCenterDeviation, tagCenterDeviation[i] );
		//minTagCenterDeviation = min( minTagCenterDeviation, tagCenterDeviation[i] );
		//maxTagCountDeviation = max( maxTagCountDeviation, tagCountDeviation[i] );
		//minTagCountDeviation = min( minTagCountDeviation, tagCountDeviation[i] );
		//maxTagRhoDeviation = max( maxTagRhoDeviation, tagRhoDeviation[i] );
		//minTagRhoDeviation = min( minTagRhoDeviation, tagRhoDeviation[i] );
		//maxTagThetaDeviation = max( maxTagThetaDeviation, tagThetaDeviation[i] );
		//minTagThetaDeviation = min( minTagThetaDeviation, tagThetaDeviation[i] );

	}
	/*
	double deltaTagCenterDeviation, deltaTagCountDeviation, deltaTagRhoDeviation, deltaTagThetaDeviation;
	deltaTagCenterDeviation = maxTagCenterDeviation - minTagCenterDeviation;
	deltaTagCountDeviation = maxTagCountDeviation - minTagCountDeviation;
	deltaTagRhoDeviation = maxTagRhoDeviation - minTagRhoDeviation;
	deltaTagThetaDeviation = maxTagThetaDeviation - minTagThetaDeviation;

	for ( int i = 0; i < tagNum; i++ ) {

	double tempEnergy[5];

	tempEnergy[0] = (tagCenterDeviation[i] - minTagCenterDeviation) / deltaTagCenterDeviation;
	tempEnergy[1] = (tagCountDeviation[i] - minTagCountDeviation) / deltaTagCountDeviation;
	tempEnergy[2] = (tagRhoDeviation[i] - minTagRhoDeviation) / deltaTagRhoDeviation;
	tempEnergy[3] = (tagThetaDeviation[i] - minTagThetaDeviation) / deltaTagThetaDeviation;
	tempEnergy[4] = max( tempEnergy[0], max( tempEnergy[1], max( tempEnergy[2], tempEnergy[3] ) ) );
	tagEnergy[i] = 0;
	for ( int j = 0; j < 5; j++ ) tagEnergy[i] += tempEnergy[j];
	tagEnergy[i] /= 5;

	}
	*/
	double maxEnergy[layerLimit], minEnergy[layerLimit], deltaEnergy[layerLimit];
	/*
	maxEnergy[0] = -INF;
	minEnergy[0] = INF;

	for ( int i = 0; i < tagNum; i++ ) {
	maxEnergy[0] = max( maxEnergy[0], tagEnergy[i] );
	minEnergy[0] = min( minEnergy[0], tagEnergy[i] );
	}
	deltaEnergy[0] = maxEnergy[0] - minEnergy[0];
	*/
	vector<int> tagLayer( tagNum, 1 );
	/*
	for ( int i = 0; i < tagNum; i++ ) {
	tagEnergy[i] = deltaEnergy[0] > 0 ? (tagEnergy[i] - minEnergy[0]) / deltaEnergy[0] : 0.5;

	tagLayer[i] = 1;
	if ( tagEnergy[i] < 0.1 ) tagLayer[i] = 0;
	if ( tagEnergy[i] > 0.9 ) tagLayer[i] = 2;
	}
	*/

	userInteraction( pixelTag, frames, tagLayer, frameCount, frameSize, frameStId );


	double maxCent[layerLimit], maxCount[layerLimit], maxRho[layerLimit], maxTheta[layerLimit];
	double minCent[layerLimit], minCount[layerLimit], minRho[layerLimit], minTheta[layerLimit];
	double deltaCent[layerLimit], deltaCount[layerLimit], deltaRho[layerLimit], deltaTheta[layerLimit];

	for ( int i = 0; i < layerLimit; i++ ) {
		maxCent[i] = maxCount[i] = maxRho[i] = maxTheta[i] = -INF;
		minCent[i] = minCount[i] = minRho[i] = minTheta[i] = INF;
	}
	for ( int i = 0; i < tagNum; i++ ) {

		int layer = tagLayer[i];
		maxCent[layer] = max( maxCent[layer], tagCenterDeviation[i] );
		minCent[layer] = min( minCent[layer], tagCenterDeviation[i] );
		maxCount[layer] = max( maxCount[layer], tagCountDeviation[i] );
		minCount[layer] = min( minCount[layer], tagCountDeviation[i] );
		maxRho[layer] = max( maxRho[layer], tagRhoDeviation[i] );
		minRho[layer] = min( minRho[layer], tagRhoDeviation[i] );
		maxTheta[layer] = max( maxTheta[layer], tagThetaDeviation[i] );
		minTheta[layer] = min( minTheta[layer], tagThetaDeviation[i] );
	}
	for ( int i = 0; i < layerLimit; i++ ) {

		deltaCent[i] = maxCent[i] - minCent[i];
		deltaCount[i] = maxCount[i] - minCount[i];
		deltaRho[i] = maxRho[i] - minRho[i];
		deltaTheta[i] = maxTheta[i] - minTheta[i];
	}
	for ( int i = 0; i < tagNum; i++ ) {

		int layer = tagLayer[i];
		tagCenterDeviation[i] = deltaCent[layer] > 0 ? (tagCenterDeviation[i] - minCent[layer]) / deltaCent[layer] : 0.5;
		tagCountDeviation[i] = deltaCount > 0 ? (tagCountDeviation[i] - minCount[layer]) / deltaCount[layer] : 0.5;
		tagRhoDeviation[i] = deltaRho > 0 ? (tagRhoDeviation[i] - minRho[layer]) / deltaRho[layer] : 0.5;
		tagThetaDeviation[i] = deltaTheta > 0 ? (tagThetaDeviation[i] - minTheta[layer]) / deltaTheta[layer] : 0.5;
		//tagEnergy[i] = (tagCenterDeviation[i] + tagCountDeviation[i] + tagRhoDeviation[i] + tagThetaDeviation[i] ) / 4;
		double maxTagEnergy = max( tagCenterDeviation[i], max( tagCountDeviation[i], max( tagRhoDeviation[i], tagThetaDeviation[i] ) ) );
		tagEnergy[i] = (tagCenterDeviation[i] + tagCountDeviation[i] + tagRhoDeviation[i] + tagThetaDeviation[i] + maxTagEnergy) / 5;
	}

	for ( int i = 0; i < layerLimit; i++ ) {
		maxEnergy[i] = -INF;
		minEnergy[i] = INF;
	}
	for ( int i = 0; i < tagNum; i++ ) {

		int layer = tagLayer[i];
		maxEnergy[layer] = max( maxEnergy[layer], tagEnergy[i] );
		minEnergy[layer] = min( minEnergy[layer], tagEnergy[i] );
	}
	for ( int i = 0; i < layerLimit; i++ ) deltaEnergy[i] = maxEnergy[i] - minEnergy[i];
	//cout << maxEnergy[2] << " " << minEnergy[2] << endl;
	//cout << maxEnergy[0] << " " << maxEnergy[1] << endl;
	//cout << minEnergy[0] << " " << minEnergy[1] << endl;
	for ( int i = 0; i < tagNum; i++ ) {

		int layer = tagLayer[i];
		//cout << layer << " " << tagEnergy[i] << " ";

		tagEnergy[i] = deltaEnergy[layer] > 0 ? (tagEnergy[i] - minEnergy[layer]) / deltaEnergy[layer] : 0.5;

		switch ( layer ) {
			case 0:
				tagEnergy[i] = tagEnergy[i] * 0.25 + 0;
				break;
			case 1:
				tagEnergy[i] = tagEnergy[i] * 0.4 + 0.3;
				break;
			case 2:
				tagEnergy[i] = tagEnergy[i] * 0.25 + 0.75;
				break;
			default:
				break;
		}
		//if ( tagEnergy[i] < 0 || tagEnergy[i] > 1 ) cout << tagEnergy[i] << " " << layer << endl;

		//cout << " Tag Num : " << i << " cent " << tagCenterDeviation[i] << " count " << tagCountDeviation[i] << " rho " << tagRhoDeviation[i] << " theta " << tagThetaDeviation[i] << endl;
		//cout << " energy " << tagEnergy[i] << endl;
	}

	pixelEnergy.clear();
	for ( int t = 0; t < frameCount; t++ ) {
		pixelEnergy.push_back( Mat::zeros( frameSize, CV_8UC1 ) );
		for ( int y = 0; y < frameSize.height; y++ ) {
			uchar *rowData = pixelEnergy[t].ptr<uchar>( y );
			for ( int x = 0; x < frameSize.width; x++ ) {

				pixelNum = txy2pos( t, x, y, frameCount, frameSize );
				int nowTag = pixelTag[pixelNum];
				rowData[x] = cvRound( tagEnergy[nowTag] * 255 );
			}
		}

		//imshow( "pixelEnergy", pixelEnergy[t] );
		//waitKey();
	}

	char pngName[100];
	for ( int i = 0; i < frameCount; i++ ) {
		sprintf( pngName, "pixelEnergy//%d.png", i + frameStId );
		imwrite( pngName, pixelEnergy[i] );
	}
}

void calcEdgeProtect( vector<Mat> &frames, vector<Mat> &edgeProtect ) {

	edgeProtect.clear();

	int frameCount = frames.size();

	Mat grayFrame, gradX, absGradX, tempMat;
	for ( int t = 0; t < frameCount; t++ ) {

		cvtColor( frames[t], grayFrame, COLOR_RGB2GRAY );
		Sobel( grayFrame, gradX, CV_16S, 1, 0, 1 );
		convertScaleAbs( gradX, absGradX );
		//absGradX = 0.5*absGradX;
		absGradX.convertTo( tempMat, CV_8UC1 );
		edgeProtect.push_back( tempMat.clone() );
	}
	/*
		char pngName[100];
		for ( int i = 0; i < frameCount; i++ ) {
		sprintf( pngName, "edgeProtect//%d.png", i );
		imwrite( pngName, edgeProtect[i] );
		}*/
}


#endif