
#include<string>
#include<iostream>
#include<fstream>
#include<time.h>
#include<map>
#include<memory.h>
using namespace std;

const int POPULATION_NUM = 10;		 //种群数据设为10
const int MAX_TABU_ITER = 10000;	 //tabu search最多的迭代次数
const int MAX_CROSS_NUM = 3000;
const int SOL = POPULATION_NUM + 1;	 //存储解

//#define SHOWBUG

int Ver_num, Col_num;
int Seed;
int **NB_ID; //顶点数*(顶点数-1)，保存邻点有哪些
int* AdjVer_num; //保存邻点个数
int** Sol_methods;//保存解 [第i个解][第j个顶点] 即p*N
int** AdjVer_Col_num;//邻接颜色表
int** Tabu_tenure;

int* Color_num;//保存每种颜色的个数，大小为K
int* s;
int Burden;//当前种群的f
void Tabu(int p);

void Initialize(string fileName)
{
	ifstream file;
	file.open(fileName);
	string str;
	int tmp;
	file >> str;
	while (!file.eof())
	{
		if (str == "edge")
		{
			file >> Ver_num;
			//初始化NbID为N*(N-1)数组和num_adj
			AdjVer_num = new int[Ver_num];
			NB_ID = new int*[Ver_num];
			for (int i = 0; i < Ver_num; i++)
			{
				AdjVer_num[i] = 0;
				NB_ID[i] = new int[Ver_num - 1];
				for (int j = 0; j < Ver_num - 1; j++) NB_ID[i][j] = 0;
			}
		}
		if (str == "e")
		{
			int m, n;  //顶点的ID
			file >> m >> n;
			m--;
			n--;
			//保存邻边信息
			tmp = AdjVer_num[m];
			NB_ID[m][tmp] = n;
			AdjVer_num[m]++;

			tmp = AdjVer_num[n];
			NB_ID[n][tmp] = m;
			AdjVer_num[n]++;
		}
		file >> str;
	}
	file.close();
	srand(Seed);

	int* h_sol;
	Sol_methods = new int*[POPULATION_NUM + 1];
	for (size_t i = 0; i < POPULATION_NUM + 1; i++)
	{
		Sol_methods[i] = new int[Ver_num];
	}
	for (int i = 0; i < POPULATION_NUM; i++)
	{
		h_sol = Sol_methods[i];
		for (int j = 0; j < Ver_num; j++)
			h_sol[j] = rand() % Col_num;
	}

	//开辟邻接颜色表
	AdjVer_Col_num = new int*[Ver_num];
	for (int i = 0; i < Ver_num; i++)
		AdjVer_Col_num[i] = new int[Col_num];
	//开辟tt
	Tabu_tenure = new int*[Ver_num];
	for (int i = 0; i < Ver_num; i++)
		Tabu_tenure[i] = new int[Col_num];
	//开辟color_num
	Color_num = new int[Col_num];


	for (int i = 0; i < POPULATION_NUM; i++)
	{
		Tabu(i);
		if (!Burden) break;//重要
	}
}

//禁忌算法

int best_f = 10000;
int f[SOL] = { 10000 };
void Initialize_()//计算邻接颜色表
{
	//int* s = sol[p];  //第p个种群
	for (int i = 0; i < Ver_num; i++)
		for (int j = 0; j < Col_num; j++)
			AdjVer_Col_num[i][j] = 0;

	int* h_adj_table;
	int* h_NbID;
	int edge_v;//邻点
	int edge_v_color;//邻点颜色
	int c_color;//当前顶点的颜色

	Burden = 0;
	for (int i = 0; i < Ver_num; i++)
	{
		h_adj_table = AdjVer_Col_num[i];
		h_NbID = NB_ID[i];
		c_color = s[i];
		for (int j = 0; j < AdjVer_num[i]; j++)
		{
			edge_v = h_NbID[j];//得到邻点ID、
			edge_v_color = s[edge_v];//得到邻点颜色
			h_adj_table[edge_v_color]++;
			if (c_color == edge_v_color) Burden++;
		}
	}
	Burden = Burden / 2;
	best_f = Burden;

	for (int i = 0; i < Ver_num; i++)
		for (int j = 0; j < Col_num; j++)
			Tabu_tenure[i][j] = 0;
}

int delt;
int iter;
int Chg_methods_num;
int Chg_methods[2000][2];
void TryTail()
{
	delt = 10000;//初始为最大整数
	int tmp_delt;
	int Cur_color;//当前结点颜色
	int *h_color;//邻接颜色表行首指针
	int *h_tabu;//禁忌表行首指针
	int c_color_table;//当前结点邻接颜色表的值
	for (int i = 0; i < Ver_num; i++) {
		Cur_color = s[i];
		h_color = AdjVer_Col_num[i];
		c_color_table = h_color[Cur_color];//即adj_color_table[i][sol[i]]的值
		if (c_color_table > 0) {  //颜色表此处的值不为0
			h_tabu = Tabu_tenure[i];
			for (int j = 0; j < Col_num; j++) {
				if (Cur_color != j) { //如果颜色不相同
					tmp_delt = h_color[j] - c_color_table;
					if (tmp_delt <= delt && (iter > h_tabu[j] || (tmp_delt + Burden) < best_f)) {
						if (tmp_delt < delt) {//当前解小于本次迭代最优解,则重新开始保存解

							Chg_methods_num = 0;
							delt = tmp_delt;

						}
						Chg_methods[Chg_methods_num][0] = i;
						Chg_methods[Chg_methods_num][1] = j;
						Chg_methods_num++;
					}
				}
			}
		}
	}
}
void TakeMove()
{
	int tmp_id = rand() % Chg_methods_num;//有多个解时，随机选择
	int sel_vertex = Chg_methods[tmp_id][0];
	int sel_color = Chg_methods[tmp_id][1];
	Burden = delt + Burden;
	if (Burden < best_f) best_f = Burden;
	int old_color = s[sel_vertex];
	s[sel_vertex] = sel_color; //更新颜色
	Tabu_tenure[sel_vertex][old_color] = iter + Burden + (rand() % 10) + 1;//更新禁忌表，是否最后要+1？？
																 //还要有数组存下每个顶点的邻边，以及邻边的数量
	int* h_NbID = NB_ID[sel_vertex];
	int num_edge = AdjVer_num[sel_vertex];
	int tmp;
	for (int i = 0; i < num_edge; i++) {//更新邻接颜色表
		tmp = h_NbID[i];//等于nbID[sel_vertex][i]
		AdjVer_Col_num[tmp][old_color]--;
		AdjVer_Col_num[tmp][sel_color]++;
	}
}

void Tabu(int p) //输入的是第p个种群
{
	s = Sol_methods[p];//指向当前种群
	Burden = f[p];//赋值当前种群的f，最后再赋值回去
	Initialize_();
	iter = 0;
	while (iter < MAX_TABU_ITER && best_f)
	{
		TryTail();
		TakeMove();
		iter++;
	}
	f[p] = Burden;

}

map<int, int> s1, s2, *h_s;
map<int, int>::iterator it, itTmp;
int max_equ_count, max_k[200];
void CrossOver()
{
	max_equ_count = 0;
	int count_s = 0;
	int m, n;
	m = rand() % POPULATION_NUM;
	while ((n = rand() % POPULATION_NUM) == m); //避免选了两个一样的种群

	int* h_s1 = Sol_methods[m], *h_s2 = Sol_methods[n];
	for (int i = 0; i < Ver_num; i++)
	{
		s1[i] = h_s1[i];
		s2[i] = h_s2[i];
	}

	bool flag = false;
	int c_color, c_v;
	for (int l = 0; l < Col_num; l++)
	{
		memset(Color_num, 0, Col_num * sizeof(int));//每次都要清零
		if (flag)
		{
			flag = false;
			h_s = &s1;
		}
		else
		{
			flag = true;
			h_s = &s2;
		}

		it = h_s->begin();
		while (it != h_s->end())
		{
			c_color = it->second;
			Color_num[c_color]++;
			it++;
		}
		int tmp_max = -1;

		for (int i = 0; i < Col_num; i++)
		{
			if (tmp_max <= Color_num[i])
			{
				if (tmp_max < Color_num[i])
				{
					tmp_max = Color_num[i];//保存下所有的最大card的颜色然后随机选一个

					max_equ_count = 0;
				}
				max_k[max_equ_count] = i;
				max_equ_count++;
			}
		}
		int max_card;//最大种群的颜色
		max_card = max_k[rand() % max_equ_count];//从最大颜色中随机选了一个


		it = h_s->begin();
		while (it != h_s->end())
		{

			c_color = it->second;
			if (c_color == max_card)
			{
				c_v = it->first;
				it++;

				s1.erase(c_v);
				s2.erase(c_v);
				Sol_methods[SOL - 1][c_v] = l;
			}
			else it++;
		}

	}

	it = s1.begin();
	while (it != s1.end())
	{
		c_v = it->first;
		Sol_methods[SOL - 1][c_v] = rand() % Col_num;//随机设
		it++;
	}

}


int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "para error." << endl;
        return -1;
    }
    string Input_path = argv[1];
    string Output_path = argv[2];
    string Col_num_str = argv[3];
    string Seed_str = argv[4];
    for (int i = 0; i < Seed_str.length(); i++) {
        int temp = Seed_str[i] - '0';
        for (int j = 0; j < Seed_str.length() - 1 - i; j++)
            temp *= 10;
        Seed += temp;
    }
    Col_num = (Col_num_str[0] - '0') * 10 + (Col_num_str[1] - '0');
	int min_f = INT16_MAX;

	
		Initialize(Input_path);
	for (int i = 0; i < POPULATION_NUM; i++)
		if (min_f > f[i])
			min_f = f[i];

	int cross_num = 0;
	int equ_worst_f[2000], tmp_count;

	clock_t start, end;
	start = clock();
	while (cross_num <= MAX_CROSS_NUM && min_f)
	{
		CrossOver();
		Tabu(SOL - 1);
		if (f[SOL - 1] < min_f) //s0<S*
			min_f = f[SOL - 1];

		int tmp_worst = -1;
		int worst_p;//最差的种群
		for (int i = 0; i < POPULATION_NUM; i++) //找出最差解
		{
			if (tmp_worst <= f[i])
			{
				if (tmp_worst < f[i])
				{
					tmp_worst = f[i];
					tmp_count = 0;
				}
				equ_worst_f[tmp_count] = i;
				tmp_count++;
			}
		}
		worst_p = equ_worst_f[rand() % tmp_count];
		memcpy(Sol_methods[worst_p], Sol_methods[SOL - 1], Ver_num * sizeof(int));
		f[worst_p] = f[SOL - 1];
		cross_num++;
	}
	end = clock();
	cout << "\t Num of color =" << Col_num << "\t Cross Iter = " << cross_num << "\t time=" << (end - start) << endl;
	
	ofstream file;
	file.open(Output_path, ofstream::out);
    for (int i = 0; i < Ver_num; i++)
        file << i + 1 << " " << Sol_methods[POPULATION_NUM][i] << endl;
    file.close();

	for (int i = 0; i < Ver_num; i++)
		delete[]NB_ID[i];
	delete[]NB_ID;

	delete[]AdjVer_num;

	for (int i = 0; i < Ver_num; i++)
		delete[]AdjVer_Col_num[i];
	delete[]AdjVer_Col_num;

	for (int i = 0; i < POPULATION_NUM + 1; i++)
		delete[]Sol_methods[i];
	delete[]Sol_methods;

	for (int i = 0; i < Ver_num; i++)
		delete[]Tabu_tenure[i];
	delete[]Tabu_tenure;

	delete[]Color_num;
	
	return 0;
}