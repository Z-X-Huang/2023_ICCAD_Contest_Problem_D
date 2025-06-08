#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <cstdlib> 
#include <ctime>

using namespace std;

#include "structure.h"
#include "update.h"
#include "search.h"

void swap(int a, int b, vector<Tile*>& stl)
{
	Tile* temp_t = stl[a];
	stl[a] = stl[b];
	stl[b] = temp_t;
}

int main()
{
	int x = 0;
	int y = 0;
	int count = 0;

	float x1 = 0;
	float y1 = 0;

	int chip_width = 0;
	int chip_height = 0;
	int ti_width = 0;
	int ti_height = 0;

	int soft_num = 0;
	int fixed_num = 0;
	int con_num = 0;
	float avg_weight = 0;
	srand(time(NULL));	//time(NULL)1692450063
	cout << "seed " << time(NULL) << endl;

	string s = "NULL";
	string str = "NULL";

	vector<Tile*> soft_tile_list;
	vector<Tile*> fixed_tile_list;
	vector<Tile*> white_tile_list;

	ifstream file;
	file.open("out3.pl");

	ifstream file1;
	file1.open("circuit3.nodes");

	ifstream file2;
	file2.open("case03-input.txt");

	ofstream newFile2;
	newFile2.open("output.txt");


	file >> s >> s >> s;

	file1 >> str >> str >> str;
	file1 >> str >> chip_width >> chip_height;

	file1 >> str >> str >> soft_num;
	file1 >> str >> str >> fixed_num;
	soft_num -= fixed_num;

	file2 >> s >> s >> s >> s >> s;

	Plane* plane = new Plane(chip_width, chip_height);
	Plane* plane_best = new Plane(chip_width, chip_height);

	while (file >> s) {
		count++;
		if (count <= soft_num) {
			file >> x1 >> y1;
			file1 >> str >> ti_width >> ti_height;
			
			x = int(x1);
			y = int(y1);

			Point p_ll(x, y);
			Point p_ur(x + ti_width, y + ti_height);

			Tile* input_tile = new Tile(1, p_ll, p_ur, s);
			file2 >> s >> s;
			////////////////////讀檔小改 多讀最小面積 資料結構(Tile Trantile)多miniarea//////////////////////////
			input_tile->miniarea = stoi(s);
			//////////////////////////////////////////////////////////////////////////////////////
			soft_tile_list.push_back(input_tile);
			file >> s >> s;
		}
		else {
			file >> x1 >> y1;
			file1 >> str >> ti_width >> ti_height >> str;
			x = int(x1);
			y = int(y1);
			Point p_ll(x, y);
			Point p_ur(x + ti_width, y + ti_height);
			Rect input_rect(p_ll, p_ur);
			Tile* input_tile = new Tile();
			input_tile = InsertFixedTile(input_rect, plane);
			input_tile->ti_ll = p_ll;
			input_tile->ti_ur = p_ur;
			input_tile->ti_name = s;
			fixed_tile_list.push_back(input_tile);
			file >> s >> s;
		}
	}

	file2 >> s >> s;
	for (int i = 0;i < fixed_num;i++) {
		file2 >> s >> s >> s >> s >> s;
	}

	file2 >> s >> con_num;

	//Enumerate(plane, white_tile_list);


	for (int i = 0;i < con_num;i++)
	{
		string m1;
		string m2;
		int weight = 0;
		file2 >> m1 >> m2 >> weight;

		Tile* t1 = plane->pl_hint;
		Tile* t2 = plane->pl_hint;

		for (int j = 0;j < fixed_tile_list.size();j++)
		{
			if (!fixed_tile_list[j]->ti_name.compare(m1))
			{
				t1 = fixed_tile_list[j];
			}
			if (!fixed_tile_list[j]->ti_name.compare(m2))
			{
				t2 = fixed_tile_list[j];
			}
		}
		for (int j = 0;j < soft_tile_list.size();j++)
		{
			if (!soft_tile_list[j]->ti_name.compare(m1))
			{
				t1 = soft_tile_list[j];
			}
			if (!soft_tile_list[j]->ti_name.compare(m2))
			{
				t2 = soft_tile_list[j];
			}
		}
		avg_weight = avg_weight + weight;

		t1->name.push_back(t2);
		t1->weight.push_back(weight);

		t2->name.push_back(t1);
		t2->weight.push_back(weight);

	}
	avg_weight = avg_weight / con_num;

	for (int i = 0;i < soft_tile_list.size();i++)
	{
		cout << soft_tile_list[i]->ti_name << "\t" << soft_tile_list[i]->miniarea << "\t" << AREA(soft_tile_list[i]) << endl;
	}

	///////////////////////順序好像有改//////////////////////////
	plane->soft_tile_list = soft_tile_list;
	plane->fixed_tile_list = fixed_tile_list;
	plane_best->fixed_tile_list = fixed_tile_list;


	InsertSoftTile(plane, plane->soft_tile_list, 1);
	plane_best->hpwl = plane->hpwl;
	plane_best->soft_tile_list = plane->soft_tile_list;

	/////////////////////////////////////////////////////////////////////

	for (int i = 0; i < 50000; i++) 
	{
		/*for (int j = 0; j < 5; j++) {
			int te1 = rand() % soft_num;
			int te2 = rand() % soft_num;
			Tile* temp_t = plane->soft_tile_list[te1];
			plane->soft_tile_list[te1] = plane->soft_tile_list[te2];
			plane->soft_tile_list[te2] = temp_t;
		}*/
		//cout << i << endl;

		int first = rand() % soft_num;
		swap(first, 0, plane->soft_tile_list);
		for (int j = 1;j < plane->soft_tile_list.size();j++)
		{
			int pre = j - 1;

			vector<string> priority;
			for (int k = 0;k < plane->soft_tile_list[pre]->weight.size();k++)
			{
				if (plane->soft_tile_list[pre]->weight[k] > avg_weight)
				{
					priority.push_back(plane->soft_tile_list[pre]->name[k]->ti_name);
				}
			}

			int a = -1;
			if (priority.size() != 0)
			{
				int next = rand() % priority.size();
				for (int k = j;k < plane->soft_tile_list.size();k++)
				{
					if (plane->soft_tile_list[k]->ti_name == priority[next])
					{
						a = k;
					}
				}
			}

			if (a == -1)
			{
				a = j + rand() % (plane->soft_tile_list.size() - j);
			}
			swap(a, j, plane->soft_tile_list);
		}

		InsertSoftTile(plane, plane->soft_tile_list, 0);
		if (plane->legal != 0 && plane->hpwl < plane_best->hpwl) {
			plane_best = plane;
			newFile2 << plane_best->hpwl << endl;
			cout << i << "\t" << plane_best->hpwl << " " << plane_best->soft_tile_list[0]->miniarea << endl;
		}

		plane = new Plane(plane_best->pl_width, plane_best->pl_height);
		for (int j = 0; j < fixed_tile_list.size(); j++) {
			Point p_ll(fixed_tile_list[j]->ti_ll.p_x, fixed_tile_list[j]->ti_ll.p_y);
			Point p_ur(fixed_tile_list[j]->ti_ur.p_x, fixed_tile_list[j]->ti_ur.p_y);
			Rect input_rect(p_ll, p_ur);
			Tile* input_tile = new Tile();
			input_tile = InsertFixedTile(input_rect, plane);
		}
		plane->soft_tile_list = soft_tile_list;
		plane->fixed_tile_list = fixed_tile_list;
	}

	cout << "best HPWL : " << plane_best->hpwl << endl;

	vector<Trantile> soft_tile_list2;
	int cccc = 0;
	for (int i = 0;i < plane_best->soft_tile_list.size();i++)
	{
		Trantile temp;
		temp.frame_mid = plane_best->soft_tile_list[i]->ti_mid;
		temp.t1 = plane_best->soft_tile_list[i];
		temp.t2 = nullptr;
		temp.ti_name = plane_best->soft_tile_list[i]->ti_name;
		temp.weight = plane_best->soft_tile_list[i]->weight;
		temp.totalweight = 0;
		temp.miniarea = plane_best->soft_tile_list[i]->miniarea;
		//cout << i << " " << temp.miniarea << endl;
		
		for (int j = 0;j < plane_best->soft_tile_list[i]->name.size();j++)
		{
			temp.totalweight = temp.totalweight + plane_best->soft_tile_list[i]->weight[j];
			cccc++;
			for (int k = 0; k < fixed_tile_list.size(); k++)
			{
				if (plane_best->soft_tile_list[i]->name[j]->ti_name == fixed_tile_list[k]->ti_name)
				{
					temp.type.push_back(0);
					temp.name.push_back(k);
				}
			}
			for (int k = 0; k < soft_tile_list.size(); k++)
			{
				if (plane_best->soft_tile_list[i]->name[j]->ti_name == plane_best->soft_tile_list[k]->ti_name)
				{
					temp.type.push_back(1);
					temp.name.push_back(k);
				}
			}
		}
		soft_tile_list2.push_back(temp);
	}

	//////////////////////////////////新function Replace/////////////////////
	
	double hpwl = 0;
	cout << "---------Replace----------" << endl;

	int aaaa = 0;
	double prehpwl = plane_best->hpwl;
	while (aaaa < 10)
	{
		hpwl = 0;
		Replace(plane_best, soft_tile_list2, fixed_tile_list);
		for (int i = 0;i < soft_tile_list2.size();i++)
		{
			//cout << point_cost_tran(soft_tile_list2, i, fixed_tile_list, soft_tile_list2[i].frame_mid, 1) << endl;
			hpwl = hpwl + point_cost_tran(soft_tile_list2, i, fixed_tile_list, soft_tile_list2[i].frame_mid, 1);
		}
		hpwl = hpwl / 2;
		cout << aaaa << "\t" << hpwl << endl;

		if (hpwl == prehpwl)
			break;

		prehpwl = hpwl;
		aaaa++;
	}
	///////////////////////////////////////////////////////////////////////////////

	/*cout << "---------Special_Transform----------" << endl;
	
	hpwl = 0;
	Special_Transform(plane_best, soft_tile_list2, fixed_tile_list);
	for (int i = 0;i < soft_tile_list2.size();i++)
	{
		hpwl = hpwl + point_cost_tran(soft_tile_list2, i, fixed_tile_list, soft_tile_list2[i].frame_mid, 1);
	}
	hpwl = hpwl / 2;
	cout << "result\t" << hpwl << endl;*/
	Enumerate(plane_best, white_tile_list);

	ofstream newfile4;
	newfile4.open("case5r.m");
	newfile4<< "axis equal;\n" << "hold on;\n" << "grid on;\n";
	newfile4 << "block_x=[0 0 " << chip_width << " " << chip_width << " 0];" << endl;
	newfile4 << "block_y=[0 " << chip_height << " " << chip_height << " 0 0];" << endl;
	newfile4 << "fill(block_x, block_y, 'c');" << endl;

	for (int i = 0; i < white_tile_list.size(); i++)
	{
		newfile4 << "block_x=[" << LEFT(white_tile_list[i]) << " " << LEFT(white_tile_list[i]) << " ";
		newfile4 << RIGHT(white_tile_list[i]) << " " << RIGHT(white_tile_list[i]) << " " << LEFT(white_tile_list[i]) << "];" << endl;
		newfile4 << "block_y=[" << BOTTOM(white_tile_list[i]) << " " << TOP(white_tile_list[i]) << " ";
		newfile4 << TOP(white_tile_list[i]) << " " << BOTTOM(white_tile_list[i]) << " " << BOTTOM(white_tile_list[i]) << "];" << endl;
		newfile4 << "fill(block_x, block_y, 'w');" << endl;
	}

	for (int i = 0; i < fixed_tile_list.size(); i++)
	{
		newfile4 << "block_x=[" << LEFT(fixed_tile_list[i]) << " " << LEFT(fixed_tile_list[i]) << " ";
		newfile4 << RIGHT(fixed_tile_list[i]) << " " << RIGHT(fixed_tile_list[i]) << " " << LEFT(fixed_tile_list[i]) << "];" << endl;
		newfile4 << "block_y=[" << BOTTOM(fixed_tile_list[i]) << " " << TOP(fixed_tile_list[i]) << " ";
		newfile4 << TOP(fixed_tile_list[i]) << " " << BOTTOM(fixed_tile_list[i]) << " " << BOTTOM(fixed_tile_list[i]) << "];" << endl;
		newfile4 << "fill(block_x, block_y, 'y');" << endl;
	}

	if (plane_best->legal)
	{
		for (int i = 0;i < soft_tile_list2.size();i++)
		{
			if (soft_tile_list2[i].t2 == nullptr)
			{
				newfile4 << "block_x=[" << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
				newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << "];" << endl;
				newfile4 << "block_y=[" << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
				newfile4 << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
			}
			else
			{
				if (soft_tile_list2[i].dir == 1)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newfile4 << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 2)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newfile4 << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 3)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newfile4 << TOP(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newfile4 << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 4)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newfile4 << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 5)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newfile4 << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 6)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t2) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newfile4 << TOP(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newfile4 << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 7)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t2) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newfile4 << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 8)
				{
					newfile4 << "block_x=[";
					newfile4 << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newfile4 << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newfile4 << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newfile4 << "block_y=[";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newfile4 << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newfile4 << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newfile4 << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
			}
			newfile4 << "fill(block_x, block_y, 'g');" << endl;
			newfile4 << "text(" << soft_tile_list2[i].t1->ti_ll.p_x << "," << soft_tile_list2[i].frame_mid.p_y;
			newfile4 << ",'" << soft_tile_list2[i].ti_name << "')" << endl;
		}
	}

	cout << "---------Transform----------" << endl;
	int ccccc = 0;
	prehpwl = hpwl;
	hpwl = 0;
	while (1)
	{
		hpwl = 0;
		Transform(plane_best, soft_tile_list2, fixed_tile_list);
		for (int i = 0;i < soft_tile_list2.size();i++)
		{
			hpwl = hpwl + point_cost_tran(soft_tile_list2, i, fixed_tile_list, soft_tile_list2[i].frame_mid, 1);
		}
		hpwl = hpwl / 2;
		cout << ccccc << "\t" << hpwl << endl;

		if (hpwl == prehpwl)
			break;

		prehpwl = hpwl;
		ccccc++;
	}

	Enumerate(plane_best, white_tile_list);

	ofstream newFile;
	newFile.open("case5t.m");
	newFile << "axis equal;\n" << "hold on;\n" << "grid on;\n";
	newFile << "block_x=[0 0 " << chip_width << " " << chip_width << " 0];" << endl;
	newFile << "block_y=[0 " << chip_height << " " << chip_height << " 0 0];" << endl;
	newFile << "fill(block_x, block_y, 'c');" << endl;
	
	for (int i = 0; i < white_tile_list.size(); i++)
	{
		newFile << "block_x=[" << LEFT(white_tile_list[i]) << " " << LEFT(white_tile_list[i]) << " ";
		newFile << RIGHT(white_tile_list[i]) << " " << RIGHT(white_tile_list[i]) << " " << LEFT(white_tile_list[i]) << "];" << endl;
		newFile << "block_y=[" << BOTTOM(white_tile_list[i]) << " " << TOP(white_tile_list[i]) << " ";
		newFile << TOP(white_tile_list[i]) << " " << BOTTOM(white_tile_list[i]) << " " << BOTTOM(white_tile_list[i]) << "];" << endl;
		newFile << "fill(block_x, block_y, 'w');" << endl;
	}

	for (int i = 0; i < fixed_tile_list.size(); i++)
	{
		newFile << "block_x=[" << LEFT(fixed_tile_list[i]) << " " << LEFT(fixed_tile_list[i]) << " ";
		newFile << RIGHT(fixed_tile_list[i]) << " " << RIGHT(fixed_tile_list[i]) << " " << LEFT(fixed_tile_list[i]) << "];" << endl;
		newFile << "block_y=[" << BOTTOM(fixed_tile_list[i]) << " " << TOP(fixed_tile_list[i]) << " ";
		newFile << TOP(fixed_tile_list[i]) << " " << BOTTOM(fixed_tile_list[i]) << " " << BOTTOM(fixed_tile_list[i]) << "];" << endl;
		newFile << "fill(block_x, block_y, 'y');" << endl;
	}

	if (plane_best->legal)
	{
		for (int i = 0;i < soft_tile_list2.size();i++)
		{
			if (soft_tile_list2[i].t2 == nullptr)
			{
				newFile << "block_x=[" << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
				newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << "];" << endl;
				newFile << "block_y=[" << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
				newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
			}
			else
			{
				if (soft_tile_list2[i].dir == 1)
				{
					newFile << "block_x=[";
					newFile	<< LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile	<< LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile	<< LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 2)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 3)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 4)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 5)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 6)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t1) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 7)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t2) << " " << LEFT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t2) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t2) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t2) << "];" << endl;
				}
				else if (soft_tile_list2[i].dir == 8)
				{
					newFile << "block_x=[";
					newFile << LEFT(soft_tile_list2[i].t1) << " " << LEFT(soft_tile_list2[i].t1) << " ";
					newFile << RIGHT(soft_tile_list2[i].t2) << " " << RIGHT(soft_tile_list2[i].t2) << " ";
					newFile << RIGHT(soft_tile_list2[i].t1) << " " << RIGHT(soft_tile_list2[i].t1) << " ";
					newFile << LEFT(soft_tile_list2[i].t1) << "];" << endl;
					newFile << "block_y=[";
					newFile << BOTTOM(soft_tile_list2[i].t1) << " " << TOP(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t2) << " " << BOTTOM(soft_tile_list2[i].t2) << " ";
					newFile << TOP(soft_tile_list2[i].t1) << " " << BOTTOM(soft_tile_list2[i].t1) << " ";
					newFile << BOTTOM(soft_tile_list2[i].t1) << "];" << endl;
				}
			}
			newFile << "fill(block_x, block_y, 'g');" << endl;
			newFile << "text(" << soft_tile_list2[i].t1->ti_ll.p_x << "," << soft_tile_list2[i].frame_mid.p_y;
			newFile << ",'" << soft_tile_list2[i].ti_name << "')" << endl;
		}
	}

	ofstream newFile3;
	newFile3.open("case05-output.txt");
	newFile3 << "HPWL " << hpwl << endl;
	newFile3 << "SOFTMODULE " << soft_tile_list2.size() << endl;
	for (int i = 0;i < soft_tile_list2.size();i++)
	{
		newFile3 << soft_tile_list2[i].ti_name << " ";
		if (soft_tile_list2[i].t2 == nullptr)
		{
			newFile3 << "4" << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
			newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
		}
		else
		{
			newFile3 << "6" << endl;
			if (soft_tile_list2[i].dir == 1)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 2)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 3)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 4)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 5)
			{
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 6)
			{
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 7)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
			else if (soft_tile_list2[i].dir == 8)
			{
				newFile3 << soft_tile_list2[i].t1->ti_ll.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ll.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t2->ti_ur.p_x << " " << soft_tile_list2[i].t2->ti_ll.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ur.p_y << endl;
				newFile3 << soft_tile_list2[i].t1->ti_ur.p_x << " " << soft_tile_list2[i].t1->ti_ll.p_y << endl;
			}
		}
	}

	return 0;
}
