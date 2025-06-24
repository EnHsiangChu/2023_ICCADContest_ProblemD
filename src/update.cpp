#include <iostream>
#include <queue>
#include <cmath>
#include<fstream>
#include <limits.h>

#include "update.h"
using namespace std;

struct insert_point
{
	Point check_p;
	float cost;
	int dir;
	Rect canusearea;
	Rect realarea;
};

struct trans_point
{
	Point check_point;
	Point frame_mid;
	int type;	// 1:rt	2:tr 3:dr 4:rd 5:ld 6:dl 7:tl 8:lt
	float cost;
	Rect area1;
	Rect area2;
	int tran;
};

bool canMergeVertical(Tile* tile1, Tile* tile2) {
	if (tile1->ti_body != 0 || tile2->ti_body != 0)
		return false;

	if (LEFT(tile1) != LEFT(tile2) || RIGHT(tile1) != RIGHT(tile2))
		return false;

	return true;
}

bool canMergeHorizontal(Tile* tile1, Tile* tile2) {
	if (tile1->ti_body != 0 || tile2->ti_body != 0)
		return false;

	if (TOP(tile1) != TOP(tile2) || BOTTOM(tile1) != BOTTOM(tile2))
		return false;

	return true;
}

int DownMerge(Tile* &tile, Plane* &plane) {
	Tile* btp = LB(tile);
	if (canMergeVertical(tile, btp)) {
		TiJoinY(tile, btp, plane);
		return 1;
	}
	return 0;
}

Tile* SplitAndMerge(Tile* tile, Tile* target, Plane* plane, Rect rect) {
	
	if (LEFT(tile) < rect.r_ll.p_x)
	{
		Tile* old_tile = tile;
		tile = TiSplitX(tile, rect.r_ll.p_x);
		DownMerge(old_tile, plane);
	}

	if (RIGHT(tile) > rect.r_ur.p_x)
	{
		Tile* new_tile = TiSplitX(tile, rect.r_ur.p_x);
		DownMerge(new_tile, plane);
	}

	if (target != NULL) {
		TiJoinY(tile, target, plane);
	}
	
	target = tile;
	return target;
}

Tile* InsertFixedTile(Rect rect, Plane* plane) {
	Tile* tp = TiSrPoint(plane->pl_hint, plane, rect.r_ll);

	if (BOTTOM(tp) < rect.r_ll.p_y) {
		tp = TiSplitY(tp, rect.r_ll.p_y);
	}

	Tile* target = NULL;
	while (TOP(tp) <= rect.r_ur.p_y)
	{
		target = SplitAndMerge(tp, target, plane, rect);
		tp = target;
		tp = RT(tp);
	}

	if ((BOTTOM(tp) < rect.r_ur.p_y) && (TOP(tp) > rect.r_ur.p_y)) {
		TiSplitY(tp, rect.r_ur.p_y);
		target = SplitAndMerge(tp, target, plane, rect);
	}

	//chu 2023/7/15
	/*DownMerge(RT(target), plane);
	DownMerge(TR(target), plane);
	DownMerge(LB(target), plane);
	DownMerge(BL(target), plane);*/
	//end

	target->ti_body = 1;
	return target;
}

connect_info read_connection(Tile * t)
{
	connect_info c;
	c.ti_ll.p_x = t->ti_ll.p_x;
	c.ti_ll.p_y = t->ti_ll.p_y;
	c.ti_ur.p_x = t->ti_ur.p_x;
	c.ti_ur.p_y = t->ti_ur.p_y;
	c.ti_name = t->ti_name;

	return c;
}

Tile* TiSplitX(Tile* tile, int x) {
	Tile* newtile = new Tile;
	Tile* tp;

	newtile->ti_body = 0;
	LEFT(newtile) = x;
	BOTTOM(newtile) = BOTTOM(tile);
	BLeft(newtile) = tile;
	TRight(newtile) = TRight(tile);
	RT(newtile) = RT(tile);
	newtile->ti_ur = tile->ti_ur;
	Point p1((newtile->ti_ll.p_x + newtile->ti_ur.p_x) / 2, (newtile->ti_ll.p_y + newtile->ti_ur.p_y) / 2);
	newtile->ti_mid = p1;
	Point p2(x, newtile->ti_ur.p_y);
	tile->ti_ur = p2;
	Point p3((tile->ti_ll.p_x + tile->ti_ur.p_x) / 2, (tile->ti_ll.p_y + tile->ti_ur.p_y) / 2);
	tile->ti_mid = p3;

	for (tp = TRight(tile); BLeft(tp) == tile; tp = LB(tp))	//Adjust corner stitches along the right edge
		BLeft(tp) = newtile;
	TRight(tile) = newtile;

	for (tp = RT(tile); LEFT(tp) >= x; tp = BLeft(tp))		//Adjust corner stitches along the top edge
		LB(tp) = newtile;
	RT(tile) = tp;

	for (tp = LB(tile); RIGHT(tp) <= x; tp = TRight(tp)) {
		if (tp->ti_body == 3) {
			break;
		}
	}
	LB(newtile) = tp;

	while (RT(tp) == tile)
	{
		RT(tp) = newtile;
		tp = TRight(tp);
	}

	
	return newtile;
}

Tile* TiSplitY(Tile* tile, int y) {
	Tile* newtile = new Tile;
	
	Tile* tp;

	newtile->ti_body = 0;
	LEFT(newtile) = LEFT(tile);
	BOTTOM(newtile) = y;
	LB(newtile) = tile;
	RT(newtile) = RT(tile);
	TRight(newtile) = TRight(tile);
	newtile->ti_ur = tile->ti_ur;
	Point p1((newtile->ti_ll.p_x + newtile->ti_ur.p_x) / 2, (newtile->ti_ll.p_y + newtile->ti_ur.p_y) / 2);
	newtile->ti_mid = p1;
	Point p2(newtile->ti_ur.p_x, y);
	tile->ti_ur = p2;
	Point p3((tile->ti_ll.p_x + tile->ti_ur.p_x) / 2, (tile->ti_ll.p_y + tile->ti_ur.p_y) / 2);
	tile->ti_mid = p3;

	for (tp = RT(tile); LB(tp) == tile; tp = BLeft(tp)) {	//Adjust corner stitches along top edge
		LB(tp) = newtile;
	}
	RT(tile) = newtile;

	for (tp = TRight(tile); BOTTOM(tp) >= y; tp = LB(tp)) {	//Adjust corner stitches along right edge
		BLeft(tp) = newtile;
	}
	TRight(tile) = tp;

	for (tp = BLeft(tile); TOP(tp) <= y; tp = RT(tp)) {
		if (tp->ti_body == 3) {
			break;
		}
	}

	BLeft(newtile) = tp;
	//cout << "         " << BLeft(newtile)->ti_name << endl;

	while (TRight(tp) == tile)
	{
		TRight(tp) = newtile;
		tp = RT(tp);
	}

	
	return newtile;
}

void TiJoinX(Tile* tile1, Tile* tile2, Plane* &plane) {
	Tile *tp;

	for (tp = RT(tile2); LB(tp) == tile2; tp = BLeft(tp)) {
		LB(tp) = tile1;
	}

	for (tp = LB(tile2); RT(tp) == tile2; tp = TRight(tp)) {
		RT(tp) = tile1;
	}

	if (LEFT(tile1) < LEFT(tile2)) {
		for (tp = TRight(tile2); BLeft(tp) == tile2; tp = LB(tp)) {
			BLeft(tp) = tile1;
		}
			
		TRight(tile1) = TRight(tile2);
		RT(tile1) = RT(tile2);

		tile1->ti_ur = tile2->ti_ur;
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}
	else {
		for (tp = BLeft(tile2); TRight(tp) == tile2; tp = RT(tp)) {
			TRight(tp) = tile1;
		}
		BLeft(tile1) = BLeft(tile2);
		LB(tile1) = LB(tile2);
		LEFT(tile1) = LEFT(tile2);
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}

	if (plane->pl_hint == tile2) {
		plane->pl_hint = tile1;
	}	
	
	//chu 2023/7/17
	/*for (int i = 0; i < plane->soft_tile_list.size(); i++) {
		if (tile2 == plane->soft_tile_list[i]) {
			plane->soft_tile_list[i]=tile1;
		}
	}*/
	
	//end

	delete tile2;
	tile2 = NULL;
	
}

void TiJoinY(Tile* tile1, Tile* tile2, Plane* &plane) {
	Tile *tp;

	for (tp = TRight(tile2); BLeft(tp) == tile2; tp = LB(tp)) {
		BLeft(tp) = tile1;
	}

	for (tp = BLeft(tile2); TRight(tp) == tile2; tp = RT(tp)) {
		TRight(tp) = tile1;
	}

	if (BOTTOM(tile1) < BOTTOM(tile2)) {
		for (tp = RT(tile2); LB(tp) == tile2; tp = BLeft(tp)) {
			LB(tp) = tile1;
		}
		RT(tile1) = RT(tile2);
		TRight(tile1) = TRight(tile2);
		tile1->ti_ur = tile2->ti_ur;
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}
	else {
		for (tp = LB(tile2); RT(tp) == tile2; tp = TRight(tp)) {
			RT(tp) = tile1;
		}
		LB(tile1) = LB(tile2);
		BLeft(tile1) = BLeft(tile2);
		BOTTOM(tile1) = BOTTOM(tile2);
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}

	if (plane->pl_hint == tile2) {
		plane->pl_hint = tile1;
	}

	//chu 2023/7/17
	/*for (int i = 0; i < plane->soft_tile_list.size(); i++) {
		if (tile2 == plane->soft_tile_list[i]) {
			plane->soft_tile_list[i] = tile1;
		}
	}*/
	//end




	delete tile2;
	tile2 = NULL;
	
}

void Enumerate(Plane* plane, vector<Tile*>& white)
{
	white.clear();
	Point p(1, plane->pl_height - 1);
	queue<Tile*> next_search;
	next_search.push(GoToPoint(plane->pl_hint, p));

	while (!next_search.empty())
	{
		int unvisit = 1;
		Tile* tp = next_search.front();

		while (tp->ti_body != 3)
		{
			for (int i = 0;i < white.size();i++)
			{
				if (GEO_SAMEPOINT(tp->ti_ll, white[i]->ti_ll))
				{
					unvisit = 0;
				}
			}
			if (unvisit)
			{
				if (tp->ti_body == 0)
				{
					white.push_back(tp);
				}
			}
			
			if (tp->ti_bl->ti_body == 3)
			{
				next_search.push(tp->ti_lb);
			}
			else if (tp->ti_lb->ti_ll.p_y >= tp->ti_bl->ti_ll.p_y)
			{
				next_search.push(tp->ti_lb);
			}
			
			

			tp = tp->ti_tr;
		}
		next_search.pop();
	}
}

int mini_dis(Point mid, int chip_width, int chip_height)
{
	int left = mid.p_x;
	int right = chip_width - left;
	int bottom = mid.p_y;
	int top = chip_height - bottom;

	if (left > right)
		left = right;

	if (left > bottom)
		left = bottom;

	if (left > top)
		left = top;

	return left;
}

int mini_dis(Tile* r, int chip_width, int chip_height)
{
	return r->ti_mid.p_x + r->ti_mid.p_y;
}

int length(Tile* soft_tile, int soft_x, int soft_y) {
	int leng = 0;
	for (int i = 0; i < soft_tile->name.size(); i++) {
		Point p_mid((soft_tile->name[i].ti_ll.p_x + soft_tile->name[i].ti_ur.p_x) / 2
			, (soft_tile->name[i].ti_ll.p_y + soft_tile->name[i].ti_ur.p_y) / 2);


		leng += (abs(p_mid.p_x - soft_x) + 
			abs(p_mid.p_y - soft_y)) * soft_tile->weight[i];
	}
	return leng;
}

int wire_length(Tile* soft_tile, Tile* white_tile) {
	soft_tile->ti_mid.p_x = (soft_tile->ti_ll.p_x + soft_tile->ti_ur.p_x) / 2;
	soft_tile->ti_mid.p_y = (soft_tile->ti_ll.p_y + soft_tile->ti_ur.p_y) / 2;
	int x1 = soft_tile->ti_mid.p_x - soft_tile->ti_ll.p_x + white_tile->ti_ll.p_x;
	int y1 = soft_tile->ti_mid.p_y - soft_tile->ti_ll.p_y + white_tile->ti_ll.p_y;
	int x2 = white_tile->ti_ur.p_x - soft_tile->ti_ur.p_x + soft_tile->ti_mid.p_x;
	int y2 = white_tile->ti_ur.p_y - soft_tile->ti_ur.p_y + soft_tile->ti_mid.p_y;

	int leng1 = length(soft_tile, x1, y1);
	int leng2 = length(soft_tile, x2, y2);

	if(leng1 < leng2) {
		return leng1;
	}
	else {
		return leng2;
	}
}

int manhattan(Tile* soft_tile, Tile* white_tile) {
	int leng1 = abs(soft_tile->ti_mid.p_x - white_tile->ti_ll.p_x) + abs(soft_tile->ti_mid.p_y - white_tile->ti_ll.p_y);
	int leng2 = abs(soft_tile->ti_mid.p_x - white_tile->ti_ur.p_x) + abs(soft_tile->ti_mid.p_y - white_tile->ti_ur.p_y);
	if (leng1 < leng2) {
		return leng1;
	}
	else {
		return leng2;
	}
}

void sort_white_tile_order(vector<Tile*>& white_tile_list, Tile* soft_tile) {

	for (int i = 1; i < white_tile_list.size(); i++) {
		Tile* key = white_tile_list[i];
		int j = i - 1;

		while (j >= 0 && wire_length(soft_tile, white_tile_list[j]) > wire_length(soft_tile, key)) {
			white_tile_list[j + 1] = white_tile_list[j];
			j--;
		}
		if (j >= 0 && wire_length(soft_tile, white_tile_list[j]) == wire_length(soft_tile, key)) {
			if (manhattan(soft_tile, white_tile_list[j]) > manhattan(soft_tile, key)) {
				white_tile_list[j + 1] = white_tile_list[j];
				white_tile_list[j] = key;
			}
			else {
				white_tile_list[j + 1] = key;
			}
		}
		else {
			white_tile_list[j + 1] = key;
		}
	}
}

void sort_area(int start, int end, vector<Tile*>& soft_tile_list) {

	for (int i = start + 1; i <= end; i++) {
		Tile* key = soft_tile_list[i];
		int j = i - 1;

		while (j >= start && TileArea(soft_tile_list[j]) < TileArea(key)) {
			soft_tile_list[j + 1] = soft_tile_list[j];
			j--;
		}
		soft_tile_list[j + 1] = key;
	}
}

void sort_x(int start, int end, vector<Tile*>& soft_tile_list) {

	for (int i = start + 1; i <= end; i++) {
		Tile* key = soft_tile_list[i];
		int j = i - 1;

		while (j >= start && soft_tile_list[j]->ti_ll.p_x > key->ti_ll.p_x) {
			soft_tile_list[j + 1] = soft_tile_list[j];
			j--;
		}
		soft_tile_list[j + 1] = key;
	}
}

int insert_order(vector<Tile*>& soft_tile_list) {
	sort_area(0, soft_tile_list.size() - 1, soft_tile_list);

	int max_area = TileArea(soft_tile_list[0]);

	float bound = 0;
	if (log10(max_area) - (int)log10(max_area) > 0.7)
	{
		bound = 0.7 + (int)log10(max_area); //�i�令���n+0.7
	}
	else
	{
		bound = (int)log10(max_area);
	}

	for (int i = 0; i < soft_tile_list.size(); i++)
	{
		if (log10(TileArea(soft_tile_list[i])) < bound)
		{
			sort_x(0, i - 1, soft_tile_list);
			return i;
		}
	}
}

float point_cost(Tile* tp, Point mid)
{
	float cost = 0;
	for (int i = 0;i < tp->name.size();i++)
	{
		Point mid1((tp->name[i].ti_ll.p_x + tp->name[i].ti_ur.p_x) / 2,
			(tp->name[i].ti_ll.p_y + tp->name[i].ti_ur.p_y) / 2);

		cost = cost + tp->weight[i] * (abs(mid1.p_x - mid.p_x) + abs(mid1.p_y - mid.p_y));
	}
	return cost;
}

float HeightPerWidth(Rect r)
{
	if (r.GEO_HEIGHT() > r.GEO_WIDTH())
		return float(r.GEO_HEIGHT()) / float(r.GEO_WIDTH());
	else
		return float(r.GEO_WIDTH()) / float(r.GEO_HEIGHT());
}

void sort_y_up(int start, int end, vector<Rect>& soft_module) {

	for (int i = start + 1; i <= end; i++) {
		Rect key = soft_module[i];
		int j = i - 1;

		while (j >= start && soft_module[j].r_ll.p_y > key.r_ll.p_y) {
			soft_module[j + 1] = soft_module[j];
			j--;
		}
		soft_module[j + 1] = key;
	}
}

void sort_y_down(int start, int end, vector<Rect>& soft_module) {

	for (int i = start + 1; i <= end; i++) {
		Rect key = soft_module[i];
		int j = i - 1;

		while (j >= start && soft_module[j].r_ll.p_y < key.r_ll.p_y) {
			soft_module[j + 1] = soft_module[j];
			j--;
		}
		soft_module[j + 1] = key;
	}
}

Rect CanUseArea1(vector<Tile*> white, Point start, int target)
{
	vector<Rect> rect;
	Point p2 = start;
	for (int i = 0; i < white.size(); i++)
	{
		if (start.p_x >= white[i]->ti_ll.p_x && start.p_y >= white[i]->ti_ll.p_y)
		{
			if (start.p_x < white[i]->ti_ur.p_x && start.p_y < white[i]->ti_ur.p_y)
				p2 = white[i]->ti_ur;
		}
	}
	Rect area(start, p2);
	for (int i = 0; i < white.size(); i++)
	{
		if (white[i]->ti_ll.p_x > start.p_x || white[i]->ti_tr->ti_ll.p_x < start.p_x)
			continue;
		if (white[i]->ti_ll.p_y < start.p_y)
			continue;

		Point p1(start.p_x, white[i]->ti_ll.p_y);
		Point p2(white[i]->ti_tr->ti_ll.p_x, white[i]->ti_rt->ti_ll.p_y);
		if (p2.p_x > area.r_ur.p_x)
			p2.p_x = area.r_ur.p_x;
		Rect r(p1, p2);
		rect.push_back(r);
	}

	sort_y_up(0, rect.size() - 1, rect);

	for (int i = 1; i < rect.size(); i++)
	{
		if (rect[i].r_ll.p_y != area.r_ur.p_y)
			break;

		Rect temp(area.r_ll, rect[i].r_ur);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ur.p_x > area.r_ur.p_x)
					rect[j].r_ur.p_x = area.r_ur.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ur.p_x > area.r_ur.p_x)
					rect[j].r_ur.p_x = area.r_ur.p_x;
			}
		}
	}
	return area;
}

Rect CanUseArea2(vector<Tile*> white, Point start, int target)
{
	vector<Rect> rect;
	Point p1 = start;
	Point p2 = start;
	for (int i = 0; i < white.size(); i++)
	{
		if (start.p_x > white[i]->ti_ll.p_x && start.p_y >= white[i]->ti_ll.p_y)
		{
			if (start.p_x <= white[i]->ti_ur.p_x && start.p_y < white[i]->ti_ur.p_y)
			{
				p1.p_x = white[i]->ti_ll.p_x;
				p1.p_y = start.p_y;
				p2.p_x = start.p_x;
				p2.p_y = white[i]->ti_ur.p_y;
			}
		}
	}
	Rect area(p1, p2);
	for (int i = 0; i < white.size(); i++)
	{
		if (white[i]->ti_ll.p_x > start.p_x || white[i]->ti_ur.p_x < start.p_x)
			continue;
		if (white[i]->ti_ll.p_y < start.p_y)
			continue;

		Point p1(white[i]->ti_ll.p_x, white[i]->ti_ll.p_y);
		Point p2(start.p_x, white[i]->ti_ur.p_y);
		if (p1.p_x < area.r_ll.p_x)
			p1.p_x = area.r_ll.p_x;
		Rect r(p1, p2);
		rect.push_back(r);
	}

	sort_y_up(0, rect.size() - 1, rect);

	for (int i = 1; i < rect.size(); i++)
	{
		if (rect[i].r_ll.p_y != area.r_ur.p_y)
			break;

		Point p1(rect[i].r_ll.p_x, area.r_ll.p_y);
		Rect temp(p1, rect[i].r_ur);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
	}
	return area;
}

Rect CanUseArea3(vector<Tile*> white, Point start, int target)
{
	vector<Rect> rect;
	Point p1 = start;
	Point p2 = start;
	for (int i = 0; i < white.size(); i++)
	{
		if (start.p_x >= white[i]->ti_ll.p_x && start.p_y > white[i]->ti_ll.p_y)
		{
			if (start.p_x < white[i]->ti_ur.p_x && start.p_y <= white[i]->ti_ur.p_y)
			{
				p1.p_x = start.p_x;
				p1.p_y = white[i]->ti_ll.p_y;
				p2.p_x = white[i]->ti_ur.p_x;
				p2.p_y = start.p_y;
			}
		}
	}
	Rect area(p1, p2);
	for (int i = 0; i < white.size(); i++)
	{
		if (white[i]->ti_ll.p_x > start.p_x || white[i]->ti_tr->ti_ll.p_x < start.p_x)
			continue;
		if (white[i]->ti_ll.p_y > start.p_y)
			continue;

		Point p1(start.p_x, white[i]->ti_ll.p_y);
		Point p2(white[i]->ti_tr->ti_ll.p_x, white[i]->ti_rt->ti_ll.p_y);
		if (p2.p_x > area.r_ur.p_x)
			p2.p_x = area.r_ur.p_x;
		Rect r(p1, p2);
		rect.push_back(r);
	}

	sort_y_down(0, rect.size() - 1, rect);

	for (int i = 1; i < rect.size(); i++)
	{
		if (rect[i].r_ur.p_y != area.r_ll.p_y)
			break;

		Point p1(rect[i].r_ur.p_x, area.r_ur.p_y);
		Rect temp(rect[i].r_ll, p1);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ur.p_x > area.r_ur.p_x)
					rect[j].r_ur.p_x = area.r_ur.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ur.p_x > area.r_ur.p_x)
					rect[j].r_ur.p_x = area.r_ur.p_x;
			}
		}
	}
	return area;
}

Rect CanUseArea4(vector<Tile*> white, Point start, int target)
{
	vector<Rect> rect;
	Point p1 = start;
	for (int i = 0; i < white.size(); i++)
	{
		if (start.p_x > white[i]->ti_ll.p_x && start.p_y > white[i]->ti_ll.p_y)
		{
			if (start.p_x <= white[i]->ti_ur.p_x && start.p_y <= white[i]->ti_ur.p_y)
			{
				p1 = white[i]->ti_ll;
			}
		}
	}
	Rect area(p1, start);
	for (int i = 0; i < white.size(); i++)
	{
		if (white[i]->ti_ll.p_x > start.p_x || white[i]->ti_tr->ti_ll.p_x < start.p_x)
			continue;
		if (white[i]->ti_ll.p_y > start.p_y)
			continue;

		Point p1(white[i]->ti_ll.p_x, white[i]->ti_ll.p_y);
		Point p2(start.p_x, white[i]->ti_rt->ti_ll.p_y);
		if (p1.p_x < area.r_ll.p_x)
			p1.p_x = area.r_ll.p_x;
		Rect r(p1, p2);
		rect.push_back(r);
	}

	sort_y_down(0, rect.size() - 1, rect);

	for (int i = 1; i < rect.size(); i++)
	{
		if (rect[i].r_ur.p_y != area.r_ll.p_y)
			break;

		Rect temp(rect[i].r_ll, area.r_ur);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1; j < rect.size(); j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
	}
	return area;
}



void InsertSoftTile(Plane* plane, vector<Tile*>& soft_tile_list, int type) {  

	if (type) {
		int ind = 0;
		ind = insert_order(soft_tile_list);

		for (int i = ind + 1; i < soft_tile_list.size(); i++) {
			Tile* key = soft_tile_list[i];
			int j = i - 1;

			while (j >= ind && mini_dis(soft_tile_list[j], plane->pl_width, plane->pl_height) > mini_dis(key, plane->pl_width, plane->pl_height)) {
				soft_tile_list[j + 1] = soft_tile_list[j];
				j--;
			}
			soft_tile_list[j + 1] = key;
		}
	}

	for (int i = 0; i < soft_tile_list.size(); i++)
	{
		vector<Tile*> white;
		Enumerate(plane, white);
		vector<insert_point> point;
		vector<Rect> model;

		int target = AREA(soft_tile_list[i]);
		double ratio = 1;
		for (int j = 0; j < 20; j++)
		{
			int shorter = ceil(sqrt(double(target / ratio)));
			int longer = ceil(ratio * shorter);

			Point a(0, 0);
			Point b(shorter, longer);
			Point c(longer, shorter);

			model.push_back(Rect(a, b));
			model.push_back(Rect(a, c));

			ratio = ratio + 0.05;
		}

		/*for (int j = 0;j < model.size();j++)
		{
			cout << model[j].r_ur.p_x << " " << model[j].r_ur.p_x << " " << HeightPerWidth(model[j]) << endl;
		}*/

		for (int j = 0; j < white.size(); j++)
		{
			Point white_mid((white[j]->ti_ll.p_x + white[j]->ti_ur.p_x) / 2, (white[j]->ti_ll.p_y + white[j]->ti_ur.p_y) / 2);

			insert_point in;
			in.cost = INT_MAX;
			in.check_p.p_x = white[j]->ti_ll.p_x;
			in.check_p.p_y = white[j]->ti_ll.p_y;
			in.canusearea = CanUseArea1(white, in.check_p, target);
			in.dir = 1;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.check_p.p_x = white[j]->ti_ur.p_x;
			in.check_p.p_y = white[j]->ti_ll.p_y;
			in.canusearea = CanUseArea2(white, in.check_p, target);
			in.dir = 2;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.check_p.p_x = white[j]->ti_ll.p_x;
			in.check_p.p_y = white[j]->ti_ur.p_y;
			in.canusearea = CanUseArea3(white, in.check_p, target);
			in.dir = 3;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.check_p.p_x = white[j]->ti_ur.p_x;
			in.check_p.p_y = white[j]->ti_ur.p_y;
			in.canusearea = CanUseArea4(white, in.check_p, target);
			in.dir = 4;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.check_p.p_x = white_mid.p_x;
			in.check_p.p_y = white_mid.p_y;
			in.canusearea = CanUseArea1(white, in.check_p, target);
			in.dir = 1;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.canusearea = CanUseArea2(white, in.check_p, target);
			in.dir = 2;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.canusearea = CanUseArea3(white, in.check_p, target);
			in.dir = 3;
			if (target <= AREA(in.canusearea))
				point.push_back(in);

			in.canusearea = CanUseArea4(white, in.check_p, target);
			in.dir = 4;
			if (target <= AREA(in.canusearea))
				point.push_back(in);
		}

		for (int j = 0; j < point.size(); j++)
		{
			for (int k = 0; k < model.size(); k++)
			{
				if (point[j].canusearea.GEO_HEIGHT() < model[k].GEO_HEIGHT()
					|| point[j].canusearea.GEO_WIDTH() < model[k].GEO_WIDTH())
				{
					continue;
				}

				Point mid;
				if (point[j].dir == 1)
				{
					mid.p_x = point[j].check_p.p_x + model[k].r_mid.p_x;
					mid.p_y = point[j].check_p.p_y + model[k].r_mid.p_y;
				}
				else if (point[j].dir == 2)
				{
					mid.p_x = point[j].check_p.p_x - model[k].r_mid.p_x;
					mid.p_y = point[j].check_p.p_y + model[k].r_mid.p_y;
				}
				else if (point[j].dir == 3)
				{
					mid.p_x = point[j].check_p.p_x + model[k].r_mid.p_x;
					mid.p_y = point[j].check_p.p_y - model[k].r_mid.p_y;
				}
				else if (point[j].dir == 4)
				{
					mid.p_x = point[j].check_p.p_x - model[k].r_mid.p_x;
					mid.p_y = point[j].check_p.p_y - model[k].r_mid.p_y;
				}

				float newcost = point_cost(soft_tile_list[i], mid);
				if (newcost < point[j].cost)
				{
					point[j].cost = newcost;
					if (point[j].dir == 1)
					{
						point[j].realarea.r_ll = point[j].check_p;
						point[j].realarea.r_ur.p_x = point[j].check_p.p_x + model[k].r_ur.p_x;
						point[j].realarea.r_ur.p_y = point[j].check_p.p_y + model[k].r_ur.p_y;
					}
					else if (point[j].dir == 2)
					{
						point[j].realarea.r_ll.p_x = point[j].check_p.p_x - model[k].r_ur.p_x;
						point[j].realarea.r_ll.p_y = point[j].check_p.p_y;
						point[j].realarea.r_ur.p_x = point[j].check_p.p_x;
						point[j].realarea.r_ur.p_y = point[j].check_p.p_y + model[k].r_ur.p_y;
					}
					else if (point[j].dir == 3)
					{
						point[j].realarea.r_ll.p_x = point[j].check_p.p_x;
						point[j].realarea.r_ll.p_y = point[j].check_p.p_y - model[k].r_ur.p_y;
						point[j].realarea.r_ur.p_x = point[j].check_p.p_x + model[k].r_ur.p_x;
						point[j].realarea.r_ur.p_y = point[j].check_p.p_y;
					}
					else if (point[j].dir == 4)
					{
						point[j].realarea.r_ll.p_x = point[j].check_p.p_x - model[k].r_ur.p_x;
						point[j].realarea.r_ll.p_y = point[j].check_p.p_y - model[k].r_ur.p_y;
						point[j].realarea.r_ur = point[j].check_p;
					}
				}
			}
		}

		int min = 0;
		for (int j = 1; j < point.size(); j++)
		{
			if (point[j].cost < point[min].cost)
				min = j;
		}

		if (point.size() == 0 || point[min].cost == INT_MAX)
		{
			plane->legal = 0;
			break;
		}
		else
		{
			string n = soft_tile_list[i]->ti_name;
			vector<connect_info> c = soft_tile_list[i]->name;
			vector<int> w = soft_tile_list[i]->weight;

			soft_tile_list[i] = InsertFixedTile(point[min].realarea, plane);
			soft_tile_list[i]->name = c;
			soft_tile_list[i]->weight = w;
			soft_tile_list[i]->ti_name = n;
			soft_tile_list[i]->ti_body = 2;

			for (int j = 0; j < plane->fixed_tile_list.size(); j++)
			{
				for (int k = 0; k < plane->fixed_tile_list[j]->name.size(); k++)
				{
					if (plane->fixed_tile_list[j]->name[k].ti_name == soft_tile_list[i]->ti_name)
					{
						plane->fixed_tile_list[j]->name[k].ti_ll = soft_tile_list[i]->ti_ll;
						plane->fixed_tile_list[j]->name[k].ti_ur = soft_tile_list[i]->ti_ur;
					}
				}
			}
			for (int j = 0; j < plane->soft_tile_list.size(); j++)
			{
				for (int k = 0; k < plane->soft_tile_list[j]->name.size(); k++)
				{
					if (plane->soft_tile_list[j]->name[k].ti_name == soft_tile_list[i]->ti_name)
					{
						plane->soft_tile_list[j]->name[k].ti_ll = soft_tile_list[i]->ti_ll;
						plane->soft_tile_list[j]->name[k].ti_ur = soft_tile_list[i]->ti_ur;
					}
				}
			}
		}
	}

	double hpwl = 0;
	for (int i = 0; i < soft_tile_list.size(); i++) {
		for (int j = 0; j < soft_tile_list[i]->name.size(); j++) {
			double x1 = double((soft_tile_list[i]->ti_ll.p_x + soft_tile_list[i]->ti_ur.p_x) / 2);
			double y1 = double((soft_tile_list[i]->ti_ll.p_y + soft_tile_list[i]->ti_ur.p_y) / 2);
			double x2 = double((soft_tile_list[i]->name[j].ti_ll.p_x + soft_tile_list[i]->name[j].ti_ur.p_x) / 2);
			double y2 = double((soft_tile_list[i]->name[j].ti_ll.p_y + soft_tile_list[i]->name[j].ti_ur.p_y) / 2);

			double length = abs(x2 - x1) + abs(y2 - y1);
			hpwl = hpwl + soft_tile_list[i]->weight[j] * length;
		}
	}
	for (int i = 0; i < plane->fixed_tile_list.size(); i++) {
		for (int j = 0; j < plane->fixed_tile_list[i]->name.size(); j++) {
			double x1 = double((plane->fixed_tile_list[i]->ti_ll.p_x + plane->fixed_tile_list[i]->ti_ur.p_x) / 2);
			double y1 = double((plane->fixed_tile_list[i]->ti_ll.p_y + plane->fixed_tile_list[i]->ti_ur.p_y) / 2);
			double x2 = double((plane->fixed_tile_list[i]->name[j].ti_ll.p_x + plane->fixed_tile_list[i]->name[j].ti_ur.p_x) / 2);
			double y2 = double((plane->fixed_tile_list[i]->name[j].ti_ll.p_y + plane->fixed_tile_list[i]->name[j].ti_ur.p_y) / 2);

			double length = abs(x2 - x1) + abs(y2 - y1);
			hpwl = hpwl + plane->fixed_tile_list[i]->weight[j] * length;
		}
	}
	plane->hpwl = hpwl / 2;



	//chu 2023/7/15
	vector<Tile*> white_tile_list;
	Enumerate(plane, white_tile_list);
	for (int i = 0; i < white_tile_list.size(); i++)
	{
		if (DownMerge(white_tile_list[i], plane))
		{
			i = 0;
			Enumerate(plane, white_tile_list);
		}
		if (canMergeHorizontal(white_tile_list[i], BLeft(white_tile_list[i])))
		{
			i = 0;
			Enumerate(plane, white_tile_list);
		}
	}
	//end

}

Plane * Build_newPlane(Plane* plane_best, vector<Tile*>& soft_tile_list, vector<Tile*>& fixed_tile_list)
{
	Plane *plane = new Plane(plane_best->pl_width, plane_best->pl_height);
	for (int j = 0; j < fixed_tile_list.size(); j++) {
		Point p_ll(fixed_tile_list[j]->ti_ll.p_x, fixed_tile_list[j]->ti_ll.p_y);
		Point p_ur(fixed_tile_list[j]->ti_ur.p_x, fixed_tile_list[j]->ti_ur.p_y);
		Rect input_rect(p_ll, p_ur);

		Tile *input_tile = InsertFixedTile(input_rect, plane);
		for (int k = 0; k < fixed_tile_list[j]->name.size(); k++)
		{
			connect_info c_tp;
			input_tile->name.push_back(c_tp);
			input_tile->name[k] = fixed_tile_list[j]->name[k];
		}
		input_tile->ti_name = fixed_tile_list[j]->ti_name;
		input_tile->weight = fixed_tile_list[j]->weight;

		plane->fixed_tile_list.push_back(input_tile);
	}


	plane->soft_tile_list = soft_tile_list;
	return plane;
}

void tile_free_memory(Plane * &plane, vector<Tile*>& soft_tile_list, vector<Tile*>& fixed_tile_list)
{
	vector<Tile*> white;
	Enumerate(plane, white);

	for (int i = white.size() - 1; i >= 0; i--)
	{
		delete white[i];
		white[i] == NULL;
	}

	for (int j = 0; j < fixed_tile_list.size(); j++)
	{
		for (int k = 0; k < plane->fixed_tile_list.size(); k++)
		{
			if (plane->fixed_tile_list[k] == NULL) continue;

			if (fixed_tile_list[j]->ti_name == plane->fixed_tile_list[k]->ti_name)
			{
				if (fixed_tile_list[j] != plane->fixed_tile_list[k])
				{
					delete plane->fixed_tile_list[k];
					plane->fixed_tile_list[k] = NULL;
				}
			}
		}
	}
	for (int j = 0; j < soft_tile_list.size(); j++)
	{
		for (int k = 0; k < plane->soft_tile_list.size(); k++)
		{
			if (plane->soft_tile_list[k] == NULL) continue;

			if (soft_tile_list[j]->ti_name == plane->soft_tile_list[k]->ti_name)
			{
				if (soft_tile_list[j] != plane->soft_tile_list[k])
				{
					delete plane->soft_tile_list[k];
					plane->soft_tile_list[k] = NULL;
				}
			}
		}
	}
}

void random_arrange_order(int exchange_time, Plane * &plane,vector<Tile*> &soft_list,
	vector<pair<vector<connect_info>, Rect>> &soft_global_result)
{
	int soft_num = plane->soft_tile_list.size();
	for (int i = 0; i < exchange_time; i++) {
		int te1 = rand() % soft_num;
		int te2 = rand() % soft_num;
		Tile* temp_t = plane->soft_tile_list[te1];
		plane->soft_tile_list[te1] = plane->soft_tile_list[te2];
		plane->soft_tile_list[te2] = temp_t;

		swap(soft_list[te1], soft_list[te2]);
		swap(soft_global_result[te1], soft_global_result[te2]);
	}

}

void read_global_result(Plane * &plane, vector<pair<vector<connect_info>, Rect>> soft_global_result, vector<pair<vector<connect_info>, string>> fixed_global_result)
{
	for (int i = 0; i < plane->fixed_tile_list.size(); i++)
	{
		plane->fixed_tile_list[i]->name.clear();
		for (int j = 0; j < fixed_global_result[i].first.size(); j++)
		{
			plane->fixed_tile_list[i]->name.push_back(fixed_global_result[i].first[j]);
		}
	}
	for (int i = 0; i < plane->soft_tile_list.size(); i++)
	{
		plane->soft_tile_list[i]->name.clear();
		plane->soft_tile_list[i]->ti_ll = soft_global_result[i].second.r_ll;
		plane->soft_tile_list[i]->ti_ur = soft_global_result[i].second.r_ur;
		plane->soft_tile_list[i]->ti_mid = soft_global_result[i].second.r_mid;
		for (int j = 0; j < soft_global_result[i].first.size(); j++)
		{
			plane->soft_tile_list[i]->name.push_back(soft_global_result[i].first[j]);
		}
	}
}



void RemoveTile_tran(Tile*& tile, Plane*& plane)
{
	vector<Tile*> white_tile_list;


	tile = TiSrPoint(NULL, plane, tile->ti_ll);
	const int del_ytop = TOP(tile);
	const int del_ybot = BOTTOM(tile);

	tile->ti_body = 0;

	Tile* right_start = TRight(tile);
	Tile* left_start = BLeft(tile);


	if ((right_start->ti_body == 0) && (TOP(right_start) > del_ytop))
	{
		TiSplitY(right_start, del_ytop);
	}

	Tile* tp = NULL;

	for (tp = right_start; BOTTOM(tp) >= del_ybot;)
	{

		Tile* next = LB(tp);

		Tile* tmp = tile;

		if (BOTTOM(tp) > del_ybot)
		{
			tmp = TiSplitY(tile, BOTTOM(tp));
		}

		if (tp->ti_body != 0)
		{
			tp = next;
			continue;
		}
		else
		{
			TiJoinX(tp, tmp, plane);
		}

		tp = next;

	}

	if ((tp->ti_body == 0) && (TOP(tp) > del_ybot))
	{
		tp = TiSplitY(tp, del_ybot);
		TiJoinX(tp, tile, plane);
	}

	if ((BLeft(RT(tp))->ti_body != 0) && canMergeVertical(tp, RT(tp)))
	{
		TiJoinY(tp, RT(tp), plane);
	}

	if ((left_start->ti_body == 0) && (BOTTOM(left_start) < del_ybot))
	{
		left_start = TiSplitY(left_start, del_ybot);
	}

	tp = left_start;

	for (tp = left_start; TOP(tp) <= del_ytop; ) {
		Tile* next = RT(tp);

		Tile* tmp = TRight(tp);
		if (TOP(tp) < TOP(tmp))
		{
			TiSplitY(tmp, TOP(tp));
		}

		if (tp->ti_body != 0)
		{
			tp = next;
			continue;
		}

		for (tmp = TRight(tp); BOTTOM(tp) < BOTTOM(tmp); tmp = LB(tmp))
		{
			TiSplitY(tp, BOTTOM(tmp));
		}
		next = RT(tp);
		TiJoinX(tp, tmp, plane);
		if (canMergeVertical(tp, LB(tp)))
		{
			TiJoinY(tp, LB(tp), plane);
		}

		tp = next;
	}

	if ((tp->ti_body == 0) && (BOTTOM(tp) < del_ytop)) {

		Tile* next = TiSplitY(tp, del_ytop);

		//chu 2023/7/14 added
		Tile* next2;
		while (BOTTOM(TRight(tp)) > BOTTOM(tp))
		{
			next2 = TiSplitY(tp, BOTTOM(TRight(tp)));
			if (canMergeHorizontal(next2, TRight(next2)))
			{
				TiJoinX(next2, TRight(next2), plane);
			}
		}
		//end

		if (canMergeHorizontal(tp, TRight(tp)))
		{
			TiJoinX(tp, TRight(tp), plane);
		}
		tp = next;
	}


	if (canMergeVertical(tp, LB(tp)))
	{

		TiJoinY(tp, LB(tp), plane);
	}

	/*for (int i = 0; i < plane->soft_tile_list.size(); i++)
	{
		if (plane->soft_tile_list[i]->ti_body == 0)
		{
			plane->soft_tile_list.erase(plane->soft_tile_list.begin() + i);
			break;
		}
	}*/

	Enumerate(plane, white_tile_list);
	for (int i = 0; i < white_tile_list.size(); i++)
	{
		if (DownMerge(white_tile_list[i], plane)) {
			i = 0;
			Enumerate(plane, white_tile_list);
		}

		if (canMergeHorizontal(white_tile_list[i], BLeft(white_tile_list[i])))
		{
			i = 0;
			Enumerate(plane, white_tile_list);
		}
	}
	//end


	// reset hint tile
	plane->pl_hint = TiSrPoint(tp, plane, plane->pl_hint->ti_ll);
}

float point_cost_tran(vector<Trantile> t, int num, vector<Tile*> fixed_tile_list, Point mid1, int mode)
{
	float cost = 0;
	for (int i = 0; i < t[num].name.size(); i++)
	{
		Point mid2;
		int w = 1;
		if (t[num].type[i] == 0)
		{
			mid2 = fixed_tile_list[t[num].name[i]]->ti_mid;

			if (mode == 1)
				w = 2;
		}
		else if (t[num].type[i] == 1)
		{
			mid2 = t[t[num].name[i]].frame_mid;
		}
		cost = cost + w * t[num].weight[i] * (abs(mid1.p_x - mid2.p_x) + abs(mid1.p_y - mid2.p_y));
	}
	return cost;
}

void Transform(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list)
{
	for (int i = 0; i < soft_tile_list.size(); i++)	//soft_tile_list.size()
	{
		
		/*cout << soft_tile_list[i].t1->ti_name << " ";
		cout << soft_tile_list[i].t1->ti_ll.p_x << " ";
		cout << soft_tile_list[i].t1->ti_ll.p_y << endl;*/

		if (soft_tile_list[i].t2 != nullptr)
			continue;

		float cost = point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 0);
		vector<trans_point> p;
		trans_point temp;
		temp.cost = INT_MAX;
		temp.check_point = soft_tile_list[i].t1->ti_ll;
		temp.type = 5;
		p.push_back(temp);
		temp.type = 6;
		p.push_back(temp);

		temp.check_point = soft_tile_list[i].t1->ti_ur;
		temp.type = 1;
		p.push_back(temp);
		temp.type = 2;
		p.push_back(temp);

		temp.check_point.p_x = soft_tile_list[i].t1->ti_ll.p_x;
		temp.check_point.p_y = soft_tile_list[i].t1->ti_ur.p_y;
		temp.type = 7;
		p.push_back(temp);
		temp.type = 8;
		p.push_back(temp);

		temp.check_point.p_x = soft_tile_list[i].t1->ti_ur.p_x;
		temp.check_point.p_y = soft_tile_list[i].t1->ti_ll.p_y;
		temp.type = 3;
		p.push_back(temp);
		temp.type = 4;
		p.push_back(temp);

		for (int j = 0; j < p.size(); j++)
		{
			
			vector<Tile*> white;
			Enumerate(plane, white);
			if (p[j].type == 1 || p[j].type == 6)
			{
				p[j].area2 = CanUseArea2(white, p[j].check_point, INT_MAX);
			}
			else if (p[j].type == 2 || p[j].type == 5)
			{
				p[j].area2 = CanUseArea3(white, p[j].check_point, INT_MAX);
			}
			else if (p[j].type == 3 || p[j].type == 8)
			{
				p[j].area2 = CanUseArea1(white, p[j].check_point, INT_MAX);
			}
			else if (p[j].type == 4 || p[j].type == 7)
			{
				p[j].area2 = CanUseArea4(white, p[j].check_point, INT_MAX);
			}

			/*cout << j << endl;
			cout << p[j].type << endl;
			cout << p[j].area2.r_ll.p_x << " " << p[j].area2.r_ll.p_y << endl;
			cout << p[j].area2.r_ur.p_x << " " << p[j].area2.r_ur.p_y << endl;
			cout << "---------------------------" << endl;*/



			if (AREA(p[j].area2) == 0)
			{
				continue;
			}

			

			Point mid = soft_tile_list[i].t1->ti_mid;
			if (p[j].type == 1 || p[j].type == 8)
			{
				if (p[j].area2.GEO_WIDTH() >= WIDTH(soft_tile_list[i].t1))
				{
					p[j].tran = 0;
					for (int k = 0; k < p[j].area2.GEO_HEIGHT(); k++)
					{
						mid.p_y++;
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
						}
					}

					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y + p[j].frame_mid.p_y - soft_tile_list[i].t1->ti_mid.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y + p[j].frame_mid.p_y - soft_tile_list[i].t1->ti_mid.p_y;
				}
				else
				{
					p[j].tran = 1;
					//int up = floor(AREA(soft_tile_list[i].t1) / (4 * (WIDTH(soft_tile_list[i].t1) - p[j].area2.GEO_WIDTH())));

					float ratio = (float)WIDTH(soft_tile_list[i].t1) / (float)p[j].area2.GEO_WIDTH();
					int b = HEIGHT(soft_tile_list[i].t1);
					int d = 0;
					int best_b = HEIGHT(soft_tile_list[i].t1);
					int best_d = 0;
					while (1)
					{
						if (d > p[j].area2.GEO_HEIGHT()) break;
						if (b < 2) break;

						float real = b * WIDTH(soft_tile_list[i].t1) + d * p[j].area2.GEO_WIDTH();
						float frame = (b + d) * WIDTH(soft_tile_list[i].t1);
						if (real / frame < 0.8) break;

						float r = float(b + d) / float(WIDTH(soft_tile_list[i].t1));
						if (r > 2 || r < 0.5) break;

						Point mid(soft_tile_list[i].t1->ti_mid.p_x, p[j].check_point.p_y - b / 2 + d / 2);
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
							best_b = b;
							best_d = d;
						}

						b--;
						d = d + ceil(ratio);
					}
					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					p[j].area1.r_ll.p_y = p[j].check_point.p_y - best_b;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y;
					p[j].area2.r_ur.p_y = p[j].area2.r_ll.p_y + best_d;
				}
			}
			else if (p[j].type == 2 || p[j].type == 3)
			{
				if (p[j].area2.GEO_HEIGHT() >= HEIGHT(soft_tile_list[i].t1))
				{
					p[j].tran = 0;

					for (int k = 0; k < p[j].area2.GEO_WIDTH(); k++)
					{
						mid.p_x++;
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
						}
					}

					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x + p[j].frame_mid.p_x - soft_tile_list[i].t1->ti_mid.p_x;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x + p[j].frame_mid.p_x - soft_tile_list[i].t1->ti_mid.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y;
				}
				else
				{
					p[j].tran = 1;
					//int right = floor(AREA(soft_tile_list[i].t1) / (4 * (HEIGHT(soft_tile_list[i].t1) - p[j].area2.GEO_HEIGHT())));

					float ratio = (float)HEIGHT(soft_tile_list[i].t1) / (float)p[j].area2.GEO_HEIGHT();
					int a = WIDTH(soft_tile_list[i].t1);
					int c = 0;
					int best_a = WIDTH(soft_tile_list[i].t1);
					int best_c = 0;
					while (1)
					{
						if (c > p[j].area2.GEO_WIDTH()) break;
						if (a < 2) break;

						float real = c * HEIGHT(soft_tile_list[i].t1) + a * p[j].area2.GEO_HEIGHT();
						float frame = (a + c) * HEIGHT(soft_tile_list[i].t1);
						if (real / frame < 0.8) break;

						float r = float(a + c) / float(HEIGHT(soft_tile_list[i].t1));
						if (r > 2 || r < 0.5) break;

						Point mid(p[j].check_point.p_x - a / 2 + c / 2, soft_tile_list[i].t1->ti_mid.p_y);
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
							best_a = a;
							best_c = c;
						}

						a--;
						c = c + ceil(ratio);
					}
					p[j].area1.r_ll.p_x = p[j].check_point.p_x - best_a;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y;
					p[j].area2.r_ur.p_x = p[j].area2.r_ll.p_x + best_c;
				}
			}
			else if (p[j].type == 4 || p[j].type == 5)
			{
				if (p[j].area2.GEO_WIDTH() >= WIDTH(soft_tile_list[i].t1))
				{
					p[j].tran = 0;

					for (int k = 0; k < p[j].area2.GEO_HEIGHT(); k++)
					{
						mid.p_y--;
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
						}
					}

					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y + p[j].frame_mid.p_y - soft_tile_list[i].t1->ti_mid.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y + p[j].frame_mid.p_y - soft_tile_list[i].t1->ti_mid.p_y;
				}
				else
				{
					p[j].tran = 1;
					//int down = floor(AREA(soft_tile_list[i].t1) / (4 * (WIDTH(soft_tile_list[i].t1) - p[j].area2.GEO_WIDTH())));
					/*cout << "1 " << AREA(soft_tile_list[i].t1) << endl;
					cout << "2 " << WIDTH(soft_tile_list[i].t1) << endl;
					cout << "3 " << p[j].area2.GEO_WIDTH() << endl;
					cout << "down " << down << endl;*/

					float ratio = (float)WIDTH(soft_tile_list[i].t1) / (float)p[j].area2.GEO_WIDTH();
					int b = HEIGHT(soft_tile_list[i].t1);
					int d = 0;
					int best_b = HEIGHT(soft_tile_list[i].t1);
					int best_d = 0;
					while (1)
					{
						if (d > p[j].area2.GEO_HEIGHT()) break;
						if (b < 2) break;

						float real = b * WIDTH(soft_tile_list[i].t1) + d * p[j].area2.GEO_WIDTH();
						float frame = (b + d) * WIDTH(soft_tile_list[i].t1);
						if (real / frame < 0.8) break;

						float r = float(b + d) / float(WIDTH(soft_tile_list[i].t1));
						if (r > 2 || r < 0.5) break;

						Point mid(soft_tile_list[i].t1->ti_mid.p_x, p[j].check_point.p_y + b / 2 - d / 2);
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
							best_b = b;
							best_d = d;
						}

						b--;
						d = d + ceil(ratio);
					}
					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ll.p_y + best_b;
					p[j].area2.r_ll.p_y = p[j].area2.r_ur.p_y - best_d;
				}
			}
			else if (p[j].type == 6 || p[j].type == 7)
			{
				if (p[j].area2.GEO_HEIGHT() >= HEIGHT(soft_tile_list[i].t1))
				{
					p[j].tran = 0;
					int limit = 99;
					int space = 1;
					if (p[j].area2.GEO_WIDTH() <= 99)
					{
						limit = p[j].area2.GEO_WIDTH();
					}
					else
					{
						space = floor((double)p[j].area2.GEO_WIDTH() / (double)100);
					}

					for (int k = 0; k < p[j].area2.GEO_WIDTH(); k++)
					{
						mid.p_x--;
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
						}
					}

					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x + p[j].frame_mid.p_x - soft_tile_list[i].t1->ti_mid.p_x;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x + p[j].frame_mid.p_x - soft_tile_list[i].t1->ti_mid.p_x;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y;
				}
				else
				{
					p[j].tran = 1;
					//int left = floor(AREA(soft_tile_list[i].t1) / (4 * (HEIGHT(soft_tile_list[i].t1) - p[j].area2.GEO_HEIGHT())));

					float ratio = (float)HEIGHT(soft_tile_list[i].t1) / (float)p[j].area2.GEO_HEIGHT();
					int a = WIDTH(soft_tile_list[i].t1);
					int c = 0;
					int best_a = WIDTH(soft_tile_list[i].t1);
					int best_c = 0;
					while (1)
					{
						if (c > p[j].area2.GEO_WIDTH()) break;
						if (a < 2) break;

						float real = c * HEIGHT(soft_tile_list[i].t1) + a * p[j].area2.GEO_HEIGHT();
						float frame = (a + c) * HEIGHT(soft_tile_list[i].t1);
						if (real / frame < 0.8) break;

						float r = float(a + c) / float(HEIGHT(soft_tile_list[i].t1));
						if (r > 2 || r < 0.5) break;

						Point mid(p[j].check_point.p_x + a / 2 - c / 2, soft_tile_list[i].t1->ti_mid.p_y);
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 0);

						if (newcost < p[j].cost)
						{
							p[j].cost = newcost;
							p[j].frame_mid = mid;
							best_a = a;
							best_c = c;
						}

						a--;
						c = c + ceil(ratio);
					}
					p[j].area1.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					p[j].area1.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y;
					p[j].area1.r_ur.p_x = soft_tile_list[i].t1->ti_ll.p_x + best_a;
					p[j].area1.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y;
					p[j].area2.r_ll.p_x = p[j].area2.r_ur.p_x - best_c;
				}
			}
		}

		for (int j = 0; j < 8; j++)
		{
			if (p[j].area2.GEO_HEIGHT() == 0 || p[j].area2.GEO_WIDTH() == 0 ||
				p[j].area1.GEO_HEIGHT() == 0 || p[j].area1.GEO_WIDTH() == 0)
			{
				p[j].cost = INT_MAX;
			}
		}

		for (int j = 1; j <= 7; j++)
		{
			trans_point key = p[j];
			int k = j - 1;

			while (k >= 0 && p[k].cost > key.cost)
			{
				p[k + 1] = p[k];
				k--;
			}
			p[k + 1] = key;
		}

		/*if (i == 5)
		{
			cout << "-------------------" << endl;
			cout << "precost " << cost << endl;
			cout << p[0].type << " " << p[0].cost << endl;
			cout << "area1" << endl;
			cout << p[0].area1.r_ll.p_x << " " << p[0].area1.r_ll.p_y << endl;
			cout << p[0].area1.r_ur.p_x << " " << p[0].area1.r_ur.p_y << endl;
			cout << "area2" << endl;
			cout << p[0].area2.r_ll.p_x << " " << p[0].area2.r_ll.p_y << endl;
			cout << p[0].area2.r_ur.p_x << " " << p[0].area2.r_ur.p_y << endl;
			cout << "start" << endl;
			cout << p[0].check_point.p_x << " " << p[0].check_point.p_y << endl;
			cout << "-------------------" << endl;
		}*/



		for (int j = 0; j < 8; j++)
		{
			if (p[j].cost >= cost)
				break;
			//cout << p[j].type << " " << p[j].cost << " ";
			//cout << p[j].frame_mid.p_x << " " << p[j].frame_mid.p_y << endl;

			soft_tile_list[i].frame_mid = p[j].frame_mid;
			soft_tile_list[i].dir = p[j].type;
			RemoveTile_tran(soft_tile_list[i].t1, plane);

			if (p[j].tran == 0)
			{
				soft_tile_list[i].t1 = InsertFixedTile(p[j].area1, plane);
				break;
			}
			else
			{
				soft_tile_list[i].t1 = InsertFixedTile(p[j].area1, plane);
				soft_tile_list[i].t2 = InsertFixedTile(p[j].area2, plane);
				break;
			}
		}

		/*cout << p[0].area1.r_ll.p_x << " " << p[0].area1.r_ll.p_y << endl;
		cout << p[0].area1.r_ur.p_x << " " << p[0].area1.r_ur.p_y << endl;
		cout << p[0].area2.r_ll.p_x << " " << p[0].area2.r_ll.p_y << endl;
		cout << p[0].area2.r_ur.p_x << " " << p[0].area2.r_ur.p_y << endl;*/
		//cout << "--------------------------------" << endl;

	}
}




Plane :: ~Plane()
{
	delete this->pl_left;
	this->pl_left = NULL;
	delete this->pl_right;
	this->pl_right = NULL;
	delete this->pl_top;
	this->pl_top = NULL;
	delete this->pl_bottom;
	this->pl_bottom = NULL;
	
}



