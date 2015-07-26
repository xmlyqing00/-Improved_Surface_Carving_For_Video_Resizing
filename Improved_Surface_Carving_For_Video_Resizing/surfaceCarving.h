/*
*	Copyright (C)   Lyq root#lyq.me
*	File Name     : surfaceCarving.h
*	Creation Time : 2015-3-19
*	Environment   : Windows8.1-64bit VS2013 OpenCV2.4.9
*	Homepage      : http://www.lyq.me
*/

#ifndef SURFACE_CARVING
#define SURFACE_CARVING

#include "baseFunction.h"

bool bfsGraphForward( vector<int> &tag, vector<int> &edgeHead, vector<typeEdge> &edge ) {

	int N = edgeHead.size();
	//queue<int> que;
	typeQue &que = *(new typeQue);
	tag = vector<int>( N, -1 );
	tag[0] = 0;
	que.push( 0 );
	while ( !que.empty() ) {

		int nowP = que.front();
		que.pop();
		for ( int p = edgeHead[nowP]; p != -1; p = edge[p].next ) {

			int nextP = edge[p].y;
			if ( tag[nextP] == -1 && edge[p].w > 0 ) {
				tag[nextP] = tag[nowP] + 1;
				que.push( nextP );
			}
		}
	}

	delete &que;

	if ( tag[N - 1] > 0 ) {
		return 1;
	} else {
		return 0;
	}
}
void bfsGraphBackward( vector<int> &tag, vector<int> &edgeHead, vector<typeEdge> &edge ) {

	int N = edgeHead.size();
	//queue<int> que;
	typeQue &que = *(new typeQue);
	tag = vector<int>( N, -1 );
	tag[N - 1] = 0;
	que.push( N - 1 );
	while ( !que.empty() ) {

		int nowP = que.front();
		que.pop();
		for ( int p = edgeHead[nowP]; p != -1; p = edge[p].next ) {

			int nextP = edge[p].y;
			if ( tag[nextP] == -1 ) {
				que.push( nextP );
				tag[nextP] = tag[nowP] + 1;
			}
		}
	}
	delete &que;
}

// unused
int dfsDinic( int nowP, int minFlow, vector<int> &tag, vector<int> &edgeHead, vector<typeEdge> &edge ) {

	//cout << nowP << " " << tag[nowP] << " " << minFlow << endl;
	if ( minFlow == 0 ) return 0;
	if ( nowP == tag.size() - 1 ) return minFlow;
	int allFlow = 0;
	for ( int p = edgeHead[nowP]; p != -1; p = edge[p].next ) {

		int nextP = edge[p].y;
		if ( tag[nextP] != tag[nowP] + 1 ) continue;
		if ( edge[p].w <= 0 ) continue;

		int flow = dfsDinic( nextP, min( minFlow, edge[p].w ), tag, edgeHead, edge );
		if ( flow > 0 ) {
			edge[p].w -= flow;
			edge[p ^ 1].w += flow;
			allFlow += flow;
		}
	}
	if ( allFlow == 0 ) tag[nowP] = -1;
	return allFlow;
}

int preMaxFlow( vector<int> &edgeHead, vector<typeEdge> &edge ) {

	int T = edgeHead.size() - 1;
	//int cnt0 = 0, cnt1 = 0;
	int ans = 0;
	
	for ( int i = edgeHead[0]; i != -1; i = edge[i].next ) {

		//cnt0++;
		int neckCap = INF;
		int t = edge[i].y;

		while ( t != T ) {

			for ( int p = edgeHead[t]; p != -1; p = edge[p].next ) {
				
				if ( edge[p].flag ) {
					neckCap = min( neckCap, edge[p].w );
					t = edge[p].y;
					break;
				}
			}
		}

		if ( neckCap == 0 ) continue;
	
		ans += neckCap;
		t = edge[i].y;
		//if ( t == 1 ) cout << neckCap << endl;
		//cnt1++;

		while ( t != T ) {

			for ( int p = edgeHead[t]; p != -1; p = edge[p].next ) {

				if ( edge[p].flag ) {
					edge[p].w -= neckCap;
					edge[p ^ 1].w += neckCap;
					t = edge[p].y;
					break;
				}
			}
		}
	}
	//cout << "cnt0 : " << cnt0 << " cnt1 : " << cnt1 << endl;
	//cout << "pre " << ans << endl;
	return ans;
}
int maxFlow( vector<int> &edgeHead, vector<typeEdge> &edge ) {
	
	int ans = 0;

	ans += preMaxFlow( edgeHead, edge );
	
	int N = edgeHead.size();
	int S = 0, T = N - 1;
	vector<int> curEdge = edgeHead;
	vector<int> gap( N + 2, 0 );
	vector<int> tag;
	vector<int> preEdge( N, 0 );

	bfsGraphBackward( tag, edgeHead, edge );
	for ( int i = 0; i < N; i++ ) gap[tag[i]]++;

	int nowP = S;

	while ( tag[S] < N ) {

		if ( nowP == T ) {

			int neckFlow = INF;
			int neckPoint;

			for ( int i = S; i != T; i = edge[curEdge[i]].y ) {
				
				if ( neckFlow > edge[curEdge[i]].w ) {
					neckFlow = edge[curEdge[i]].w;
					neckPoint = i;
				}
			}
			for ( int i = S; i != T; i = edge[curEdge[i]].y ) {

				edge[curEdge[i]].w -= neckFlow;
				edge[curEdge[i] ^ 1].w += neckFlow;
			}
			
			ans += neckFlow;
			nowP = neckPoint;
		}

		int p0 = -1;
		for ( int p = curEdge[nowP]; p != -1; p = edge[p].next ) {

			if ( edge[p].w > 0 && tag[nowP] == tag[edge[p].y] + 1 ) {
				p0 = p;
				break;
			}
		}

		if ( p0 != -1 ) {

			curEdge[nowP] = p0;
			preEdge[edge[p0].y] = nowP;
			nowP = edge[p0].y;
		} else {
		
			gap[tag[nowP]]--;
			if ( gap[tag[nowP]] == 0 ) break;
			curEdge[nowP] = edgeHead[nowP];

			int tempTag = N;
			for ( int p = edgeHead[nowP]; p != -1; p = edge[p].next ) {
				if ( edge[p].w > 0 ) tempTag = min( tempTag, tag[edge[p].y] );
			}
			tag[nowP] = tempTag + 1;
			gap[tag[nowP]]++;
			if ( nowP != S ) nowP = preEdge[nowP];
		}
	}
	
	/*
	long long ans = 0;
	vector<int> tag;

	while ( bfsGraphForward( tag, edgeHead, edge ) ) {

		int tans = 1;
		while ( tans > 0 ) {

			tans = dfsDinic( 0, 0x7fffffff, tag, edgeHead, edge );
			ans += tans;
		}
	}
	*/
	return ans;
}
	
void calcSurfaceBand( vector<Mat> &frames, vector<int> &num2pos, vector<int> &edgeHead, vector<typeEdge> &edge, 
					  vector< vector<int> > &removePts ) {

	int frameCount = frames.size();
	Size frameSize = frames[0].size();
	int N = edgeHead.size();
	int M = frameSize.width * frameSize.height * frameCount + 2;
	bool isRemoved;

	vector<int> tag;
	bfsGraphForward( tag, edgeHead, edge );

	for ( int i = 1; i < N; i++ ) {

		if ( tag[i] == -1 ) continue;
		isRemoved = false;
		for ( int p = edgeHead[i]; p != -1; p = edge[p].next ) {

			//cout << edge[p].flag << " " << tag[edge[p].y] << endl;
			if ( edge[p].flag && tag[edge[p].y] == -1 ) {
				
				isRemoved = true;
				break;
			}
		}

		if ( !isRemoved ) continue;

		int t0, x0, y0;
		if ( !pos2txy( num2pos[i], t0, x0, y0, M, frameCount, frameSize ) ) continue;

		removePts[t0][y0] = x0;
		
	}
}

void surfaceCarving( vector<Mat> &frames, vector<Mat> &pixelEnergy, vector<Mat> &edgeProtect, vector< vector<int> > &removePts,
					 vector<vector<int>> &linkHead, vector<typeLink> &link ) {

	int frameCount = frames.size();
	Size frameSize = frames[0].size();
	typeLink oneLink;

	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {

			int rest = removePts[t][y];

			if ( linkHead[t][y] == -1 ) {
				
				oneLink.y = rest;
				oneLink.next = -1;
				linkHead[t][y] = link.size();
				link.push_back( oneLink );
				continue;
			}
			
			oneLink = link[linkHead[t][y]];
			if ( oneLink.y > rest ) {
				
				oneLink.y = rest;
				oneLink.next = linkHead[t][y];
				linkHead[t][y] = link.size();
				link.push_back( oneLink );
				continue;
			} else {
				rest = rest - oneLink.y + 1;
			}

			for ( int p = linkHead[t][y]; p != -1; p = link[p].next) {

				int nextP = link[p].next;
				if ( nextP == -1 || (link[nextP].y - link[p].y - 1) >= rest ) {
					
					oneLink.y = link[p].y + rest;
					oneLink.next = nextP;
					link[p].next = link.size();
					link.push_back( oneLink );
					break;
				} else {
					rest -= link[nextP].y - link[p].y - 1;
				}
			}
		}
	}

	FILE *file = fopen( "link.txt", "w" );
	for ( int t = 0; t < frameCount; t++ ) {
		for ( int y = 0; y < frameSize.height; y++ ) {
			fprintf( file, " t %d, y %d : ", t, y );
			for ( int p = linkHead[t][y]; p != -1; p = link[p].next ) {
				fprintf( file, "%d ", link[p].y );
			}
			fprintf( file, "\n" );
		}
	}
	fclose( file );
	for ( int t = 0; t < frameCount; t++ ) {

		for ( int y = 0; y < frameSize.height; y++ ) {

			Vec3b *rowDataFrame = frames[t].ptr<Vec3b>( y );
			uchar *rowDataPixelEnergy = pixelEnergy[t].ptr<uchar>( y );
			uchar *rowDataEdgeProtect = edgeProtect[t].ptr<uchar>( y );
			
			for ( int x = removePts[t][y] +1; x < frameSize.width; x++ ) {
				rowDataFrame[x - 1] = rowDataFrame[x];
				rowDataPixelEnergy[x - 1] = rowDataPixelEnergy[x];
				rowDataEdgeProtect[x - 1] = rowDataEdgeProtect[x];
			}
		}

		frames[t] = frames[t].colRange( 0, frameSize.width - 1 );
		pixelEnergy[t] = pixelEnergy[t].colRange( 0, frameSize.width - 1 );
		edgeProtect[t] = edgeProtect[t].colRange( 0, frameSize.width - 1 );
	}
}

#endif