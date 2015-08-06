/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : baseFunction.h
*	Creation Time : 2015-3-19
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef BASE_FUNCTION
#define BASE_FUNCTION

#include <opencv2\opencv.hpp>
#include <opencv2\video\video.hpp>
using namespace cv;

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>
#include <queue>
#include <ctime>

using namespace std;

//#define SHOW_GRAPH

#define INF 2000000000
#define sqr(_x) ( (_x) * (_x) )
#define max_que_cap 50000000

struct typeEdge {
	int x, y, w, next;
	bool flag;
};

struct typeLink {
	int y, next;
	
};

class typeQue {

private:
	int data[max_que_cap];
	int p_front, p_size, p_rear;

public:
	typeQue() {
		p_front = 0;
		p_size = 0;
		p_rear = 0;
	}
	bool empty() {
		if ( p_size == 0 ) return true;
		else return false;
	}
	void push( int x ) {
		data[p_rear++] = x;
		if ( p_rear == max_que_cap ) p_rear = 0;
		p_size++;
		if ( p_size > max_que_cap ) {
			cout << "que out of size !! " << endl;
			return;
		}
	}
	int front() {
		return data[p_front];
	}
	void pop() {
		p_front++;
		if ( p_front == max_que_cap ) p_front = 0;
		p_size--;
	}
	int size() {
		return p_size;
	}
	void clear() {
		p_size = 0;
		p_front = 0;
		p_rear = 0;
	}
};

void help( void ) {

	printf( "===	Copyright (C) Lyq root#lyq.me\n"
			"===	Program Name  : Improved Surface Carving for Cartoon Retargeting\n"
			"===	Creation Time : 2015-3-19 22:50:00 UTC+8\n"
			"===	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9\n"
			"===	Homepage      : http://www.lyq.me\n"
			"===	\n"
			"===	This program can resize a cartoon in any propotion.\n"
			"===	It learns the pixels' Energy and calculates the min surface to urve.\n"
			"===	Reference: Improved Seam Carving for Video Retargeting\n"
			"		\n" );
}

inline int txy2pos( int t, int x, int y, int frameCount, const Size &frameSize ) {

	if ( t < 0 || t >= frameCount ) return -1;
	if ( x < 0 || x >= frameSize.width ) return -1;
	if ( y < 0 || y >= frameSize.height ) return -1;
	return t * frameSize.width * frameSize.height + y * frameSize.width + x + 1;

}

inline bool pos2txy( int num, int &t, int &x, int &y, int N, int frameCount, const Size &frameSize ) {

	if ( num <= 0 || num >= N - 1 ) {
		cout << "pos2txy error : " << num << " t " << t << " x " << x << " y " << y << " count " << frameCount << " size " << frameSize.height << " " << frameSize.width << endl;
		return false;
	}

	num--;
	int temp = frameSize.width * frameSize.height;
	t = num / temp;
	num = num - t * temp;
	y = num / frameSize.width;
	num = num - y * frameSize.width;
	x = num;

	return true;
}
inline void buildEdge( vector<int> &edgeHead, vector<typeEdge> &edge, int x, int y, int w, bool flag = false ) {

	typeEdge oneEdge;
	oneEdge.x = x;
	oneEdge.y = y;
	oneEdge.w = w;
	oneEdge.flag = flag;
	oneEdge.next = edgeHead[x];
	edgeHead[x] = edge.size();
	edge.push_back( oneEdge );

}

inline void buildLink( vector<int> &linkHead, vector<typeLink> &link, int x, int y ) {

	typeLink oneLink;
	oneLink.y = y;
	oneLink.next = linkHead[x];
	linkHead[x] = link.size();
	link.push_back( oneLink );
}

inline int colorDiff( Vec3b p0, Vec3b p1 ) {

	int colorP0, colorP1;
	double diff = 0;

	for ( int i = 0; i < 3; i++ ) {

		colorP0 = p0.val[i];
		colorP1 = p1.val[i];
		diff += sqr( colorP0 - colorP1 );
	}

	diff = sqrt( diff );
	return cvRound( diff );
}

inline Vec3b colorMid( Vec3b p0, Vec3b p1 ) {

	int colorP0, colorP1;
	Vec3b mid;

	for ( int i = 0; i < 3; i++ ) {

		colorP0 = p0.val[i];
		colorP1 = p1.val[i];
		mid.val[i] = ( colorP0 + colorP1 ) / 2;
	}

	return mid;
}

inline void showGraph( vector<int> &edgeHead, vector<typeEdge> &edge, int N, int frameCount, Size frameSize ) {
	
	FILE *file = fopen( "graph.txt", "w" );

	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			for ( int x = 0; x < frameSize.width; x++ ) {
				
				int q = txy2pos( t, x, y, frameCount, frameSize );
				for ( int p = edgeHead[q]; p != -1; p = edge[p].next ) {
					
					if ( edge[p].y == 0 || edge[p].y == N - 1 ) continue;
					int t0, y0, x0;

					if ( pos2txy( edge[p].y, t0, x0, y0, N, frameCount, frameSize ) == false ) continue;
					fprintf( file, "t %d y %d x %d t0 %d y0 %d x0 %d w %d flag %d\n", t, y, x, t0, y0, x0, edge[p].w, edge[p].flag );
				}
			}
		}
	}
	fclose( file );


}
/*
void downSample( Mat &inFrame, Mat &outFrame, int ratio = 2 ) {
	
	Mat preserveFrame = inFrame.clone();
	Size frameSize1 = inFrame.size();
	Size frameSize2 = Size( (frameSize1.width + ratio - 1) / ratio, (frameSize1.height + ratio - 1) / 2 );
	int frameType = outFrame.type();
	outFrame = Mat( frameSize2, frameType );
	
	for ( int y = 0; y < frameSize2.height; y++ ) {

		if ( frameType == CV_8UC1 ) {
			uchar *rowDataInFrameC1 = preserveFrame.ptr<uchar>( y << 1 );
			uchar *rowDataOutFrameC1 = outFrame.ptr<uchar>( y );
			for ( int x = 0; x < frameSize2.width; x++ ) {
				rowDataOutFrameC1[x] = rowDataInFrameC1[x << 1];
			}

		} else {
			Vec3b *rowDataInFrameC3 = preserveFrame.ptr<Vec3b>( y << 1 );
			Vec3b *rowDataOutFrameC3 = outFrame.ptr<Vec3b>( y );
			for ( int x = 0; x < frameSize2.width; x++ ) {
				rowDataOutFrameC3[x] = rowDataInFrameC3[x << 1];
			}
		}

	}
	
}*/


#endif