#include "structure.h"

using namespace std;


bool GEO_SAMEPOINT(Point p1, Point p2) {
	if (p1.p_x == p2.p_x && p1.p_y == p2.p_y) {
		return true;
	}
	else {
		return false;
	}
}

bool Rect::GEO_SAMERECT(Rect r1, Rect r2) {
	if (GEO_SAMEPOINT(r1.r_ll, r2.r_ll) && GEO_SAMEPOINT(r1.r_ur, r2.r_ur)) {
		return true;
	}
	else {
		return false;
	}
}

bool Rect::GEO_OVERLAP(Rect r1, Rect r2) {
	if (r1.r_ll.p_x < r2.r_ur.p_x && r2.r_ll.p_x < r1.r_ur.p_x && r1.r_ll.p_y < r2.r_ur.p_y && r2.r_ll.p_y < r1.r_ur.p_y) {
		return true;
	}
	else {
		return false;
	}
}

bool Rect::GEO_TOUCH(Rect r1, Rect r2) {
	if (r1.r_ll.p_x <= r2.r_ur.p_x && r2.r_ll.p_x <= r1.r_ur.p_x && r1.r_ll.p_y <= r2.r_ur.p_y && r2.r_ll.p_y <= r1.r_ur.p_y) {
		return true;
	}
	else {
		return false;
	}
}


int Rect::GEO_WIDTH() {
	return r_ur.p_x - r_ll.p_x;
}

int Rect::GEO_HEIGHT() {
	return r_ur.p_y - r_ll.p_y;
}

/*Rect Rect::GeoRotRect() {
	Point p1(r_ll.p_y - r_mid.p_y + r_mid.p_x, -(r_ll.p_x - r_mid.p_x) + r_mid.p_y);
	Point p2(r_ur.p_y - r_mid.p_y + r_mid.p_x, -(r_ur.p_x - r_mid.p_x) + r_mid.p_y);
	Rect des(p1, p2);

	des.r_ll.p_y = des.r_ll.p_y - GEO_WIDTH();
	des.r_ur.p_y = des.r_ur.p_y + GEO_WIDTH();

	return des;
}*/

/*Tile::Tile(int body, Point ll, string name)
{
	ti_body = body;
	ti_lb = nullptr;
	ti_bl = nullptr;
	ti_tr = nullptr;
	ti_rt = nullptr;
	ti_ll = ll;
	ti_name = name;
}*/

int TOP(Tile* tp) { //еiзя
	if (tp->ti_body == 3) {
		if (tp->ti_rt == NULL) {
			return (BOTTOM(tp) + 1);
		}
		else {
			return (BOTTOM(RT(tp)));
		}
	}
	else {
		return (BOTTOM(RT(tp)));
	}
}

int RIGHT(Tile* tp) { 
	if (tp->ti_body == 3) {
		if (tp->ti_tr == NULL) {
			return (LEFT(tp) + 1);
		}
		else {
			return (LEFT(TRight(tp)));
		}
	}
	else {
		return (LEFT(TRight(tp)));
	}
}

int AREA(Rect a) {
	return a.GEO_HEIGHT() * a.GEO_WIDTH();
}

int AREA(Tile* a) {
	return (a->ti_ur.p_x - a->ti_ll.p_x) * (a->ti_ur.p_y - a->ti_ll.p_y);
}

int HEIGHT(Tile* t) {
	return t->ti_ur.p_y - t->ti_ll.p_y;
}

int WIDTH(Tile* t) {
	return t->ti_ur.p_x - t->ti_ll.p_x;
}

