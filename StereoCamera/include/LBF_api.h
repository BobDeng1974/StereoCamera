#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
// api

//ģ�Ͳ�����ʼ��
void Initial_Model(string modelPath);
//������������
void single_FaceAlignment(string imgreadpath, string imgwritepath, vector<Point2f> &landmarks);
//������������
void patch_FaceAlignment(string imgfilelist, string imgwritelist);

void detectKeyPoints(Mat &img, vector<Point2f> &landmarks);
