#include "multithread.h"



void legalization(Plane**result, int thread_id, string case_file_name, int t)
{
	
	float avg_weight = 0;
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

	int rand_num = time(NULL) + pow(thread_id,2);


	string s = "NULL";
	string str = "NULL";

	vector<Tile*> soft_tile_list;
	vector<Tile*> fixed_tile_list;
	vector<Tile*> white_tile_list;



	ifstream file;
	file.open("out.ntup.pl");

	ifstream file1;
	file1.open("circuit.nodes");

	ifstream file2;
	file2.open(case_file_name);

	/*ofstream newFile2;
	newFile2.open("output.txt");*/


	file >> s >> s >> s;

	file1 >> str >> str >> str;
	file1 >> str >> chip_width >> chip_height;

	file1 >> str >> str >> soft_num;
	file1 >> str >> str >> fixed_num;
	soft_num -= fixed_num;

	file2 >> s >> s >> s >> s >> s;
	for (int i = 0; i < soft_num; i++) {
		file2 >> s >> s;
	}
	file2 >> s >> s;
	for (int i = 0; i < fixed_num; i++) {
		file2 >> s >> s >> s >> s >> s;
	}

	file2 >> s >> con_num;

	Plane *plane = new Plane(chip_width, chip_height);
	Plane *plane_best = new Plane(chip_width, chip_height);

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
			Tile *input_tile = new Tile();
			input_tile = InsertFixedTile(input_rect, plane);
			input_tile->ti_ll = p_ll;
			input_tile->ti_ur = p_ur;
			input_tile->ti_name = s;
			fixed_tile_list.push_back(input_tile);
			file >> s >> s;
		}
	}
	plane->soft_tile_list = soft_tile_list;
	plane->fixed_tile_list = fixed_tile_list;
	plane_best->fixed_tile_list = fixed_tile_list;

	//Enumerate(plane, white_tile_list);

	vector<pair<vector<connect_info>, Rect>> soft_global_result;
	vector<pair<vector<connect_info>, string>> fixed_global_result;

	for (int i = 0; i < con_num; i++)
	{
		string m1;
		string m2;
		int weight = 0;
		file2 >> m1 >> m2 >> weight;

		Tile* t1 = plane->pl_hint;
		Tile* t2 = plane->pl_hint;

		for (int j = 0; j < fixed_tile_list.size(); j++)
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
		for (int j = 0; j < soft_tile_list.size(); j++)
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
		t1->name.push_back(read_connection(t2));
		t1->weight.push_back(weight);

		t2->name.push_back(read_connection(t1));
		t2->weight.push_back(weight);

	}

	for (int i = 0; i < soft_tile_list.size(); i++)
	{
		pair<vector<connect_info>, Rect> p;
		p.first = soft_tile_list[i]->name;
		p.second.r_ll = soft_tile_list[i]->ti_ll;
		p.second.r_ur = soft_tile_list[i]->ti_ur;
		p.second.r_mid.p_x = (p.second.r_ll.p_x + p.second.r_ur.p_x) / 2;
		p.second.r_mid.p_y = (p.second.r_ll.p_y + p.second.r_ur.p_y) / 2;
		soft_global_result.push_back(p);
	}
	for (int i = 0; i < fixed_tile_list.size(); i++)
	{
		pair<vector<connect_info>, string> p;
		p.first = fixed_tile_list[i]->name;
		p.second = fixed_tile_list[i]->ti_name;
		fixed_global_result.push_back(p);
	}




	file.close();
	file1.close();
	file2.close();


	//2023/8/19
	//changed to initializing with global connection result 
	//in the begining of every iteration  
	cout << "========Legaliation Start========" << endl;
	cout << "[" << thread_id+1 << "]" << "random seed: " << rand_num << endl; cout << endl;
	srand(rand_num);
	//srand(1693378147);
	//srand(1693019566);
	//srand(1692952807);//good one
	//srand(1692788093);
	//srand(1692543947);
	//srand(1692515686);
	//srand(1692502311);
	//srand(1691656314);

	vector<pair<vector<Tile*>, double>> population;

	avg_weight = avg_weight / con_num;
	InsertSoftTile(plane, plane->soft_tile_list, 0);

	plane_best = plane;


	cout << "<stage 1> random iteration" << endl; cout << endl;

	int iteration_time = 0, exchange_time = 10, run_time = t;
	cout << "run time: " << double(run_time)/60.0 << "mins" << endl;
	cout << "exchange time: " << exchange_time << endl; cout << endl;
	

	
	double start_time = double(time(NULL));

	while (double(time(NULL)) - (double(start_time)) < run_time) {
		//cout<<double(time(NULL)) - (double(start_time))<<endl;
		plane = Build_newPlane(plane_best, soft_tile_list, fixed_tile_list);
		//cout << i << endl;


		//--use global placement result in every iteration--
		//read_global_result(plane, soft_global_result, fixed_global_result);
		//random_arrange_order(exchange_time, plane, soft_tile_list, soft_global_result);
		int first = rand() % soft_num;
		swap(plane->soft_tile_list[first], plane->soft_tile_list[0]);
		for (int j = 1; j < plane->soft_tile_list.size(); j++)
		{
			int pre = j - 1;

			vector<string> priority;
			for (int k = 0; k < plane->soft_tile_list[pre]->weight.size(); k++)
			{
				if (plane->soft_tile_list[pre]->weight[k] > avg_weight)
				{
					priority.push_back(plane->soft_tile_list[pre]->name[k].ti_name);
				}
			}

			int a = -1;
			if (priority.size() != 0)
			{
				int next = rand() % priority.size();
				for (int k = j; k < plane->soft_tile_list.size(); k++)
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
			swap(plane->soft_tile_list[a], plane->soft_tile_list[j]);
		}


		pair<vector<Tile*>, double> candidate;
		candidate.first = generate_candidate(plane);

		InsertSoftTile(plane, plane->soft_tile_list, 0);



		if (plane->legal != 0 && plane->hpwl < plane_best->hpwl) {
			Plane* tp_plane = plane_best;
			plane_best = plane;

			candidate.second = plane_best->hpwl;
			population.push_back(candidate);

			tile_free_memory(tp_plane, soft_tile_list, fixed_tile_list);
			delete tp_plane;

			cout << "iteration " << iteration_time << " succeed, hpwl=" << plane_best->hpwl << endl;

		}
		else {
			if (plane->legal != 0 && plane->hpwl < plane_best->hpwl*1.05)
			{
				candidate.second = plane->hpwl;
				population.push_back(candidate);
			}
			else {
				for (int k = 0; k < candidate.first.size(); k++)
					delete candidate.first[k];
			}

			/*if (i == 3736)
			{
				cout << "wewewewewew" << endl;
			}*/

			tile_free_memory(plane, soft_tile_list, fixed_tile_list);
			delete plane;

		}
		iteration_time++;
	}

	cout << "result: " << calc_hpwl(plane_best) << endl; 
	cout << "total iteration time: " << iteration_time << endl; cout << endl;
	cout<<"run time:"<<(double(time(NULL)) - (double(start_time)))/60<<endl;

	cout << "<stage 2> genetic algorithm" << endl; cout << endl;



	int generation_num = 10;
	int individual_num;
	set<double> hpwl_set;


	for (int i = 0; i < generation_num; i++)
	{
		if (int(sqrt(iteration_time/10)) < population.size())
		{
			individual_num = int(sqrt(iteration_time/10));
		}
		else
		{
			individual_num = population.size();
		}


		cout << "generation " << i + 1 << ", population size:" << individual_num << endl;
		cout << endl;

		sort_population(population);

		if (population.size() > individual_num)
		{
			shrink_population(individual_num, population);
		}


		//crossover
		for (int j = 0; j < individual_num; j++)
		{
			for (int k = 0; k < individual_num; k++)
			{
				if (k == j) continue;

				
				plane = Build_newPlane(plane_best, soft_tile_list, fixed_tile_list);
				crossover(population[j].first, population[k].first, plane->soft_tile_list);

				pair<vector<Tile*>, double> candidate;
				candidate.first = generate_candidate(plane);
				
				InsertSoftTile(plane, plane->soft_tile_list, 0);

				if (plane->legal != 0 && plane->hpwl < plane_best->hpwl) {
					Plane* tp_plane = plane_best;
					plane_best = plane;

					candidate.second = plane_best->hpwl;
					hpwl_set.insert(candidate.second);
					population.push_back(candidate);

					tile_free_memory(tp_plane, soft_tile_list, fixed_tile_list);
					delete tp_plane;


					cout << "crossover succeed, hpwl=" << plane_best->hpwl << endl;

				}
				else {
					if (plane->legal != 0 && plane->hpwl < plane_best->hpwl*1.05 && hpwl_set.count(plane->hpwl) == 0)
					{
						candidate.second = plane->hpwl;
						hpwl_set.insert(candidate.second);
						population.push_back(candidate);
					}
					else {
						for (int k = 0; k < candidate.first.size(); k++)
							delete candidate.first[k];
					}
					tile_free_memory(plane, soft_tile_list, fixed_tile_list);
					delete plane;
					plane = NULL;
				}
			}
		}

		//mutation
		for (int j = 0; j < individual_num; j++)
		{
		
			plane = Build_newPlane(plane_best, soft_tile_list, fixed_tile_list);
			
			mutation(population[j].first, plane->soft_tile_list);
			
			pair<vector<Tile*>, double> candidate;
			candidate.first = generate_candidate(plane);
			
			InsertSoftTile(plane, plane->soft_tile_list, 0);

			if (plane->legal != 0 && plane->hpwl < plane_best->hpwl) {
				Plane* tp_plane = plane_best;
				plane_best = plane;

				candidate.second = plane_best->hpwl;
				hpwl_set.insert(candidate.second);
				population.push_back(candidate);

				tile_free_memory(tp_plane, soft_tile_list, fixed_tile_list);
				delete tp_plane;


				cout << "mutation succeed, hpwl=" << plane_best->hpwl << endl;

			}
			else {
				if (plane->legal != 0 && plane->hpwl < plane_best->hpwl*1.02 && hpwl_set.count(plane->hpwl) == 0)
				{
					candidate.second = plane->hpwl;
					hpwl_set.insert(candidate.second);
					population.push_back(candidate);
				}
				else {
					for (int k = 0; k < candidate.first.size(); k++)
						delete candidate.first[k];
				}
				tile_free_memory(plane, soft_tile_list, fixed_tile_list);
				delete plane;
				plane = NULL;
			}
		}
	}


	*result = plane_best;

	
	return;
}


vector<Trantile> transform_function(Plane* &plane_result)
{
	for (int i = 0; i < plane_result->soft_tile_list.size(); i++)
	{
		Point tp_mid((plane_result->soft_tile_list[i]->ti_ll.p_x + plane_result->soft_tile_list[i]->ti_ur.p_x) / 2
			, (plane_result->soft_tile_list[i]->ti_ll.p_y + plane_result->soft_tile_list[i]->ti_ur.p_y) / 2);

		plane_result->soft_tile_list[i]->ti_mid = tp_mid;
	}

	for (int i = 0; i < plane_result->fixed_tile_list.size(); i++)
	{
		Point tp_mid((plane_result->fixed_tile_list[i]->ti_ll.p_x + plane_result->fixed_tile_list[i]->ti_ur.p_x) / 2
			, (plane_result->fixed_tile_list[i]->ti_ll.p_y + plane_result->fixed_tile_list[i]->ti_ur.p_y) / 2);

		plane_result->fixed_tile_list[i]->ti_mid = tp_mid;
	}



	vector<Trantile> soft_tile_list2;
	int cccc = 0;
	for (int i = 0; i < plane_result->soft_tile_list.size(); i++)
	{
		Trantile temp;
		temp.frame_mid = plane_result->soft_tile_list[i]->ti_mid;
		temp.t1 = plane_result->soft_tile_list[i];
		temp.t2 = nullptr;
		temp.ti_name = plane_result->soft_tile_list[i]->ti_name;
		temp.weight = plane_result->soft_tile_list[i]->weight;


		temp.totalweight = 0;

		for (int j = 0; j < plane_result->soft_tile_list[i]->name.size(); j++)
		{
			temp.totalweight = temp.totalweight + plane_result->soft_tile_list[i]->weight[j];
			cccc++;
			for (int k = 0; k < plane_result->fixed_tile_list.size(); k++)
			{
				if (plane_result->soft_tile_list[i]->name[j].ti_name == plane_result->fixed_tile_list[k]->ti_name)
				{
					temp.type.push_back(0);
					temp.name.push_back(k);
				}
			}
			for (int k = 0; k < plane_result->soft_tile_list.size(); k++)
			{
				if (plane_result->soft_tile_list[i]->name[j].ti_name == plane_result->soft_tile_list[k]->ti_name)
				{
					temp.type.push_back(1);
					temp.name.push_back(k);
				}
			}
		}
		soft_tile_list2.push_back(temp);
	}



	//cout << "---------ForceDirected----------" << endl;
	//Special_Transform(plane_best, soft_tile_list2, fixed_tile_list);

	cout << "---------Transform----------" << endl;
	int ccccc = 0;
	double prehpwl = plane_result->hpwl;
	double hpwl = 0;
	while (1)
	{
		hpwl = 0;
		Transform(plane_result, soft_tile_list2, plane_result->fixed_tile_list);
		for (int i = 0; i < soft_tile_list2.size(); i++)
		{
			//cout << point_cost_tran(soft_tile_list2, i, fixed_tile_list, soft_tile_list2[i].frame_mid, 1) << endl;
			hpwl = hpwl + point_cost_tran(soft_tile_list2, i, plane_result->fixed_tile_list, soft_tile_list2[i].frame_mid, 1);
		}
		hpwl = hpwl / 2;
		cout << ccccc << "\t" << hpwl << endl;

		if (hpwl == prehpwl)
			break;

		prehpwl = hpwl;
		ccccc++;
	}

	plane_result->hpwl = hpwl;

	return soft_tile_list2;
}
