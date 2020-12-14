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

//------------------------����-----------------------
int Ver_num;//������
int Col_num;//Ҫ����ɫ��
int method_num = 0;//���ŵĽ��������
int BURDEN = INT16_MAX;
int LEA_BURDEN = INT16_MAX;  //��ͻ����Ŀ������ֹ����
int *Ver_Col;//������ɫ��
int **AdjVer_Col_num;//�ڽӽڵ���ɫ������
int **tabu_tenure;//�ڵ���ɳ���
int **NB_id;//�ڵ���ڽӽڵ�ID��
int *Adj_num;//�ڵ���ڽӽڵ����
bool **Adj_bool;//�߼��ڽӱ�
int **ChgCol_Methods;//���ܵı�ɫ·��,[i][0]����iԭɫ��[i][1]����i��ɫ

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void Initiate(string ex_path){
    ifstream file;
    file.open(ex_path);
    string content;
    //----------------------------------���ݶ�ȡ-------------------------
    while(file.eof()){//���ļ�δ����
        file >> content;
        if(content == "edge"){
            file >> Ver_num;
            //��ʼ���ڽӾ���
            Adj_bool = new bool *[Ver_num];
            for (int i = 0; i < Ver_num;i++){
                Adj_bool[i] = new bool[Ver_num];
                for (int j = 0; j < Ver_num;j++)
                    Adj_bool[i][j] = false;
            }
        }
        //�����ڽӾ���
        if(content == "e"){
            int v1, v2;
            file >> v1 >> v2;
            Adj_bool[v1 - 1][v2 - 1] = true;
            Adj_bool[v2 - 1][v1 - 1] = true;
        }
    }
    file.close();

    //--------------------��ʼ���ڵ���ڽӽڵ�ID�����ڵ���ڽӽڵ����-----------------
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

    //-------------------------------�����������ɫ---------------------------------
    srand(time(NULL));
    Ver_Col = new int[Ver_num];
    for (int i = 0; i < Ver_num;i++){
        int k = rand();
        Ver_Col[i] = k % Col_num;
    }
    
    //---------------------------��ʼ���ڽӽڵ���ɫ������--------------------------
    AdjVer_Col_num = new int *[Ver_num];
    BURDEN = 0;
    for (int i = 0; i < Ver_num;i++){
        AdjVer_Col_num[i] = new int[Col_num];
        for (int j = 0; j < Col_num; j++)
            AdjVer_Col_num[i][j] = 0;
        for (int j = 0; j < Ver_num; j++)
        { 
             //��������������ڣ�ȡj������ɫΪĿ��ɫ�ۼ�
            if (Adj_bool[i][j])
            {
                AdjVer_Col_num[i][Ver_Col[j]]++;
                AdjVer_Col_num[j][Ver_Col[i]]++;
                //����������ڶ�����ɫ��ͬ����ͻ����һ
                if(Ver_Col[i] == Ver_Col[j])
                    BURDEN++;
            }
        }
    }
    LEA_BURDEN = BURDEN;//���ٳ�ͻ

    //------------------------------��ʼ�����ɳ���-----------------------------
    tabu_tenure = new int *[Ver_num];
    for (int i = 0; i < Ver_num;i++){
        tabu_tenure[i] = new int[Ver_num];
        for (int j = 0; j < Ver_num;j++)
            tabu_tenure[i][j] = 0;
    }

    //-------------------------------��ʼ�����ܵı仯·��-----------------------
    ChgCol_Methods = new int *[Ver_num * (Col_num - 1)];
    for (int i = 0; i < Ver_num * (Col_num - 1);i++)
        ChgCol_Methods[i] = new int[2];
}

void Track_step(int& delt_, const int iter){
    int tem_delt;
    //���ڵ���ͬ��ɫ�Ľڵ�����С��
    int Cur_color;
    for (int i = 0; i < Ver_num;i++){
        int ConCol_num = 0;//���ڵ���ͬ��ɫ�Ľڵ���
        Cur_color = Ver_Col[i];
        ConCol_num = AdjVer_Col_num[i][Cur_color];
        if(ConCol_num>0){//�����ǰ�ڵ㵱ǰ��ɫ�޳�ͻ�����ػ�ɫ
            for(int j=0;j<Col_num;j++){//���Ը�����ɫ���л�
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
    Ver_Col[ChgCol_Methods[snum][0]] = new_color;//�����ɫ
    tabu_tenure[ChgCol_Methods[snum][0]][new_color] = iter + BURDEN;
    int nb_num = Adj_num[ChgCol_Methods[snum][0]];
    for (int i = 0; i < nb_num;i++){
        int tem_id = NB_id[ChgCol_Methods[snum][0]][i];
        AdjVer_Col_num[tem_id][old_color]--;
        AdjVer_Col_num[tem_id][new_color]++;
    }
        
    
}


/**
 * @description: ������
 * @param {
 * ������ȡ·��|������·��|��ɫ��
 * *}
 * @return {*}
 */
int main(int argc,char* argv[]){
    if(argc != 4){
        cout << "�����в�������Ҫ��" << endl;
        return -1;
    }
    string Input_path = argv[1];
    string Output_path = argv[2];
    string Col_num_str = argv[3];
    int Col_num = (Col_num_str[0] - '0') * 10 + (Col_num_str[1] - '0');
    //��ʼ��
    Initiate(Input_path);
    int iter = 1;
    int delt = INT16_MAX;//��ֵ����ʼ��Ϊ���
    clock_t start = clock();
    //��ͻδ��Ϊ0�����������
    while(BURDEN){
        Track_step(delt, iter);
        Make_step(delt, iter);
        iter++;
    }
    clock_t end = clock();
    cout << "���" << Col_num_str << "ɫͿɫ�ķ�ʱ�䣺" << end - start << endl;
    cout << "��������Ϊ" << iter << endl;
    ofstream file;
    
    //���
    file.open(Output_path);
    for (int i = 0; i < Ver_num;i++)
        file << i << " " << Ver_Col[i] << endl;
    file.close();

    //----------------���տռ�-------------------
    
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
