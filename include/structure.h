#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_

#include <iostream>
#include <string>
#include <vector>

using namespace std;



/*extern string pl_file_name;
extern string node_file_name;*/
extern string case_file_name;


class Point {
	private:
		
	public:
		int p_x;
		int p_y;

		Point() {};
		Point(int x, int y) {
			p_x = x;
			p_y = y;
		};

		Point(const Point &p) {
			p_x = p.p_x;
			p_y = p.p_y;
		};

		~Point() {};

		void print()
		{
			cout << "(" << p_x << "," << p_y << ")" << endl;
		};
		
};

bool GEO_SAMEPOINT(Point p1, Point p2);

class Rect {
	private:
		
	public:
		Point r_ll;
		Point r_ur;
		Point r_mid; //center

		Rect() {};
		Rect(Point ll, Point ur) {
			r_ll = ll;
			r_ur = ur;
			r_mid.p_x = (r_ll.p_x + r_ur.p_x) / 2;
			r_mid.p_y = (r_ll.p_y + r_ur.p_y) / 2;
		};
		~Rect() {};

		static bool GEO_SAMERECT(Rect r1, Rect r2);
		static bool GEO_OVERLAP(Rect r1, Rect r2);
		static bool GEO_TOUCH(Rect r1, Rect r2);
		//static bool GEO_SURROUND(Rect r1, Rect r2);
		//static bool GEO_SURROUND_STRONG(Rect r1, Rect r2);
		//static bool GEO_ENCLOSE(Point p, Rect r);
		//static bool GEO_RECTNULL(Rect r);
		//static void GEO_EXPAND(Rect src, int amount, Rect dst);
		//Rect GeoRotRect();	//rotate deg 90*/
		int GEO_WIDTH();
		int GEO_HEIGHT();

		void print()
		{
			cout << "ll";
			r_ll.print();
			cout << "ur";
			r_ur.print();
		};

};


struct connect_info
{
	string ti_name;
	Point ti_ll;
	Point ti_ur;
};


class Tile
{
	private:

	public:
		int ti_body; // 0 -> white tile , 1 -> fixed module , 2 -> soft module , 3 -> boundary   
		string ti_name;
		Tile* ti_lb;
		Tile* ti_bl;
		Tile* ti_tr;
		Tile* ti_rt;
		Point ti_ll; //���U���y��
		Point ti_ur;
		Point ti_mid;
		vector<connect_info> name;
		vector<int> weight;
		
		Tile() {};
		Tile(int body, Point ll, Point ur, string name) {
			ti_body = body;
			ti_lb = nullptr;
			ti_bl = nullptr;
			ti_tr = nullptr;
			ti_rt = nullptr;
			ti_ll = ll;
			ti_ur = ur;
			Point p((ll.p_x + ur.p_x) / 2, (ll.p_y + ur.p_y) / 2);
			ti_mid = p;
			ti_name = name;
		};
		Tile(Tile* t) {
			ti_body = t->ti_body;
			ti_name = t->ti_name;
			ti_lb = t->ti_lb;
			ti_bl = t->ti_bl;
			ti_tr = t->ti_tr;
			ti_rt = t->ti_rt;
			ti_ll = t->ti_ll; //���U���y��
			ti_ur = t->ti_ur;
			ti_mid = t->ti_mid;
			name = t->name;
			weight = t->weight;
		}
		~Tile() {
			
		
		};
		
		void print_coordinate()
		{
			cout << "ll(" << ti_ll.p_x << "," << ti_ll.p_y << "), "
				 << "ur(" << ti_ur.p_x << "," << ti_ur.p_y << ")" << endl;
		};

};


class Trantile
{
private:

public:
	Tile* t1;
	Tile* t2;
	Tile* t3;
	Point frame_mid;
	string ti_name;
	int dir;
	int totalweight;
	vector<int> name;
	vector<int> type;
	vector<int> weight;

	Trantile() {};
	~Trantile() {};
};


#define	BOTTOM(tp)		(tp->ti_ll.p_y)
#define	LEFT(tp)		(tp->ti_ll.p_x)
#define	LB(tp)		(tp->ti_lb)
#define	BLeft(tp)		(tp->ti_bl)
#define	TRight(tp)		(tp->ti_tr)
#define	RT(tp)		(tp->ti_rt)
#define	TileWidth(tp)		(tp->ti_ur.p_x - tp->ti_ll.p_x)
#define	TileHeight(tp)		(tp->ti_ur.p_y - tp->ti_ll.p_y)
#define	TileArea(tp)		(TileWidth(tp) * TileHeight(tp))

int TOP(Tile* tp);

int RIGHT(Tile* tp);

class Plane
{
	private:

	public:
		Tile *pl_left;		/* Left pseudo-tile */
		Tile *pl_top;		/* Top pseudo-tile */
		Tile *pl_right;		/* Right pseudo-tile */
		Tile *pl_bottom;	/* Bottom pseudo-tile */
		int pl_height;
		int pl_width;
		Tile *pl_hint;		/* head */
		float hpwl;
		vector<Tile*> soft_tile_list;
		vector<Tile*> fixed_tile_list;
		int legal;

		
	


		Plane(int width, int height) {
			Point p0(0, 0);
			Point p1(-1, 0);
			Point p2(width, 0);
			Point p3(0, -1);
			Point p4(0, height);
			Point p5(width + 1, height);
			Point p6(width, height + 1);
			Point p7(width, height);

			Tile* left = new Tile(3, p1, p4, "LEFT");
			Tile* right = new Tile(3, p2, p5, "RIGHT");
			Tile* top = new Tile(3, p4, p6, "TOP");
			Tile* bottom = new Tile(3, p3, p2, "BOTTOM");
			Tile* new_tile = new Tile(0, p0, p7,"white");
		
			new_tile->ti_bl = left;
			new_tile->ti_lb = bottom;
			new_tile->ti_tr = right;
			new_tile->ti_rt = top;

			left->ti_lb = bottom;
			left->ti_rt = top;
			left->ti_tr = new_tile;

			top->ti_bl = left;
			top->ti_lb = new_tile;
			top->ti_tr = right;

			right->ti_bl = new_tile;
			right->ti_lb = bottom;
			right->ti_rt = top;

			bottom->ti_bl = left;
			bottom->ti_rt = new_tile;
			bottom->ti_tr = right;

			pl_left = left;
			pl_right = right;
			pl_top = top;
			pl_bottom = bottom;
			pl_hint = new_tile;
			pl_height = height;
			pl_width = width;
			hpwl = 0;
			legal = 1;
		};
		~Plane();



};

int AREA(Rect a);
int AREA(Tile* a);
int HEIGHT(Tile* t);
int WIDTH(Tile* t);




#endif _STRUCTURE_H_
