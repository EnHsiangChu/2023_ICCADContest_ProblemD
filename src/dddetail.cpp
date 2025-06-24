#include <iostream>
#include <queue>
#include <cmath>
#include <string>
#include <fstream>
#include <utility>
#include <algorithm>
#include <fstream>
#include <limits>
#include <map>
#include <set>

#include "dddetail.h"

using namespace std;

bool compare_length(const pair<Rect, int> &pair1, const pair<Rect, int> &pair2)
{
	return pair1.second <= pair2.second;
}

bool gene_compare(const pair<vector<Tile*>, double> &pair1, const pair<vector<Tile*>, double> &pair2)
{
	return pair1.second <= pair2.second;
}


void plot_matlab(Plane* plane, string file_name)
{
	vector<Tile*> white;
	Enumerate(plane, white);

	ofstream newFile;
	newFile.open(file_name);
	newFile << "axis equal;\n" << "hold on;\n" << "grid on;\n";
	newFile << "block_x=[0 0 " << plane->pl_width << " " << plane->pl_width << " 0];" << endl;
	newFile << "block_y=[0 " << plane->pl_height << " " << plane->pl_height << " 0 0];" << endl;
	newFile << "fill(block_x, block_y, 'c');" << endl;

	for (int i = 0; i < white.size(); i++)
	{
		newFile << "block_x=[" << LEFT(white[i]) << " " << LEFT(white[i]) << " ";
		newFile << RIGHT(white[i]) << " " << RIGHT(white[i]) << " " << LEFT(white[i]) << "];" << endl;
		newFile << "block_y=[" << BOTTOM(white[i]) << " " << TOP(white[i]) << " ";
		newFile << TOP(white[i]) << " " << BOTTOM(white[i]) << " " << BOTTOM(white[i]) << "];" << endl;
		newFile << "fill(block_x, block_y, 'w');" << endl;
	}

	for (int i = 0; i < plane->fixed_tile_list.size(); i++)
	{
		newFile << "block_x=[" << LEFT(plane->fixed_tile_list[i]) << " " << LEFT(plane->fixed_tile_list[i]) << " ";
		newFile << RIGHT(plane->fixed_tile_list[i]) << " " << RIGHT(plane->fixed_tile_list[i]) << " " << LEFT(plane->fixed_tile_list[i]) << "];" << endl;
		newFile << "block_y=[" << BOTTOM(plane->fixed_tile_list[i]) << " " << TOP(plane->fixed_tile_list[i]) << " ";
		newFile << TOP(plane->fixed_tile_list[i]) << " " << BOTTOM(plane->fixed_tile_list[i]) << " " << BOTTOM(plane->fixed_tile_list[i]) << "];" << endl;
		newFile << "fill(block_x, block_y, 'y');" << endl;
	}

	if (plane->legal) {
		for (int i = 0; i < plane->soft_tile_list.size(); i++) //soft_tile_list.size() 5
		{
			newFile << "block_x=[" << LEFT(plane->soft_tile_list[i]) << " " << LEFT(plane->soft_tile_list[i]) << " ";
			newFile << RIGHT(plane->soft_tile_list[i]) << " " << RIGHT(plane->soft_tile_list[i]) << " " << LEFT(plane->soft_tile_list[i]) << "];" << endl;
			newFile << "block_y=[" << BOTTOM(plane->soft_tile_list[i]) << " " << TOP(plane->soft_tile_list[i]) << " ";
			newFile << TOP(plane->soft_tile_list[i]) << " " << BOTTOM(plane->soft_tile_list[i]) << " " << BOTTOM(plane->soft_tile_list[i]) << "];" << endl;
			newFile << "fill(block_x, block_y, 'g');" << endl;
			newFile << "text(" << plane->soft_tile_list[i]->ti_ll.p_x << "," << plane->soft_tile_list[i]->ti_mid.p_y << ",'" << plane->soft_tile_list[i]->ti_name << "')" << endl;
		}
	}
}

int RemoveTile(Tile*& tile, Plane* &plane)
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

	int index;
	for (int i = 0; i < plane->soft_tile_list.size(); i++)
	{
		if (plane->soft_tile_list[i]->ti_body == 0)
		{
			index = i;
			plane->soft_tile_list.erase(plane->soft_tile_list.begin() + i);
			break;
		}
	}



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

	return index;
}

void Insert_Removed(Tile * tile, Plane * plane, int index)
{
	Rect tmp_r(tile->ti_ll, tile->ti_ur);
	Tile* tmp_t = InsertFixedTile(tmp_r, plane);
	tmp_t->ti_name = tile->ti_name;
	tmp_t->ti_body = 2;
	
	plane->soft_tile_list.insert(plane->soft_tile_list.begin() + index, tmp_t);

	delete tile;
	return;
}


//exchange algorithm
int ExchangeSoftTile(Tile* &tile1, Tile* &tile2, Plane *& plane)
{

	Tile tp1, tp2;
	tp1.ti_ll = tile1->ti_ll; tp1.ti_ur = tile1->ti_ur;
	tp2.ti_ll = tile2->ti_ll; tp2.ti_ur = tile2->ti_ur;
	

	string ti_name1 = tile1->ti_name;
	string ti_name2 = tile2->ti_name;

	
	int init_hpwl = calc_hpwl(plane);


	int index1 = RemoveTile(tile1, plane);
	
	Tile* tmp = TiSrPoint(plane->fixed_tile_list[0], plane, tp2.ti_ll);
	int index2 = RemoveTile(tmp, plane);
	
	
	Tile * white_for2 = TiSrPoint(plane->fixed_tile_list[0], plane, tp1.ti_ll);
	Tile * white_for1 = TiSrPoint(plane->fixed_tile_list[0], plane, tp2.ti_ll);
	

	vector<Tile*> white;
	Enumerate(plane, white);

	pair<Rect, bool> insert_position1[4];
	pair<Rect, bool> insert_position2[4];

	Rect best_position1;
	Rect best_position2;
	double best_hpwl = -1;
	
	double exchange_hpwl;

	
	for (int i = 0; i < 4; i++)
	{
		insert_position1[i] = find_insert_position(plane, &tp1, white_for1, i + 1);
		insert_position2[i] = find_insert_position(plane, &tp2, white_for2, i + 1);
	}
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (insert_position1[i].second == false || insert_position2[j].second == false)
				continue;

			if (overlap(insert_position1[i].first, insert_position2[j].first))
				continue;
			
			Point mid1((insert_position1[i].first.r_ll.p_x + insert_position1[i].first.r_ur.p_x) / 2
				, (insert_position1[i].first.r_ll.p_y + insert_position1[i].first.r_ur.p_y) / 2);
			Point mid2((insert_position2[j].first.r_ll.p_x + insert_position2[j].first.r_ur.p_x) / 2
				, (insert_position2[j].first.r_ll.p_y + insert_position2[j].first.r_ur.p_y) / 2);


			exchange_hpwl = calc_exchange_hpwl(plane, ti_name1, mid1, ti_name2, mid2);
			
			if (exchange_hpwl >= init_hpwl)
				continue;

			best_hpwl = exchange_hpwl;
			best_position1 = insert_position1[i].first;
			best_position2 = insert_position2[j].first;
		}
	}


	if (best_hpwl == -1)//can't insert
	{	
		Rect tmp_rect2(tp2.ti_ll,tp2.ti_ur);
		Tile* tmp2 = InsertFixedTile(tmp_rect2, plane);
		tmp2->ti_name = ti_name2;
		tmp2->ti_body = 2;
		plane->soft_tile_list.insert(plane->soft_tile_list.begin() + index2, tmp2);

		Rect tmp_rect1(tp1.ti_ll, tp1.ti_ur);
		Tile* tmp1 = InsertFixedTile(tmp_rect1, plane);
		tmp1->ti_name = ti_name1;
		tmp1->ti_body = 2;
		plane->soft_tile_list.insert(plane->soft_tile_list.begin() + index1, tmp1);

		return 0;
	}
	

	int improvement = init_hpwl - best_hpwl;
	cout << "exchange " << ti_name1 << " & " << ti_name2 << ", reduced hpwl:" << improvement << endl;

	
	Tile* tp_tile2 = InsertFixedTile(best_position2, plane);
	tp_tile2->ti_name = ti_name2;
	tp_tile2->ti_body = 2;
	plane->soft_tile_list.insert(plane->soft_tile_list.begin() + index2, tp_tile2);


	Tile* tp_tile1 = InsertFixedTile(best_position1, plane);
	tp_tile1->ti_name = ti_name1;
	tp_tile1->ti_body = 2;
	plane->soft_tile_list.insert(plane->soft_tile_list.begin() + index1, tp_tile1);
	
	
	return improvement;
}

int calc_hpwl(Plane * &plane)
{
	double hpwl = 0;
	
	ifstream infile(case_file_name);
	string buf1, buf2, buf3;

	while (infile >> buf1)
	{
		if (buf1 == "CONNECTION")
		{
			infile >> buf1;
			break;
		}
	}

	
	while (infile >> buf1 >> buf2 >> buf3)
	{
		Point p1, p2;
		for (int i = 0; i < plane->soft_tile_list.size(); i++)
		{
			Tile* tp = plane->soft_tile_list[i];
			if (tp->ti_name == buf1)
			{
				p1.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p1.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
			else if (tp->ti_name == buf2)
			{
				p2.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p2.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
		}
		for (int i = 0; i < plane->fixed_tile_list.size(); i++)
		{
			Tile* tp = plane->fixed_tile_list[i];
			if (tp->ti_name == buf1)
			{
				p1.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p1.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
			else if (tp->ti_name == buf2)
			{
				p2.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p2.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
		}
		
		hpwl += (abs(p1.p_x - p2.p_x) + abs(p1.p_y - p2.p_y)) * stod(buf3);
	}
	
	infile.close();

	return hpwl;
}

bool shrink_area(Rect &area, Tile * tile, int type)
{
	bool can_insert = false;

	if (type == 0)
	{
		if (AREA(area) >= AREA(tile))
		{
			if (area.GEO_HEIGHT() >= HEIGHT(tile) && area.GEO_WIDTH() >= WIDTH(tile))
			{
				area.r_ur.p_x = area.r_ll.p_x + WIDTH(tile);
				area.r_ur.p_y = area.r_ll.p_y + HEIGHT(tile);
				can_insert = true;
			}
			else
			{
				if (area.GEO_HEIGHT() >= area.GEO_WIDTH())
				{
					if (AREA(tile) < 2 * area.GEO_WIDTH() * area.GEO_WIDTH())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_WIDTH());
						area.r_ur.p_y = area.r_ll.p_y + ceil(new_len);
						can_insert = true;
					}
				}
				else
				{
					if (AREA(tile) < 2 * area.GEO_HEIGHT() * area.GEO_HEIGHT())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_HEIGHT());
						area.r_ur.p_x = area.r_ll.p_x + ceil(new_len);
						can_insert = true;
					}
				}
			}
		}
	}
	else if (type == 1)
	{
		if (AREA(area) >= AREA(tile))
		{
			if (area.GEO_HEIGHT() >= HEIGHT(tile) && area.GEO_WIDTH() >= WIDTH(tile))
			{
				area.r_ll.p_x = area.r_ur.p_x - WIDTH(tile);
				area.r_ur.p_y = area.r_ll.p_y + HEIGHT(tile);
				can_insert = true;
			}
			else
			{
				if (area.GEO_HEIGHT() >= area.GEO_WIDTH())
				{
					if (AREA(tile) < 2 * area.GEO_WIDTH() * area.GEO_WIDTH())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_WIDTH());
						area.r_ur.p_y = area.r_ll.p_y + ceil(new_len);
						can_insert = true;
					}
				}
				else
				{
					if (AREA(tile) < 2 * area.GEO_HEIGHT() * area.GEO_HEIGHT())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_HEIGHT());
						area.r_ll.p_x = area.r_ur.p_x - ceil(new_len);
						can_insert = true;
					}
				}
			}
		}
	}
	else if (type == 2)
	{
		if (AREA(area) >= AREA(tile))
		{
			if (area.GEO_HEIGHT() >= HEIGHT(tile) && area.GEO_WIDTH() >= WIDTH(tile))
			{
				area.r_ll.p_y = area.r_ur.p_y - HEIGHT(tile);
				area.r_ur.p_x = area.r_ll.p_x + WIDTH(tile);
				can_insert = true;
			}
			else
			{
				if (area.GEO_HEIGHT() >= area.GEO_WIDTH())
				{
					if (AREA(tile) < 2 * area.GEO_WIDTH() * area.GEO_WIDTH())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_WIDTH());
						area.r_ll.p_y = area.r_ur.p_y - ceil(new_len);
						can_insert = true;
					}
				}
				else
				{
					if (AREA(tile) < 2 * area.GEO_HEIGHT() * area.GEO_HEIGHT())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_HEIGHT());
						area.r_ur.p_x = area.r_ll.p_x + ceil(new_len);
						can_insert = true;
					}
				}
			}
		}
	}
	else if (type == 3)
	{
		if (AREA(area) >= AREA(tile))
		{
			if (area.GEO_HEIGHT() >= HEIGHT(tile) && area.GEO_WIDTH() >= WIDTH(tile))
			{
				area.r_ll.p_y = area.r_ur.p_y - HEIGHT(tile);
				area.r_ll.p_x = area.r_ur.p_x - WIDTH(tile);
				can_insert = true;
			}
			else
			{
				if (area.GEO_HEIGHT() >= area.GEO_WIDTH())
				{
					if (AREA(tile) < 2 * area.GEO_WIDTH() * area.GEO_WIDTH())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_WIDTH());
						area.r_ll.p_y = area.r_ur.p_y - ceil(new_len);
						can_insert = true;
					}
				}
				else
				{
					if (AREA(tile) < 2 * area.GEO_HEIGHT() * area.GEO_HEIGHT())
					{
						float new_len = float(AREA(tile)) / float(area.GEO_HEIGHT());
						area.r_ll.p_x = area.r_ur.p_x - ceil(new_len);
						can_insert = true;
					}
				}
			}
		}
	}


	return can_insert;
}

tuple<int,int> find_small_rect(Rect large, int small_area)
{
	Point ur;
	int side_length1 = ceil(sqrt(small_area));
	int side_length2;

	int height = large.GEO_HEIGHT();
	int width = large.GEO_WIDTH();

	if (height > width)
	{
		if (side_length1 > width)
		{
			ur.p_x = large.r_ur.p_x;
			side_length2 = ceil(float(small_area) / float(width));
			if (side_length2 > height)
			{
				ur.p_y = large.r_ur.p_y;
			}
			else
			{
				ur.p_y = large.r_ll.p_y + side_length2;
			}
		}
		else
		{
			ur.p_x = large.r_ll.p_x + side_length1;
			ur.p_y = large.r_ll.p_y + side_length1;
		}
	}
	else
	{
		if (side_length1 > height)
		{
			ur.p_y = large.r_ur.p_y;
			side_length2 = ceil(float(small_area) / float(height));
			if (side_length2 > width)
			{
				ur.p_x = large.r_ur.p_x;
			}
			else
			{
				ur.p_x = large.r_ll.p_x + side_length2;
			}
		}
		else
		{
			ur.p_x = large.r_ll.p_x + side_length1;
			ur.p_y = large.r_ll.p_y + side_length1;
		}
	}


	return make_tuple(ur.p_x, ur.p_y);
}

double calc_exchange_hpwl(Plane* plane, string nlarge, Point plarge, string nsmall, Point psmall)
{
	double hpwl = 0;

	ifstream infile(case_file_name);
	string buf1, buf2, buf3;

	while (infile >> buf1)
	{
		if (buf1 == "CONNECTION")
		{
			infile >> buf1;
			break;
		}
	}


	while (infile >> buf1 >> buf2 >> buf3)
	{
		Point p1, p2;

		if (buf1 == nlarge)
		{
			p1.p_x = plarge.p_x;
			p1.p_y = plarge.p_y;
		}
		if (buf1 == nsmall)
		{
			p1.p_x = psmall.p_x;
			p1.p_y = psmall.p_y;
		}
		if (buf2 == nlarge)
		{
			p2.p_x = plarge.p_x;
			p2.p_y = plarge.p_y;
		}
		if (buf2 == nsmall)
		{
			p2.p_x = psmall.p_x;
			p2.p_y = psmall.p_y;
		}

		for (int i = 0; i < plane->soft_tile_list.size(); i++)
		{
			Tile* tp = plane->soft_tile_list[i];

			if (tp->ti_name == nlarge || tp->ti_name == nsmall)
				continue;

			if (tp->ti_name == buf1)
			{
				p1.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p1.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
			else if (tp->ti_name == buf2)
			{
				p2.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p2.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
		}


		for (int i = 0; i < plane->fixed_tile_list.size(); i++)
		{
			Tile* tp = plane->fixed_tile_list[i];

			if (tp->ti_name == nlarge || tp->ti_name == nsmall)
				continue;


			if (tp->ti_name == buf1)
			{
				p1.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p1.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
			else if (tp->ti_name == buf2)
			{
				p2.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p2.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
		}

		hpwl += (abs(p1.p_x - p2.p_x) + abs(p1.p_y - p2.p_y)) * stod(buf3);
	}

	infile.close();


	return hpwl;
}

pair<Rect, bool> find_insert_position(Plane * plane, Tile * target, Tile* white_tile, int type)
{
	pair<Rect, bool> position;
	vector<Tile*> white_list;

	Enumerate(plane, white_list);


	// 1:lb	2:rb 3:lu 4:ru
	if (type == 1)
	{
		Point p(white_tile->ti_ll);
		position.first = CanUseArea1(white_list, p, TileArea(target));
		position.second = shrink_area(position.first, target, type);
	}
	else if (type == 2)
	{
		Point p(white_tile->ti_ur.p_x, white_tile->ti_ll.p_y);
		position.first = CanUseArea2(white_list, p, TileArea(target));
		position.second = shrink_area(position.first, target, type);
	}
	else if (type == 3)
	{
		Point p(white_tile->ti_ll.p_x, white_tile->ti_ur.p_y);
		position.first = CanUseArea3(white_list, p, TileArea(target));
		position.second = shrink_area(position.first, target, type);
	}
	else if (type == 4)
	{
		Point p(white_tile->ti_ur);
		position.first = CanUseArea4(white_list, p, TileArea(target));
		position.second = shrink_area(position.first, target, type);
	}

	return position;
}

bool overlap(const Rect &r1, const Rect &r2)
{
	bool noOverlap = r1.r_ll.p_x > r2.r_ur.p_x ||
		r2.r_ll.p_x > r1.r_ur.p_x ||
		r1.r_ll.p_y > r2.r_ur.p_y ||
		r2.r_ll.p_y > r1.r_ur.p_y;

	return !noOverlap;
}


//re-insert algorithm
int reinsert(Plane * plane, Tile* target)
{
	int area = TileArea(target);
	int init_hpwl = calc_hpwl(plane);
	Tile* tp_target = duplicate_tile(target);


	int index = RemoveTile(target, plane);
	

	vector<Tile*> white_list;
	Enumerate(plane, white_list);

	int reinsert_hpwl;
	Rect rect_insert;
	int min_hpwl = -1;
	for (int i = 0; i < white_list.size(); i++)
	{
		
		Tile* tp = white_list[i];

		// 1:lb	2:rb 3:lu 4:ru
		Point p1(tp->ti_ll);
		Point p2(tp->ti_ur.p_x, tp->ti_ll.p_y);
		Point p3(tp->ti_ll.p_x, tp->ti_ur.p_y);
		Point p4(tp->ti_ur);

		Rect r[4];
		r[0] = CanUseArea1(white_list, p1, area);
		r[1] = CanUseArea2(white_list, p2, area);
		r[2] = CanUseArea3(white_list, p3, area);
		r[3] = CanUseArea4(white_list, p4, area);
		
		bool can_insert[4];
		

		for (int j = 0; j < 4; j++)
		{
			
			
			can_insert[j] = shrink_area(r[j], tp_target, j);
			
			if (can_insert[j])
			{
				Point tmp_p((r[j].r_ur.p_x + r[j].r_ll.p_x) / 2,
					(r[j].r_ur.p_y + r[j].r_ll.p_y) / 2);
			
				reinsert_hpwl = calc_reinsert_hpwl(plane, tp_target->ti_name, tmp_p);

				if (reinsert_hpwl >= init_hpwl)	can_insert[j] = false;
			}

			if (can_insert[j] && min_hpwl > reinsert_hpwl)
			{
				rect_insert = r[j];
				min_hpwl = reinsert_hpwl;
			}
		}
	}

	

	if (min_hpwl == -1)
	{
		
		Insert_Removed(tp_target, plane, index);
		return 0;
	}
	

	cout << "reinset " << tp_target->ti_name << ", reduced hpwl:" << init_hpwl - min_hpwl << endl;

	tp_target->ti_ll = rect_insert.r_ll;
	tp_target->ti_ur = rect_insert.r_ur;

	Insert_Removed(tp_target, plane, index);


	return 0;
}

int calc_reinsert_hpwl(Plane * plane, string ti_name, Point p)
{
	double hpwl = 0;

	ifstream infile(case_file_name);
	string buf1, buf2, buf3;

	while (infile >> buf1)
	{
		if (buf1 == "CONNECTION")
		{
			infile >> buf1;
			break;
		}
	}


	while (infile >> buf1 >> buf2 >> buf3)
	{
		Point p1, p2;

		if (buf1 == ti_name)
		{
			p1.p_x = p.p_x;
			p1.p_y = p.p_y;
		}
		if (buf2 == ti_name)
		{
			p2.p_x = p.p_x;
			p2.p_y = p.p_y;
		}

		for (int i = 0; i < plane->soft_tile_list.size(); i++)
		{
			Tile* tp = plane->soft_tile_list[i];

			if (tp->ti_name == ti_name)
				continue;

			if (tp->ti_name == buf1)
			{
				p1.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p1.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
			else if (tp->ti_name == buf2)
			{
				p2.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p2.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
		}


		for (int i = 0; i < plane->fixed_tile_list.size(); i++)
		{
			Tile* tp = plane->fixed_tile_list[i];


			if (tp->ti_name == buf1)
			{
				p1.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p1.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
			else if (tp->ti_name == buf2)
			{
				p2.p_x = 0.5 * (tp->ti_ll.p_x + tp->ti_ur.p_x);
				p2.p_y = 0.5 * (tp->ti_ll.p_y + tp->ti_ur.p_y);
			}
		}

		hpwl += (abs(p1.p_x - p2.p_x) + abs(p1.p_y - p2.p_y)) * stod(buf3);
	}

	infile.close();


	return hpwl;
}

Tile * duplicate_tile(Tile * tile)
{
	Tile* t = new Tile;
	t->ti_ll = tile->ti_ll;
	t->ti_ur = tile->ti_ur;
	t->ti_mid = tile->ti_mid;
	t->ti_name = tile->ti_name;
	t->ti_body = tile->ti_body;
	t->name.clear();
	for (int i = 0; i < tile->name.size(); i++)
	{
		t->name.push_back(tile->name[i]);
	}
	t->weight.clear();
	for (int i = 0; i < tile->weight.size(); i++)
	{
		t->weight.push_back(tile->weight[i]);
	}
	//t->name = tile->name;
	//t->weight = tile->weight;
	
	

	return t;
}




//genetic algorthm
vector<Tile*> generate_candidate(Plane* plane)
{
	vector<Tile*> candidate;

	for (int i = 0; i < plane->soft_tile_list.size(); i++)
	{
		candidate.push_back(duplicate_tile(plane->soft_tile_list[i]));
	}

	return candidate;
}

void sort_population(vector<pair<vector<Tile*>, double>>& population)
{
	sort(population.begin(), population.end(), gene_compare);
}

void crossover(vector<Tile*> v1, vector<Tile*> v2, vector<Tile*> &soft_list)
{
	set<string> inserted_already;

	int count = 0;
	for (int i = 0; i < v1.size() / 2; i++)
	{
		copy_data_tile(v1[i], soft_list[i]);
		inserted_already.insert(v1[i]->ti_name);
		count++;
	}

	
	for (int i = 0; i < v2.size(); i++)
	{
		if (inserted_already.count(v2[i]->ti_name)) continue;

		copy_data_tile(v2[i], soft_list[count]);
		count++;
	}
}

void mutation(vector<Tile*> v, vector<Tile*>& soft_list)
{
	for (int i = 0; i < v.size(); i++)
	{
		copy_data_tile(v[i], soft_list[i]);
	}

	int mutate_num = soft_list.size() / 10;
	if (mutate_num < 3)
	{
		mutate_num = 3;
	}

	vector<int> mutate_index;
	set<int> s;
	while (mutate_index.size() != mutate_num)
	{
		int index = rand() % soft_list.size();
		if (s.count(index)) continue;

		mutate_index.push_back(index);
		s.insert(index);
	}

	for (int i = 0; i < mutate_index.size() - 1; i++)
	{
		swap(soft_list[mutate_index[i]], soft_list[mutate_index[i + 1]]);
	}

}

void shrink_population(int individual_num, vector<pair<vector<Tile*>, double>>& population)
{
	for (int i = 1 + individual_num; i < population.size(); i++)
	{
		for (int j = 0; j < population[i].first.size(); j++)
		{
			delete population[i].first[j];
		}
	}

	population.resize(individual_num);
	population.shrink_to_fit();
}

void copy_data_tile(Tile * t, Tile * &target)
{
	target->ti_ll = t->ti_ll;
	target->ti_ur = t->ti_ur;
	target->ti_mid = t->ti_mid;
	target->ti_name = t->ti_name;
	target->ti_body = t->ti_body;

	target->name.clear();
	for (int i = 0; i < t->name.size(); i++)
	{
		target->name.push_back(t->name[i]);
	}
	target->weight.clear();
	for (int i = 0; i < t->weight.size(); i++)
	{
		target->weight.push_back(t->weight[i]);
	}
	
	//target->name = t->name;
	//target->weight = t->weight;
}

bool is_legal(Plane * plane)
{
	return false;
}


/*void gene_arrange_order(Plane * plane, pair<vector<string>, double> order)
{
	map<string, int> m;

	int soft_num = order.first.size();

	for (int i = 0; i < soft_num; i++)
	{
		m[plane->soft_tile_list[i]->ti_name] = i;
	}

	for (int i = 0; i < soft_num; i++)
	{
		string s = plane->soft_tile_list[i]->ti_name;
		swap(plane->soft_tile_list[m[order.first[i]]], plane->soft_tile_list[i]);
		swap(m[order.first[i]], m[s]);
	}

}*/
