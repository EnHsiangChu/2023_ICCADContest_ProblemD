#ifndef _DETAIL_H_
#define _DETAIL_H_

#include <tuple>

#include "structure.h"
#include "update.h"
#include "search.h"

using namespace std;

void plot_matlab(Plane* plane, string file_name);
int RemoveTile(Tile*& tile, Plane*& plane);
void Insert_Removed(Tile* tile, Plane* plane, int index);
int ExchangeSoftTile(Tile* &tile1, Tile* &tile2, Plane* &plane);
int calc_hpwl(Plane* &plane);
bool shrink_area(Rect &area, Tile* tile, int type);
tuple<int,int> find_small_rect(Rect large, int small_area);
double calc_exchange_hpwl(Plane* plane, string nlarge, Point plarge, string nsmall, Point psmall);
int reinsert(Plane* plane, Tile* target);
int calc_reinsert_hpwl(Plane* plane, string ti_name, Point p);
Tile* duplicate_tile(Tile* tile);
pair<Rect, bool> find_insert_position(Plane * plane, Tile * target, Tile* white_tile, int type);
bool overlap(const Rect &r1, const Rect &r2);


vector<Tile*> generate_candidate(Plane* plane);
void sort_population(vector<pair<vector<Tile*>, double>>& population);
void crossover(vector<Tile*> v1, vector<Tile*> v2, vector<Tile*> &soft_list);
void mutation(vector<Tile*> v, vector<Tile*> &soft_list);
void shrink_population(int individual_num, vector<pair<vector<Tile*>, double>>& population);

void copy_data_tile(Tile* t, Tile* &target);

bool is_legal(Plane* plane);

bool compare_length(const pair<Rect, int> &pair1, const pair<Rect, int> &pair2);
bool gene_compare(const pair<vector<Tile*>, double> &pair1, const pair<vector<Tile*>, double> &pair2);

#endif _DETAIL_H_