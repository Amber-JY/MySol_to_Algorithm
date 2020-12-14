/*
 * @Author: ZJY
 * @Date: 2020-12-10 21:01:56
 * @LastEditTime: 2020-12-11 09:48:39
 * @LastEditors: Please set LastEditors
 * @Description: 48 colors coding, 49 colors OK
 * @FilePath: \undefinedc:\Users\50465\Desktop\SmartLAB\TS_Coloring.cpp
 */
#include<iostream>
#include<ctime>
#include<fstream>
#include<string>
#include<cstdlib>

using namespace std;

//------------------------变量-----------------------
int Ver_num;//顶点数
int Col_num;//要求颜色数
int method_num = 0;//最优的解决方法数
int BURDEN = INT16_MAX;
int LEA_BURDEN = INT16_MAX;  //冲突总数目，即终止条件
int *Ver_Col;//顶点颜色集
int **AdjVer_Col_num;//邻接节点颜色个数集
int **tabu_tenure;//节点禁忌长度
int **NB_id;//节点的邻接节点ID集
int *Adj_num;//节点的邻接节点个数
bool **Adj_bool;//逻辑邻接表
int **ChgCol_Methods;//可能的变色路径,[i][0]顶点i原色，[i][1]顶点i新色

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Initiate(string ex_path){
    ifstream file;
    file.open(ex_path);
    string content;
    //----------------------------------数据读取-------------------------
    while(file.eof()){//若文件未结束
        file >> content;
        if(content == "edge"){
            file >> Ver_num;
            //初始化邻接矩阵
            Adj_bool = new bool *[Ver_num];
            for (int i = 0; i < Ver_num;i++){
                Adj_bool[i] = new bool[Ver_num];
                for (int j = 0; j < Ver_num;j++)
                    Adj_bool[i][j] = false;
            }
        }
        //建立邻接矩阵
        if(content == "e"){
            int v1, v2;
            file >> v1 >> v2;
            Adj_bool[v1 - 1][v2 - 1] = true;
            Adj_bool[v2 - 1][v1 - 1] = true;
        }
    }
    file.close();

    //--------------------初始化节点的邻接节点ID集，节点的邻接节点个数-----------------
    NB_id = new int*[Ver_num];
    Adj_num = new int[Ver_num];
    for (int i = 0; i < Ver_num;i++){
        Adj_num[i] = 0;
        for (int j = 0; j < Ver_num;j++){
            if(Adj_bool[i][j]){
                NB_id[i][Adj_num[i]] = j;
                Adj_num[i]++;
            }
        }
    }

    //-------------------------------余数法随机着色---------------------------------
    srand(time(NULL));
    Ver_Col = new int[Ver_num];
    for (int i = 0; i < Ver_num;i++){
        int k = rand();
        Ver_Col[i] = k % Col_num;
    }
    
    //---------------------------初始化邻接节点颜色个数集--------------------------
    AdjVer_Col_num = new int *[Ver_num];
    BURDEN = 0;
    for (int i = 0; i < Ver_num;i++){
        AdjVer_Col_num[i] = new int[Col_num];
        for (int j = 0; j < Col_num; j++)
            AdjVer_Col_num[i][j] = 0;
        for (int j = 0; j < Ver_num; j++)
        { 
             //如果两个顶点相邻，取j顶点颜色为目标色累加
            if (Adj_bool[i][j])
            {
                AdjVer_Col_num[i][Ver_Col[j]]++;
                AdjVer_Col_num[j][Ver_Col[i]]++;
                //如果两个相邻顶点颜色相同，冲突数加一
                if(Ver_Col[i] == Ver_Col[j])
                    BURDEN++;
            }
        }
    }
    LEA_BURDEN = BURDEN;//最少冲突

    //------------------------------初始化禁忌长度-----------------------------
    tabu_tenure = new int *[Ver_num];
    for (int i = 0; i < Ver_num;i++){
        tabu_tenure[i] = new int[Ver_num];
        for (int j = 0; j < Ver_num;j++)
            tabu_tenure[i][j] = 0;
    }

    //-------------------------------初始化可能的变化路径-----------------------
    ChgCol_Methods = new int *[Ver_num * (Col_num - 1)];
    for (int i = 0; i < Ver_num * (Col_num - 1);i++)
        ChgCol_Methods[i] = new int[2];
}

void Track_step(int& delt_, const int iter){
    int tem_delt;
    //相邻的相同颜色的节点数最小者
    int Cur_color;
    for (int i = 0; i < Ver_num;i++){
        int ConCol_num = 0;//相邻的相同颜色的节点数
        Cur_color = Ver_Col[i];
        ConCol_num = AdjVer_Col_num[i][Cur_color];
        if(ConCol_num>0){//如果当前节点当前颜色无冲突，不必换色
            for(int j=0;j<Col_num;j++){//尝试各种颜色的切换
                if(Cur_color!=j){
                    tem_delt = AdjVer_Col_num[i][j] - AdjVer_Col_num[i][Cur_color];
                    if(tem_delt<=delt_ && (iter > tabu_tenure[i][j] || BURDEN + tem_delt < LEA_BURDEN)){
                        if(tem_delt<delt_){
                            method_num = 0;
                            delt_ = tem_delt;
                        }
                        ChgCol_Methods[method_num][0] = i;
                        ChgCol_Methods[method_num][1] = j;
                        method_num++;
                    }
                }
            }
        }
    }
}

void Make_step(const int delt_, const int iter){
    BURDEN = BURDEN + delt_;
    if(BURDEN<LEA_BURDEN)
        LEA_BURDEN = BURDEN;
    int snum = rand() % method_num;
    int old_color = Ver_Col[ChgCol_Methods[snum][0]];
    int new_color = ChgCol_Methods[snum][1];
    Ver_Col[ChgCol_Methods[snum][0]] = new_color;//变更颜色
    tabu_tenure[ChgCol_Methods[snum][0]][new_color] = iter + BURDEN;
    int nb_num = Adj_num[ChgCol_Methods[snum][0]];
    for (int i = 0; i < nb_num;i++){
        int tem_id = NB_id[ChgCol_Methods[snum][0]][i];
        AdjVer_Col_num[tem_id][old_color]--;
        AdjVer_Col_num[tem_id][new_color]++;
    }
        
    
}


/**
 * @description: 主函数
 * @param {
 * 算例读取路径|结果输出路径|颜色数
 * *}
 * @return {*}
 */
int main(int argc,char* argv[]){
    if(argc != 4){
        cout << "命令行参数不合要求" << endl;
        return -1;
    }
    string Input_path = argv[1];
    string Output_path = argv[2];
    string Col_num_str = argv[3];
    int Col_num = (Col_num_str[0] - '0') * 10 + (Col_num_str[1] - '0');
    //初始化
    Initiate(Input_path);
    int iter = 1;
    int delt = INT16_MAX;//差值，初始化为最大
    clock_t start = clock();
    //冲突未降为0，则继续迭代
    while(BURDEN){
        Track_step(delt, iter);
        Make_step(delt, iter);
        iter++;
    }
    clock_t end = clock();
    cout << "求解" << Col_num_str << "色涂色耗费时间：" << end - start << endl;
    cout << "迭代次数为" << iter << endl;
    ofstream file;
    
    //输出
    file.open(Output_path);
    for (int i = 0; i < Ver_num;i++)
        file << i << " " << Ver_Col[i] << endl;
    file.close();

    //----------------回收空间-------------------
    
    delete[] Ver_Col;
    delete[] Adj_num;
    for (int i = 0; i < Ver_num;i++){
        delete[] Adj_bool[i];
        delete[] NB_id[i];
        delete[] tabu_tenure[i];
        delete[] Adj_bool[i];
        delete[] AdjVer_Col_num[i];
    }
    delete[] Adj_bool;
    delete[] NB_id;
    delete[] tabu_tenure;
    delete[] Adj_bool;
    delete[] AdjVer_Col_num;
    for (int i = 0; i < Ver_num * (Col_num - 1);i++)
        delete[] ChgCol_Methods[i];
    delete[] ChgCol_Methods;
    return 0;
}
