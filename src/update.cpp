#include <iostream>
#include <queue>
#include <cmath>
#include <fstream>
#include "update.h"
#include "structure.h"
#include "search.h"
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
	Rect area3;
	int tran;
};

static bool canMergeVertical(Tile* tile1, Tile* tile2) 
{
	if (tile1->ti_body != 0 || tile2->ti_body != 0)
		return false;

	if (LEFT(tile1) != LEFT(tile2) || RIGHT(tile1) != RIGHT(tile2))
		return false;

	return true;
}

static bool canMergeHorizontal(Tile* tile1, Tile* tile2) 
{
	if (tile1->ti_body != 0 || tile2->ti_body != 0)
		return false;

	if (TOP(tile1) != TOP(tile2) || BOTTOM(tile1) != BOTTOM(tile2))
		return false;

	return true;
}

int DownMerge(Tile* tile, Plane* plane) {
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

	Tile* target = new Tile();
	target = NULL;
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

	target->ti_body = 1;

	vector<Tile*> white_tile_list;
	Enumerate(plane, white_tile_list);
	for (int i = 0; i < white_tile_list.size(); i++)
	{
		if (DownMerge(white_tile_list[i], plane)) {
			i = 0;
			Enumerate(plane, white_tile_list);
		}

		if (canMergeHorizontal(white_tile_list[i], BL(white_tile_list[i])))
		{
			i = 0;
			Enumerate(plane, white_tile_list);
		}
	}
	return target;
}

Tile* TiSplitX(Tile* tile, int x) {
	Tile* newtile = new Tile();
	Tile* tp = new Tile();

	newtile->ti_body = 0;
	LEFT(newtile) = x;
	BOTTOM(newtile) = BOTTOM(tile);
	BL(newtile) = tile;
	TR(newtile) = TR(tile);
	RT(newtile) = RT(tile);
	newtile->ti_ur = tile->ti_ur;
	Point p1((newtile->ti_ll.p_x + newtile->ti_ur.p_x) / 2, (newtile->ti_ll.p_y + newtile->ti_ur.p_y) / 2);
	newtile->ti_mid = p1;
	Point p2(x, newtile->ti_ur.p_y);
	tile->ti_ur = p2;
	Point p3((tile->ti_ll.p_x + tile->ti_ur.p_x) / 2, (tile->ti_ll.p_y + tile->ti_ur.p_y) / 2);
	tile->ti_mid = p3;

	for (tp = TR(tile); BL(tp) == tile; tp = LB(tp))	//Adjust corner stitches along the right edge
		BL(tp) = newtile;
	TR(tile) = newtile;

	for (tp = RT(tile); LEFT(tp) >= x; tp = BL(tp))		//Adjust corner stitches along the top edge
		LB(tp) = newtile;
	RT(tile) = tp;

	for (tp = LB(tile); RIGHT(tp) <= x; tp = TR(tp)) {
		if (tp->ti_body == 3) {
			break;
		}
	}
	LB(newtile) = tp;

	while (RT(tp) == tile)
	{
		RT(tp) = newtile;
		tp = TR(tp);
	}

	return newtile;
}

Tile* TiSplitY(Tile* tile, int y) {
	Tile* newtile = new Tile();
	Tile* tp = new Tile();

	newtile->ti_body = 0;
	LEFT(newtile) = LEFT(tile);
	BOTTOM(newtile) = y;
	LB(newtile) = tile;
	RT(newtile) = RT(tile);
	TR(newtile) = TR(tile);
	newtile->ti_ur = tile->ti_ur;
	Point p1((newtile->ti_ll.p_x + newtile->ti_ur.p_x) / 2, (newtile->ti_ll.p_y + newtile->ti_ur.p_y) / 2);
	newtile->ti_mid = p1;
	Point p2(newtile->ti_ur.p_x, y);
	tile->ti_ur = p2;
	Point p3((tile->ti_ll.p_x + tile->ti_ur.p_x) / 2, (tile->ti_ll.p_y + tile->ti_ur.p_y) / 2);
	tile->ti_mid = p3;

	for (tp = RT(tile); LB(tp) == tile; tp = BL(tp)) {	//Adjust corner stitches along top edge
		LB(tp) = newtile;
	}
	RT(tile) = newtile;

	for (tp = TR(tile); BOTTOM(tp) >= y; tp = LB(tp)) {	//Adjust corner stitches along right edge
		BL(tp) = newtile;
	}
	TR(tile) = tp;

	for (tp = BL(tile); TOP(tp) <= y; tp = RT(tp)) {
		if (tp->ti_body == 3) {
			break;
		}
	}

	BL(newtile) = tp;
	//cout << "         " << BL(newtile)->ti_name << endl;

	while (TR(tp) == tile)
	{
		TR(tp) = newtile;
		tp = RT(tp);
	}

	return newtile;
}

void TiJoinX(Tile* tile1, Tile* tile2, Plane* plane) {
	Tile *tp;

	for (tp = RT(tile2); LB(tp) == tile2; tp = BL(tp)) {
		LB(tp) = tile1;
	}

	for (tp = LB(tile2); RT(tp) == tile2; tp = TR(tp)) {
		RT(tp) = tile1;
	}

	if (LEFT(tile1) < LEFT(tile2)) {
		for (tp = TR(tile2); BL(tp) == tile2; tp = LB(tp)) {
			BL(tp) = tile1;
		}
			
		TR(tile1) = TR(tile2);
		RT(tile1) = RT(tile2);

		tile1->ti_ur = tile2->ti_ur;
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}
	else {
		for (tp = BL(tile2); TR(tp) == tile2; tp = RT(tp)) {
			TR(tp) = tile1;
		}
		BL(tile1) = BL(tile2);
		LB(tile1) = LB(tile2);
		LEFT(tile1) = LEFT(tile2);
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}

	if (plane->pl_hint == tile2) {
		plane->pl_hint = tile1;
	}	
}

void TiJoinY(Tile* tile1, Tile* tile2, Plane* plane) {
	Tile *tp;

	for (tp = TR(tile2); BL(tp) == tile2; tp = LB(tp)) {
		BL(tp) = tile1;
	}

	for (tp = BL(tile2); TR(tp) == tile2; tp = RT(tp)) {
		TR(tp) = tile1;
	}

	if (BOTTOM(tile1) < BOTTOM(tile2)) {
		for (tp = RT(tile2); LB(tp) == tile2; tp = BL(tp)) {
			LB(tp) = tile1;
		}
		RT(tile1) = RT(tile2);
		TR(tile1) = TR(tile2);
		tile1->ti_ur = tile2->ti_ur;
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}
	else {
		for (tp = LB(tile2); RT(tp) == tile2; tp = TR(tp)) {
			RT(tp) = tile1;
		}
		LB(tile1) = LB(tile2);
		BL(tile1) = BL(tile2);
		BOTTOM(tile1) = BOTTOM(tile2);
		Point p1((tile1->ti_ll.p_x + tile1->ti_ur.p_x) / 2, (tile1->ti_ll.p_y + tile1->ti_ur.p_y) / 2);
		tile1->ti_mid = p1;
	}

	if (plane->pl_hint == tile2) {
		plane->pl_hint = tile1;
	}
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
		leng += (abs(soft_tile->name[i]->ti_mid.p_x - soft_x) + abs(soft_tile->name[i]->ti_mid.p_y - soft_y)) * soft_tile->weight[i];
	}
	return leng;
}

int wire_length(Tile* soft_tile, Tile* white_tile) {
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
		bound = 0.7 + (int)log10(max_area); //可改成不要+0.7
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
		Point mid1 = tp->name[i]->ti_mid;

		if (tp->name[i]->ti_body == 1)
		{
			cost = cost + 2 * tp->weight[i] * (abs(mid1.p_x - mid.p_x) + abs(mid1.p_y - mid.p_y));
		}
		else if (tp->name[i]->ti_body == 2)
		{
			cost = cost + tp->weight[i] * (abs(mid1.p_x - mid.p_x) + abs(mid1.p_y - mid.p_y));
		}
	}
	return cost;
}

float point_cost_tran(vector<Trantile> t, int num, vector<Tile*> fixed_tile_list, Point mid1, int mode)
{
	float cost = 0;
	for (int i = 0;i < t[num].name.size();i++)
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
	for (int i = 0;i < white.size();i++)
	{
		if (start.p_x >= white[i]->ti_ll.p_x && start.p_y >= white[i]->ti_ll.p_y)
		{
			if (start.p_x < white[i]->ti_ur.p_x && start.p_y < white[i]->ti_ur.p_y)
				p2 = white[i]->ti_ur;
		}
	}
	Rect area(start, p2);
	for (int i = 0;i < white.size();i++)
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

	for (int i = 1;i < rect.size();i++)
	{
		if (rect[i].r_ll.p_y != area.r_ur.p_y)
			break;

		Rect temp(area.r_ll, rect[i].r_ur);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
			{
				if (rect[j].r_ur.p_x > area.r_ur.p_x)
					rect[j].r_ur.p_x = area.r_ur.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
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
	for (int i = 0;i < white.size();i++)
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
	for (int i = 0;i < white.size();i++)
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

	for (int i = 1;i < rect.size();i++)
	{
		if (rect[i].r_ll.p_y != area.r_ur.p_y)
			break;

		Point p1(rect[i].r_ll.p_x, area.r_ll.p_y);
		Rect temp(p1, rect[i].r_ur);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
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
	for (int i = 0;i < white.size();i++)
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
	for (int i = 0;i < white.size();i++)
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

	for (int i = 1;i < rect.size();i++)
	{
		if (rect[i].r_ur.p_y != area.r_ll.p_y)
			break;

		Point p1(rect[i].r_ur.p_x, area.r_ur.p_y);
		Rect temp(rect[i].r_ll, p1);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
			{
				if (rect[j].r_ur.p_x > area.r_ur.p_x)
					rect[j].r_ur.p_x = area.r_ur.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
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
	for (int i = 0;i < white.size();i++)
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
	for (int i = 0;i < white.size();i++)
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

	for (int i = 1;i < rect.size();i++)
	{
		if (rect[i].r_ur.p_y != area.r_ll.p_y)
			break;

		Rect temp(rect[i].r_ll, area.r_ur);
		if (AREA(area) < target || HeightPerWidth(area) > 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
		else if (AREA(temp) > AREA(area) && HeightPerWidth(temp) <= 2)
		{
			area = temp;
			for (int j = i + 1;j < rect.size();j++)
			{
				if (rect[j].r_ll.p_x < area.r_ll.p_x)
					rect[j].r_ll.p_x = area.r_ll.p_x;
			}
		}
	}
	return area;
}
/*
case1: 2.07857e+08
case2: 4.19506e+07
case3: 2.70984e+06
*/
void InsertSoftTile(Plane* plane, vector<Tile*>& soft_tile_list, int type1) { //改回soft_tile_list不要&  

	if(type1) {
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
		for (int j = 0;j < 20;j++)
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

		for (int j = 0;j < white.size();j++)
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

		for (int j = 0;j < point.size();j++)
		{
			for (int k = 0;k < model.size();k++)
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
			vector<Tile*> c = soft_tile_list[i]->name;
			vector<int> w = soft_tile_list[i]->weight;
			int m = soft_tile_list[i]->miniarea;

			soft_tile_list[i] = InsertFixedTile(point[min].realarea, plane);
			soft_tile_list[i]->name = c;
			soft_tile_list[i]->weight = w;
			soft_tile_list[i]->ti_name = n;
			soft_tile_list[i]->ti_body = 2;
			soft_tile_list[i]->miniarea = m;

			for (int j = 0; j < plane->fixed_tile_list.size(); j++)
			{
				for (int k = 0; k < plane->fixed_tile_list[j]->name.size(); k++) 
				{
					if (plane->fixed_tile_list[j]->name[k]->ti_name == soft_tile_list[i]->ti_name) 
					{
						plane->fixed_tile_list[j]->name[k] = soft_tile_list[i];
					}
				}
			}
			for (int j = 0; j < plane->soft_tile_list.size(); j++)
			{
				for (int k = 0; k < plane->soft_tile_list[j]->name.size(); k++)
				{
					if (plane->soft_tile_list[j]->name[k]->ti_name == soft_tile_list[i]->ti_name)
					{
						plane->soft_tile_list[j]->name[k] = soft_tile_list[i];
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
			double x2 = double((soft_tile_list[i]->name[j]->ti_ll.p_x + soft_tile_list[i]->name[j]->ti_ur.p_x) / 2);
			double y2 = double((soft_tile_list[i]->name[j]->ti_ll.p_y + soft_tile_list[i]->name[j]->ti_ur.p_y) / 2);

			double length = abs(x2 - x1) + abs(y2 - y1);
			hpwl = hpwl + soft_tile_list[i]->weight[j] * length;
		}
	}
	for (int i = 0; i < plane->fixed_tile_list.size(); i++) {
		for (int j = 0; j < plane->fixed_tile_list[i]->name.size(); j++) {
			double x1 = double((plane->fixed_tile_list[i]->ti_ll.p_x + plane->fixed_tile_list[i]->ti_ur.p_x) / 2);
			double y1 = double((plane->fixed_tile_list[i]->ti_ll.p_y + plane->fixed_tile_list[i]->ti_ur.p_y) / 2);
			double x2 = double((plane->fixed_tile_list[i]->name[j]->ti_ll.p_x + plane->fixed_tile_list[i]->name[j]->ti_ur.p_x) / 2);
			double y2 = double((plane->fixed_tile_list[i]->name[j]->ti_ll.p_y + plane->fixed_tile_list[i]->name[j]->ti_ur.p_y) / 2);

			double length = abs(x2 - x1) + abs(y2 - y1);
			hpwl = hpwl + plane->fixed_tile_list[i]->weight[j] * length;
		}
	}
	plane->hpwl = hpwl / 2;
}

void RemoveTile(Tile*& tile, Plane*& plane)
{
	vector<Tile*> white_tile_list;


	tile = TiSrPoint(NULL, plane, tile->ti_ll);
	const int del_ytop = TOP(tile);
	const int del_ybot = BOTTOM(tile);

	tile->ti_body = 0;

	Tile* right_start = TR(tile);
	Tile* left_start = BL(tile);


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

	if ((BL(RT(tp))->ti_body != 0) && canMergeVertical(tp, RT(tp)))
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

		Tile* tmp = TR(tp);
		if (TOP(tp) < TOP(tmp))
		{
			TiSplitY(tmp, TOP(tp));
		}

		if (tp->ti_body != 0)
		{
			tp = next;
			continue;
		}

		for (tmp = TR(tp); BOTTOM(tp) < BOTTOM(tmp); tmp = LB(tmp))
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
		while (BOTTOM(TR(tp)) > BOTTOM(tp))
		{
			next2 = TiSplitY(tp, BOTTOM(TR(tp)));
			if (canMergeHorizontal(next2, TR(next2)))
			{
				TiJoinX(next2, TR(next2), plane);
			}
		}
		//end

		if (canMergeHorizontal(tp, TR(tp)))
		{
			TiJoinX(tp, TR(tp), plane);
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

		if (canMergeHorizontal(white_tile_list[i], BL(white_tile_list[i])))
		{
			i = 0;
			Enumerate(plane, white_tile_list);
		}
	}
	//end


	// reset hint tile
	plane->pl_hint = TiSrPoint(tp, plane, plane->pl_hint->ti_ll);
}

void Replace(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list)  //改回soft_tile_list不要&  
{
	for (int i = 0; i < soft_tile_list.size(); i++) //soft_tile_list.size()
	{
		insert_point pre;
		pre.realarea.r_ll = soft_tile_list[i].t1->ti_ll;
		pre.realarea.r_ur = soft_tile_list[i].t1->ti_ur;
		
		RemoveTile(soft_tile_list[i].t1, plane);
		vector<Tile*> white;
		Enumerate(plane, white);
		vector<insert_point> point;
		vector<Rect> model;
		
		pre.cost = point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 1);
		point.push_back(pre);
		//cout << i << "\t" << soft_tile_list[i].ti_name << "\t" << pre.cost << "\t";
		//cout << soft_tile_list[i].t1->ti_ll.p_x << " " << soft_tile_list[i].t1->ti_ll.p_y << " ";
		//cout << soft_tile_list[i].t1->ti_ur.p_x << " " << soft_tile_list[i].t1->ti_ur.p_y << endl;

		int target = soft_tile_list[i].miniarea;
		double ratio = 1;
		for (int j = 0;j < 40;j++)
		{
			int shorter = ceil(sqrt(double(target / ratio)));
			int longer = ceil(ratio * shorter);

			Point a(0, 0);
			Point b(shorter, longer);
			Point c(longer, shorter);

			model.push_back(Rect(a, b));
			model.push_back(Rect(a, c));

			ratio = ratio + 0.025;
		}

		/*cout << "model" << endl;
		for (int j = 0;j < model.size();j++)
		{
			cout << model[j].r_ll.p_x << " " << model[j].r_ll.p_y << " ";
			cout << model[j].r_ur.p_x << " " << model[j].r_ur.p_y << " " << HeightPerWidth(model[j]) << " " << AREA(model[j]) << endl;
		}
		cout << endl;*/

		Point fac[4] = { {0,0},{1,0},{0,1},{1,1} };
		insert_point in;
		in.cost = INT_MAX;
		for (int a = 0;a < white.size();a++) //white.size()
		{
			float whw = WIDTH(white[a]) / 4;
			float whh = HEIGHT(white[a]) / 4;

			//cout << white[a]->ti_ll.p_x << " " << white[a]->ti_ll.p_y << " " << white[a]->ti_ur.p_x << " " << white[a]->ti_ur.p_y << endl;

			for (int j = 0;j < 4;j++)
			{
				in.dir = j + 1;
				Point start;
				start.p_x = white[a]->ti_ll.p_x + round(fac[j].p_x * whw);
				start.p_y = white[a]->ti_ll.p_y + round(fac[j].p_y * whh);

				for (int k = 0;k < 5;k++)
				{
					for (int m = 0;m < 5;m++)
					{
						in.check_p.p_x = white[a]->ti_ll.p_x + round(m * whw);
						in.check_p.p_y = white[a]->ti_ll.p_y + round(k * whh);

						//cout << in.check_p.p_x << " " << in.check_p.p_y << endl;

						if (j == 0)
							in.canusearea = CanUseArea1(white, in.check_p, target);
						else if (j == 1)
							in.canusearea = CanUseArea2(white, in.check_p, target);
						else if (j == 2)
							in.canusearea = CanUseArea3(white, in.check_p, target);
						else if (j == 3)
							in.canusearea = CanUseArea4(white, in.check_p, target);

						if (target <= AREA(in.canusearea))
							point.push_back(in);
					}
				}
			}

		}

		for (int j = 0;j < point.size();j++)
		{
			for (int k = 0;k < model.size();k++)
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

				float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, mid, 1);
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
		for (int j = 0; j < point.size(); j++)
		{
			if (point[j].cost < point[min].cost)
				min = j;
		}

		/*cout << point.size() << " " << min << endl;

		cout << point[min].realarea.r_ll.p_x << "\t" << point[min].realarea.r_ll.p_y << "\t";
		cout << point[min].realarea.r_ur.p_x << "\t" << point[min].realarea.r_ur.p_y << endl;*/

		/*if (i == 18)
		{
			cout << "11111118" << endl;
			cout << point[min].realarea.r_ll.p_x << "\t" << point[min].realarea.r_ll.p_y << "\t";
			cout << point[min].realarea.r_ur.p_x << "\t" << point[min].realarea.r_ur.p_y << endl;
		}*/

		//cout << point[min].cost << endl;

		soft_tile_list[i].t1 = InsertFixedTile(point[min].realarea, plane);
		soft_tile_list[i].frame_mid = soft_tile_list[i].t1->ti_mid;
	}
}

void Special_Transform(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list)
{
	for (int i = 0;i < soft_tile_list.size();i++)
	{
		if (soft_tile_list[i].t2 != nullptr)
			continue;
		
		for (int j = i + 1;j < soft_tile_list.size();j++)
		{
			if (soft_tile_list[j].t2 != nullptr)
				continue;

			if (soft_tile_list[i].t1->ti_ll.p_y == soft_tile_list[j].t1->ti_ll.p_y &&
				soft_tile_list[i].t1->ti_ur.p_y == soft_tile_list[j].t1->ti_ur.p_y)
			{
				if (soft_tile_list[i].t1->ti_ll.p_x == soft_tile_list[j].t1->ti_ur.p_x)
				{
					//cout << "1" << endl;
					//cout << soft_tile_list[i].ti_name << "\t" << soft_tile_list[j].ti_name << endl;
					Point premid1 = soft_tile_list[i].frame_mid;
					Point premid2 = soft_tile_list[j].frame_mid;
					float precost = point_cost_tran(soft_tile_list, i, fixed_tile_list, premid1, 1)
									+ point_cost_tran(soft_tile_list, j, fixed_tile_list, premid2, 1);
					int best_k = 0;
					//cout << "precost\t" << precost << endl;

					int a = 0;
					int b = 0;
					if (HEIGHT(soft_tile_list[i].t1) % 2 == 0)
					{
						a = HEIGHT(soft_tile_list[i].t1) / 2;
						b = HEIGHT(soft_tile_list[i].t1) / 2;
					}
					else
					{
						a = ceil(double(HEIGHT(soft_tile_list[i].t1)) / double(2));
						b = floor(double(HEIGHT(soft_tile_list[i].t1)) / double(2));
					}
					
					int count = 1;
					while(1)
					{
						soft_tile_list[i].frame_mid.p_x--;
						soft_tile_list[j].frame_mid.p_x++;

						float frame_a = (WIDTH(soft_tile_list[i].t1) + 2 * count) * HEIGHT(soft_tile_list[i].t1);
						float real_a = frame_a - 4 * count * b;
						if (real_a / frame_a < 0.8) break;

						float frame_b = (WIDTH(soft_tile_list[j].t1) + 2 * count) * HEIGHT(soft_tile_list[j].t1);
						float real_b = frame_b - 4 * count * a;
						if (real_b / frame_b < 0.8) break;
						if (real_b < soft_tile_list[j].miniarea) break;

						float r_a = float(WIDTH(soft_tile_list[i].t1) + 2 * count) / float(HEIGHT(soft_tile_list[i].t1));
						float r_b = float(WIDTH(soft_tile_list[j].t1) + 2 * count) / float(HEIGHT(soft_tile_list[j].t1));
						if (r_a > 2 || r_a < 0.5) break;
						if (r_b > 2 || r_b < 0.5) break;
						
						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 1)
										+ point_cost_tran(soft_tile_list, j, fixed_tile_list, soft_tile_list[j].frame_mid, 1);

						if (newcost < precost)
						{
							best_k = count;
							//cout << count << "\t" << newcost << endl;
						}
						count++;
					}
					soft_tile_list[i].frame_mid.p_x = premid1.p_x - best_k;
					soft_tile_list[j].frame_mid.p_x = premid2.p_x + best_k;

					if (best_k == 0)
						continue;

					soft_tile_list[i].dir = 6;
					soft_tile_list[j].dir = 2;
					

					Rect area1_a(soft_tile_list[i].t1->ti_ll, soft_tile_list[i].t1->ti_ur);
					Rect area1_b(soft_tile_list[j].t1->ti_ll, soft_tile_list[j].t1->ti_ur);
					Rect area2_a, area2_b;

					area1_a.r_ll.p_x = area1_a.r_ll.p_x + 2 * count;
					area1_b.r_ur.p_x = area1_b.r_ur.p_x - 2 * count;

					area2_a.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x - 2 * count;
					area2_a.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y;
					area2_a.r_ur.p_x = soft_tile_list[i].t1->ti_ll.p_x + 2 * count;
					area2_a.r_ur.p_y = soft_tile_list[i].t1->ti_ll.p_y + a;
					area2_b.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x - 2 * count;
					area2_b.r_ll.p_y = soft_tile_list[i].t1->ti_ur.p_y - b;
					area2_b.r_ur.p_x = soft_tile_list[i].t1->ti_ll.p_x - 2 * count;
					area2_b.r_ur.p_y = soft_tile_list[i].t1->ti_ur.p_y;
						
					RemoveTile(soft_tile_list[i].t1, plane);
					RemoveTile(soft_tile_list[j].t1, plane);
					
					soft_tile_list[i].t1 = InsertFixedTile(area1_a, plane);
					soft_tile_list[i].t2 = InsertFixedTile(area2_a, plane);
					soft_tile_list[j].t1 = InsertFixedTile(area1_b, plane);
					soft_tile_list[j].t2 = InsertFixedTile(area2_b, plane);

					//cout << "--------------" << endl;
				}
				else if (soft_tile_list[i].t1->ti_ur.p_x == soft_tile_list[j].t1->ti_ll.p_x)
				{
					//cout << "2" << endl;
					//cout << soft_tile_list[i].ti_name << "\t" << soft_tile_list[j].ti_name << endl;

					Point premid1 = soft_tile_list[i].frame_mid;
					Point premid2 = soft_tile_list[j].frame_mid;
					float precost = point_cost_tran(soft_tile_list, i, fixed_tile_list, premid1, 1)
									+ point_cost_tran(soft_tile_list, j, fixed_tile_list, premid2, 1);
					int best_k = 0;
					//cout << "precost\t" << precost << endl;

					int a = 0;
					int b = 0;
					if (HEIGHT(soft_tile_list[i].t1) % 2 == 0)
					{
						a = HEIGHT(soft_tile_list[i].t1) / 2;
						b = HEIGHT(soft_tile_list[i].t1) / 2;
					}
					else
					{
						a = ceil(double(HEIGHT(soft_tile_list[i].t1)) / double(2));
						b = floor(double(HEIGHT(soft_tile_list[i].t1)) / double(2));
					}

					int count = 1;
					while (1)
					{
						soft_tile_list[i].frame_mid.p_x++;
						soft_tile_list[j].frame_mid.p_x--;

						float frame_a = (WIDTH(soft_tile_list[i].t1) + 2 * count) * HEIGHT(soft_tile_list[i].t1);
						float real_a = frame_a - 4 * count * b;
						if (real_a / frame_a < 0.8) break;

						float frame_b = (WIDTH(soft_tile_list[j].t1) + 2 * count) * HEIGHT(soft_tile_list[j].t1);
						float real_b = frame_b - 4 * count * a;
						if (real_b / frame_b < 0.8) break;
						if (real_b < soft_tile_list[j].miniarea) break;

						float r_a = float(WIDTH(soft_tile_list[i].t1) + 2 * count) / float(HEIGHT(soft_tile_list[i].t1));
						float r_b = float(WIDTH(soft_tile_list[j].t1) + 2 * count) / float(HEIGHT(soft_tile_list[j].t1));
						if (r_a > 2 || r_a < 0.5) break;
						if (r_b > 2 || r_b < 0.5) break;

						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 1)
							+ point_cost_tran(soft_tile_list, j, fixed_tile_list, soft_tile_list[j].frame_mid, 1);

						if (newcost < precost)
						{
							best_k = count;
							//cout << count << "\t" << newcost << endl;
						}
						count++;
					}
					soft_tile_list[i].frame_mid.p_x = premid1.p_x + best_k;
					soft_tile_list[j].frame_mid.p_x = premid2.p_x - best_k;

					if (best_k == 0)
						continue;

					soft_tile_list[i].dir = 3;
					soft_tile_list[j].dir = 7;


					Rect area1_a(soft_tile_list[i].t1->ti_ll, soft_tile_list[i].t1->ti_ur);
					Rect area1_b(soft_tile_list[j].t1->ti_ll, soft_tile_list[j].t1->ti_ur);
					Rect area2_a, area2_b;

					area1_a.r_ur.p_x = area1_a.r_ur.p_x - 2 * count;
					area1_b.r_ll.p_x = area1_b.r_ll.p_x + 2 * count;

					area2_a.r_ll.p_x = soft_tile_list[j].t1->ti_ll.p_x - 2 * count;
					area2_a.r_ll.p_y = soft_tile_list[j].t1->ti_ll.p_y;
					area2_a.r_ur.p_x = soft_tile_list[j].t1->ti_ll.p_x + 2 * count;
					area2_a.r_ur.p_y = soft_tile_list[j].t1->ti_ll.p_y + a;
					area2_b.r_ll.p_x = soft_tile_list[j].t1->ti_ll.p_x - 2 * count;
					area2_b.r_ll.p_y = soft_tile_list[j].t1->ti_ur.p_y - b;
					area2_b.r_ur.p_x = soft_tile_list[j].t1->ti_ll.p_x - 2 * count;
					area2_b.r_ur.p_y = soft_tile_list[j].t1->ti_ur.p_y;

					//cout << "aaaaaaaaaaa" << endl;

					RemoveTile(soft_tile_list[i].t1, plane);
					RemoveTile(soft_tile_list[j].t1, plane);

					//cout << "bbbbbbbbbbb" << endl;

					//cout << area1_a.r_ll.p_x << "\t" << area1_a.r_ll.p_y << area1_a.r_ur.p_x << "\t" << area1_a.r_ur.p_y << endl;
					//cout << area2_a.r_ll.p_x << "\t" << area2_a.r_ll.p_y << area2_a.r_ur.p_x << "\t" << area2_a.r_ur.p_y << endl;
					//cout << area1_b.r_ll.p_x << "\t" << area1_b.r_ll.p_y << area1_b.r_ur.p_x << "\t" << area1_b.r_ur.p_y << endl;
					//cout << area2_b.r_ll.p_x << "\t" << area2_b.r_ll.p_y << area2_b.r_ur.p_x << "\t" << area2_b.r_ur.p_y << endl;

					soft_tile_list[i].t1 = InsertFixedTile(area1_a, plane);
					soft_tile_list[i].t2 = InsertFixedTile(area2_a, plane);
					soft_tile_list[j].t1 = InsertFixedTile(area1_b, plane);
					soft_tile_list[j].t2 = InsertFixedTile(area2_b, plane);

					//cout << "--------------" << endl;
				}
			}
			else if (soft_tile_list[i].t1->ti_ll.p_x == soft_tile_list[j].t1->ti_ll.p_x &&
					 soft_tile_list[i].t1->ti_ur.p_x == soft_tile_list[j].t1->ti_ur.p_x)
			{
				if (soft_tile_list[i].t1->ti_ll.p_y == soft_tile_list[j].t1->ti_ur.p_y)
				{
					//cout << "3" << endl;
					//cout << soft_tile_list[i].ti_name << "\t" << soft_tile_list[j].ti_name << endl;

					Point premid1 = soft_tile_list[i].frame_mid;
					Point premid2 = soft_tile_list[j].frame_mid;
					float precost = point_cost_tran(soft_tile_list, i, fixed_tile_list, premid1, 1)
						+ point_cost_tran(soft_tile_list, j, fixed_tile_list, premid2, 1);
					int best_k = 0;
					//cout << "precost\t" << precost << endl;

					int a = 0;
					int b = 0;
					if (WIDTH(soft_tile_list[i].t1) % 2 == 0)
					{
						a = WIDTH(soft_tile_list[i].t1) / 2;
						b = WIDTH(soft_tile_list[i].t1) / 2;
					}
					else
					{
						a = ceil(double(WIDTH(soft_tile_list[i].t1)) / double(2));
						b = floor(double(WIDTH(soft_tile_list[i].t1)) / double(2));
					}

					int count = 1;
					while (1)
					{
						soft_tile_list[i].frame_mid.p_y--;
						soft_tile_list[j].frame_mid.p_y++;

						float frame_a = (HEIGHT(soft_tile_list[i].t1) + 2 * count) * WIDTH(soft_tile_list[i].t1);
						float real_a = frame_a - 4 * count * b;
						if (real_a / frame_a < 0.8) break;

						float frame_b = (HEIGHT(soft_tile_list[j].t1) + 2 * count) * WIDTH(soft_tile_list[j].t1);
						float real_b = frame_b - 4 * count * a;
						if (real_b / frame_b < 0.8) break;
						if (real_b < soft_tile_list[j].miniarea) break;

						float r_a = float(HEIGHT(soft_tile_list[i].t1) + 2 * count) / float(WIDTH(soft_tile_list[i].t1));
						float r_b = float(HEIGHT(soft_tile_list[j].t1) + 2 * count) / float(WIDTH(soft_tile_list[j].t1));
						if (r_a > 2 || r_a < 0.5) break;
						if (r_b > 2 || r_b < 0.5) break;

						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 1)
							+ point_cost_tran(soft_tile_list, j, fixed_tile_list, soft_tile_list[j].frame_mid, 1);
						
						//cout << count << "\t" << newcost << endl;
						if (newcost < precost)
						{
							best_k = count;
						}
						count++;
					}
					soft_tile_list[i].frame_mid.p_y = premid1.p_y - best_k;
					soft_tile_list[j].frame_mid.p_y = premid2.p_y + best_k;

					//cout << "--------------" << endl;
					if (best_k == 0)
						continue;

					soft_tile_list[i].dir = 5;
					soft_tile_list[j].dir = 1;


					Rect area1_a(soft_tile_list[i].t1->ti_ll, soft_tile_list[i].t1->ti_ur);
					Rect area1_b(soft_tile_list[j].t1->ti_ll, soft_tile_list[j].t1->ti_ur);
					Rect area2_a, area2_b;

					area1_a.r_ll.p_y = area1_a.r_ll.p_y + 2 * count;
					area1_b.r_ur.p_y = area1_b.r_ur.p_y - 2 * count;

					area2_a.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					area2_a.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y - 2 * count;
					area2_a.r_ur.p_x = soft_tile_list[i].t1->ti_ll.p_x + a;
					area2_a.r_ur.p_y = soft_tile_list[i].t1->ti_ll.p_y + 2 * count;
					area2_b.r_ll.p_x = soft_tile_list[i].t1->ti_ur.p_x - b;
					area2_b.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y - 2 * count;
					area2_b.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					area2_b.r_ur.p_y = soft_tile_list[i].t1->ti_ll.p_y + 2 * count;

					//cout << area1_a.r_ll.p_x << "\t" << area1_a.r_ll.p_y << "\t" << area1_a.r_ur.p_x << "\t" << area1_a.r_ur.p_y << endl;
					//cout << area2_a.r_ll.p_x << "\t" << area2_a.r_ll.p_y << "\t" << area2_a.r_ur.p_x << "\t" << area2_a.r_ur.p_y << endl;
					//cout << area1_b.r_ll.p_x << "\t" << area1_b.r_ll.p_y << "\t" << area1_b.r_ur.p_x << "\t" << area1_b.r_ur.p_y << endl;
					//cout << area2_b.r_ll.p_x << "\t" << area2_b.r_ll.p_y << "\t" << area2_b.r_ur.p_x << "\t" << area2_b.r_ur.p_y << endl;

					RemoveTile(soft_tile_list[i].t1, plane);
					RemoveTile(soft_tile_list[j].t1, plane);

					soft_tile_list[i].t1 = InsertFixedTile(area1_a, plane);
					soft_tile_list[i].t2 = InsertFixedTile(area2_a, plane);
					soft_tile_list[j].t1 = InsertFixedTile(area1_b, plane);
					soft_tile_list[j].t2 = InsertFixedTile(area2_b, plane);	
				}
				else if (soft_tile_list[i].t1->ti_ur.p_y == soft_tile_list[j].t1->ti_ll.p_y)
				{
					//cout << "4" << endl;
					//cout << soft_tile_list[i].ti_name << "\t" << soft_tile_list[j].ti_name << endl;

					Point premid1 = soft_tile_list[i].frame_mid;
					Point premid2 = soft_tile_list[j].frame_mid;
					float precost = point_cost_tran(soft_tile_list, i, fixed_tile_list, premid1, 1)
						+ point_cost_tran(soft_tile_list, j, fixed_tile_list, premid2, 1);
					int best_k = 0;
					//cout << "precost\t" << precost << endl;

					int a = 0;
					int b = 0;
					if (WIDTH(soft_tile_list[i].t1) % 2 == 0)
					{
						a = WIDTH(soft_tile_list[i].t1) / 2;
						b = WIDTH(soft_tile_list[i].t1) / 2;
					}
					else
					{
						a = ceil(double(WIDTH(soft_tile_list[i].t1)) / double(2));
						b = floor(double(WIDTH(soft_tile_list[i].t1)) / double(2));
					}

					int count = 1;
					while (1)
					{
						soft_tile_list[i].frame_mid.p_y++;
						soft_tile_list[j].frame_mid.p_y--;

						float frame_a = (HEIGHT(soft_tile_list[i].t1) + 2 * count) * WIDTH(soft_tile_list[i].t1);
						float real_a = frame_a - 4 * count * b;
						if (real_a / frame_a < 0.8) break;

						float frame_b = (HEIGHT(soft_tile_list[j].t1) + 2 * count) * WIDTH(soft_tile_list[j].t1);
						float real_b = frame_b - 4 * count * a;
						if (real_b / frame_b < 0.8) break;
						if (real_b < soft_tile_list[j].miniarea) break;

						float r_a = float(HEIGHT(soft_tile_list[i].t1) + 2 * count) / float(WIDTH(soft_tile_list[i].t1));
						float r_b = float(HEIGHT(soft_tile_list[j].t1) + 2 * count) / float(WIDTH(soft_tile_list[j].t1));
						if (r_a > 2 || r_a < 0.5) break;
						if (r_b > 2 || r_b < 0.5) break;

						float newcost = point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 1)
							+ point_cost_tran(soft_tile_list, j, fixed_tile_list, soft_tile_list[j].frame_mid, 1);

						//cout << count << "\t" << newcost << endl;
						if (newcost < precost)
						{
							best_k = count;
						}
						count++;
					}
					soft_tile_list[i].frame_mid.p_y = premid1.p_y + best_k;
					soft_tile_list[j].frame_mid.p_y = premid2.p_y - best_k;

					//cout << "--------------" << endl;
					if (best_k == 0)
						continue;

					soft_tile_list[i].dir = 8;
					soft_tile_list[j].dir = 4;


					Rect area1_a(soft_tile_list[i].t1->ti_ll, soft_tile_list[i].t1->ti_ur);
					Rect area1_b(soft_tile_list[j].t1->ti_ll, soft_tile_list[j].t1->ti_ur);
					Rect area2_a, area2_b;

					area1_a.r_ur.p_y = area1_a.r_ur.p_y - 2 * count;
					area1_b.r_ll.p_y = area1_b.r_ll.p_y + 2 * count;

					area2_a.r_ll.p_x = soft_tile_list[i].t1->ti_ll.p_x;
					area2_a.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y - 2 * count;
					area2_a.r_ur.p_x = soft_tile_list[i].t1->ti_ll.p_x + a;
					area2_a.r_ur.p_y = soft_tile_list[i].t1->ti_ll.p_y + 2 * count;
					area2_b.r_ll.p_x = soft_tile_list[i].t1->ti_ur.p_x - b;
					area2_b.r_ll.p_y = soft_tile_list[i].t1->ti_ll.p_y - 2 * count;
					area2_b.r_ur.p_x = soft_tile_list[i].t1->ti_ur.p_x;
					area2_b.r_ur.p_y = soft_tile_list[i].t1->ti_ll.p_y + 2 * count;

					//cout << area1_a.r_ll.p_x << "\t" << area1_a.r_ll.p_y << "\t" << area1_a.r_ur.p_x << "\t" << area1_a.r_ur.p_y << endl;
					//cout << area2_a.r_ll.p_x << "\t" << area2_a.r_ll.p_y << "\t" << area2_a.r_ur.p_x << "\t" << area2_a.r_ur.p_y << endl;
					//cout << area1_b.r_ll.p_x << "\t" << area1_b.r_ll.p_y << "\t" << area1_b.r_ur.p_x << "\t" << area1_b.r_ur.p_y << endl;
					//cout << area2_b.r_ll.p_x << "\t" << area2_b.r_ll.p_y << "\t" << area2_b.r_ur.p_x << "\t" << area2_b.r_ur.p_y << endl;

					RemoveTile(soft_tile_list[i].t1, plane);
					RemoveTile(soft_tile_list[j].t1, plane);

					soft_tile_list[i].t1 = InsertFixedTile(area1_a, plane);
					soft_tile_list[i].t2 = InsertFixedTile(area2_a, plane);
					soft_tile_list[j].t1 = InsertFixedTile(area1_b, plane);
					soft_tile_list[j].t2 = InsertFixedTile(area2_b, plane);
				}
			}

		}
	}
}

void Transform(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list)
{	
	for (int i = 0;i < soft_tile_list.size();i++)	//soft_tile_list.size()
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
		
		vector<Tile*> white;
		Enumerate(plane, white);

		for (int j = 0;j < p.size();j++)
		{
			
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
			
			if (AREA(p[j].area2) == 0)
			{
				continue;
			}

			/*cout << j << endl;
			cout << p[j].type << endl;
			cout << p[j].area2.r_ll.p_x << " " << p[j].area2.r_ll.p_y << endl;
			cout << p[j].area2.r_ur.p_x << " " << p[j].area2.r_ur.p_y << endl;
			cout << "---------------------------" << endl;*/
				
			Point mid = soft_tile_list[i].t1->ti_mid;
			if (p[j].type == 1 || p[j].type == 8)
			{
				if (p[j].area2.GEO_WIDTH() >= WIDTH(soft_tile_list[i].t1))
				{
					p[j].tran = 0;
					for (int k = 0;k < p[j].area2.GEO_HEIGHT();k++)
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

					for (int k = 0;k < p[j].area2.GEO_WIDTH();k++)
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

					for (int k = 0;k < p[j].area2.GEO_HEIGHT();k++)
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

					for (int k = 0;k < p[j].area2.GEO_WIDTH();k++)
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

		for (int j = 0;j < 8;j++)
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

		
		
		for (int j = 0;j < 8;j++)
		{
			if (p[j].cost >= cost)
				break;
			//cout << p[j].type << " " << p[j].cost << " ";
			//cout << p[j].frame_mid.p_x << " " << p[j].frame_mid.p_y << endl;
			
			soft_tile_list[i].frame_mid = p[j].frame_mid;
			soft_tile_list[i].dir = p[j].type;
			RemoveTile(soft_tile_list[i].t1, plane);

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

void ForceDirected2(Plane* plane, vector<Trantile>& soft_tile_list, vector<Tile*> fixed_tile_list)
{
	float total = 0;

	ofstream newFile;
	newFile.open("case1_force.m");
	newFile << "axis equal;\n" << "hold on;\n" << "grid on;\n";
	newFile << "block_x=[0 0 " << plane->pl_width << " " << plane->pl_width << " 0];" << endl;
	newFile << "block_y=[0 " << plane->pl_height << " " << plane->pl_height << " 0 0];" << endl;
	newFile << "fill(block_x, block_y, 'c');" << endl;

	for (int i = 0;i < soft_tile_list.size();i++)
	{
		int width = WIDTH(soft_tile_list[i].t1) / 2;
		int height = HEIGHT(soft_tile_list[i].t1) / 2;

		Point zero;
		for (int j = 0;j < soft_tile_list[i].name.size();j++)
		{
			if (soft_tile_list[i].type[j] == 0)
			{
				zero.p_x = zero.p_x + soft_tile_list[i].weight[j] * fixed_tile_list[soft_tile_list[i].name[j]]->ti_mid.p_x;
				zero.p_y = zero.p_y + soft_tile_list[i].weight[j] * fixed_tile_list[soft_tile_list[i].name[j]]->ti_mid.p_y;
			}
			else
			{
				zero.p_x = zero.p_x + soft_tile_list[i].weight[j] * soft_tile_list[soft_tile_list[i].name[j]].frame_mid.p_x;
				zero.p_y = zero.p_y + soft_tile_list[i].weight[j] * soft_tile_list[soft_tile_list[i].name[j]].frame_mid.p_y;
			}
		}
		zero.p_x = zero.p_x / soft_tile_list[i].totalweight;
		zero.p_y = zero.p_y / soft_tile_list[i].totalweight;

		cout << "pre " << point_cost_tran(soft_tile_list, i, fixed_tile_list, soft_tile_list[i].frame_mid, 1);
		cout << "\t\tforce " << point_cost_tran(soft_tile_list, i, fixed_tile_list, zero, 1) << endl;
		total = total + point_cost_tran(soft_tile_list, i, fixed_tile_list, zero, 1);

		newFile << "block_x=[" << zero.p_x - width << " " << zero.p_x - width << " ";
		newFile << zero.p_x + width << " " << zero.p_x + width << " " << zero.p_x - width << "];" << endl;
		newFile << "block_y=[" << zero.p_y - height << " " << zero.p_y + height << " ";
		newFile << zero.p_y + height << " " << zero.p_y - height << " " << zero.p_y - height << "];" << endl;
		newFile << "fill(block_x, block_y, 'g');" << endl;
	}
	cout << total / 2 << endl;
}

void ForceDirected(Plane* plane, vector<Tile*>& soft_tile_list, vector<Tile*> fixed_tile_list)
{
	for (int i = 0;i < soft_tile_list.size();i++)
	{
		Point zero;
		int totalweight = 0;
		for (int j = 0;j < soft_tile_list[i]->name.size();j++)
		{
			zero.p_x = zero.p_x + soft_tile_list[i]->weight[j] * soft_tile_list[i]->name[j]->ti_mid.p_x;
			zero.p_y = zero.p_y + soft_tile_list[i]->weight[j] * soft_tile_list[i]->name[j]->ti_mid.p_y;
			totalweight = totalweight + soft_tile_list[i]->weight[j];
		}
		zero.p_x = zero.p_x / totalweight;
		zero.p_y = zero.p_y / totalweight;

		


		vector<Tile*> white;
		Enumerate(plane, white);

		if (GEO_SAMEPOINT(zero,soft_tile_list[i]->ti_mid))
		{
			continue;
		}
		else
		{
			vector<Rect> model;
			int target = AREA(soft_tile_list[i]);
			double ratio = 1;
			for (int j = 0;j < 20;j++)
			{
				int shorter = ceil(sqrt(double(target / ratio)) / 2);
				int longer = ceil(ratio * shorter);

				Point a(zero.p_x - shorter, zero.p_y - longer);
				Point b(zero.p_x + shorter, zero.p_y + longer);
				Point c(zero.p_x - longer, zero.p_y - shorter);
				Point d(zero.p_x + longer, zero.p_y + shorter);

				if (a.p_x >= 0 && a.p_x <= plane->pl_width && a.p_y >= 0 && a.p_y <= plane->pl_height &&
					b.p_x >= 0 && b.p_x <= plane->pl_width && b.p_y >= 0 && b.p_y <= plane->pl_height)
				{
					model.push_back(Rect(a, b));
				}
				if (c.p_x >= 0 && c.p_x <= plane->pl_width && c.p_y >= 0 && c.p_y <= plane->pl_height &&
					d.p_x >= 0 && d.p_x <= plane->pl_width && d.p_y >= 0 && d.p_y <= plane->pl_height)
				{
					model.push_back(Rect(c, d));
				}
				
				ratio = ratio + 0.05;
			}

			for (int j = 0;j < model.size();j++)
			{
				for (int k = 0;k < soft_tile_list.size();k++)
				{
					if ((soft_tile_list[k]->ti_ll.p_x > model[j].r_ll.p_x && soft_tile_list[k]->ti_ll.p_x < model[j].r_ur.p_x) ||
						(soft_tile_list[k]->ti_ll.p_y > model[j].r_ll.p_y && soft_tile_list[k]->ti_ll.p_y < model[j].r_ur.p_y) ||
						(soft_tile_list[k]->ti_ur.p_x > model[j].r_ll.p_x && soft_tile_list[k]->ti_ur.p_x < model[j].r_ur.p_x) ||
						(soft_tile_list[k]->ti_ur.p_y > model[j].r_ll.p_x && soft_tile_list[k]->ti_ur.p_y < model[j].r_ur.p_x))
					{

					}
				}

				for (int k = 0;k < fixed_tile_list.size();k++)
				{

				}
			}
		}

	}
}

