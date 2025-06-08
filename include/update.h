#ifndef _UPDATE_H_
#define _UPDATE_H_
#include "structure.h"
using namespace std;

Tile* TiSplitX(Tile* tile, int x);
Tile* TiSplitY(Tile* tile, int y);

void TiJoinX(Tile* tile1, Tile* tile2, Plane* plane);
void TiJoinY(Tile* tile1, Tile* tile2, Plane* plane);

Tile* SplitAndMerge(Tile* tile, Tile* target, Plane* plane, Rect rect);
int DownMerge(Tile* tile, Plane* plane);
static bool canMergeVertical(const Tile* tile1, const Tile* tile2);
static bool canMergeHorizontal(Tile* tile1, Tile* tile2);
Tile* InsertFixedTile(Rect rect, Plane* plane);

void Enumerate(Plane* plane, vector<Tile*>& white);

Rect CanUseArea1(vector<Tile*> white, Point start, int target);
Rect CanUseArea2(vector<Tile*> white, Point start, int target);
Rect CanUseArea3(vector<Tile*> white, Point start, int target);
Rect CanUseArea4(vector<Tile*> white, Point start, int target);

float point_cost(Tile* tp, Point mid);
float point_cost_tran(vector<Trantile> t, int num, vector<Tile*> fixed_tile_list, Point mid1, int mode);

int mini_dis(Tile* tile, int chip_width, int chip_height);
int length(Tile* soft_tile, int soft_x, int soft_y);
int wire_length(Tile* soft_tile, Tile* white_tile);
int manhattan(Tile* soft_tile, Tile* white_tile);
void sort_white_tile_order(vector<Tile*>& white_tile_list, Tile* soft_tile);
void sort_area(int start, int end, vector<Tile*>& soft_tile_list);
void sort_x(int start, int end, vector<Tile*>& soft_tile_list);
int insert_order(vector<Tile*>& soft_tile_list);
void InsertSoftTile(Plane* plane, vector<Tile*>& soft_tile_list, int type1);
void RemoveTile(Tile*& tile, Plane*& plane);
void Replace(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list);
void Special_Transform(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list);
//void Transform2(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list);
void Transform(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list);
void ForceDirected(Plane* plane, vector<Tile*>& soft_tile_list, vector<Tile*> fixed_tile_list);
void ForceDirected2(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list);


#endif _UPDATE_H_
