//
//  Facedetect.cpp
//  myopencv
//
//  Created by lequan on 1/24/15.
//  Copyright (c) 2015 lequan. All rights reserved.
//
#include "LBFRegressor.h"
#include "LBF_api.h"

using namespace std;
using namespace cv;
int save_count=0;

Params global_params;
LBFRegressor *regressor;


void Initial_Model(string mp){
	modelPath = mp;
	ReadGlobalParamFromFile(modelPath + "LBF.model");
	regressor = new LBFRegressor;
	regressor->Load(modelPath + "LBF.model");
}


//�����۾������ͼ����з���任
//src - ԭͼ��
//landmarks - ԭͼ����68���ؼ���
void getwarpAffineImg(Mat &src, vector<Point2f> &landmarks, string img_write_path)
{
	Point2f eye_left, eye_right;
	//37����40��Ϊ���۽ǵ�
	eye_left.x = (landmarks[36].x + landmarks[39].x)*0.5;
	eye_left.y = (landmarks[36].y + landmarks[39].y)*0.5;
	//43����46��Ϊ���۽ǵ�
	eye_right.x = (landmarks[42].x + landmarks[45].x)*0.5;
	eye_right.y = (landmarks[42].y + landmarks[45].y)*0.5;
	//�����������ĵ㣬���մ����ĵ������ת
	Point2f eyesCenter = Point2f((eye_left.x + eye_right.x) * 0.5f, (eye_left.y + eye_right.y) * 0.5f);

	// ���������۾���ĽǶ�
	double dy = (eye_right.y - eye_left.y);
	double dx = (eye_right.x - eye_left.x);
	double angle = atan2(dy, dx) * 180.0 / CV_PI; // Convert from radians to degrees.

	//��eyesCenter, andle, scale���չ�ʽ�������任���󣬴�ʱ1.0��ʾ����������
	Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, 1.0);
	Mat rot;
	// ���з���任���任���СΪsrc�Ĵ�С
	warpAffine(src, rot, rot_mat, src.size());
	//����任��image
	imwrite(img_write_path, rot);
}

void detectAndDraw1(Mat& img, LBFRegressor* regressor, string img_write_path, vector<Point2f>& landmarks){
	int i = 0;
	double t = 0;
	Mat gray;
	if (img.channels() == 3)cvtColor(img, gray, CV_BGR2GRAY);
	else img.copyTo(gray);
	equalizeHist(gray, gray);

	// --Alignment
		BoundingBox boundingbox;

		boundingbox.start_x = 0;
		boundingbox.start_y = 0;
		boundingbox.width = (gray.cols - 1);
		boundingbox.height = (gray.rows - 1);
		boundingbox.centroid_x = boundingbox.start_x + boundingbox.width / 2.0;
		boundingbox.centroid_y = boundingbox.start_y + boundingbox.height / 2.0;

		Mat_<double> current_shape = regressor->Predict(gray, boundingbox, 1);
		Point2f point;
		for (int j = 0; j < global_params.landmark_num; j++)
		{
			point.x = current_shape(j,0);
			point.y = current_shape(j, 1);
			landmarks.push_back(point);
		}
/*
		int k = 1;
		vector<Point2f>::iterator s = landmarks.begin();
		while(s != landmarks.end())
		{
			cout<<k<<" "<<"x: "<<s->x<<" "<<"y: "<<s->y<<endl;
			s++;
			k++;
		}
		*/
		getwarpAffineImg(img, landmarks, img_write_path);

}

void patch_FaceAlignment(string imgfilelist,string imgwritelist){
	fstream img_read_file;
	string img_read_path, img_write_path;
	img_read_file.open(imgfilelist.c_str(), ios::in);
	//write alignment face path to txt
	fstream image_write_file;
	image_write_file.open(imgwritelist.c_str(), ios::in);
	//param
	Mat img;
	vector<Point2f> landmarks;
	while (img_read_file >> img_read_path&&image_write_file>>img_write_path){
		img = imread(img_read_path);
		cout << img_write_path << endl;
		detectAndDraw1(img, regressor, img_write_path,landmarks);
	}
	img_read_file.close();
	image_write_file.close();
}

void single_FaceAlignment(string imgpath, string img_write_path, vector<Point2f>& landmarks){
	Mat img = imread(imgpath);
	detectAndDraw1(img, regressor, img_write_path,landmarks);
}

void detectKeyPoints(Mat &img, vector<Point2f> &landmarks)
{
	int i = 0;
	double t = 0;
	Mat gray;
	if (img.channels() == 3)cvtColor(img, gray, CV_BGR2GRAY);
	else img.copyTo(gray);
	equalizeHist(gray, gray);

	// --Alignment
		BoundingBox boundingbox;

		boundingbox.start_x = 0;
		boundingbox.start_y = 0;
		boundingbox.width = (gray.cols - 1);
		boundingbox.height = (gray.rows - 1);
		boundingbox.centroid_x = boundingbox.start_x + boundingbox.width / 2.0;
		boundingbox.centroid_y = boundingbox.start_y + boundingbox.height / 2.0;

		Mat_<double> current_shape = regressor->Predict(gray, boundingbox, 1);
		Point2f point;
		for (int j = 0; j < global_params.landmark_num; j++)
		{
			point.x = current_shape(j,0);
			point.y = current_shape(j, 1);
			landmarks.push_back(point);
		}
}
