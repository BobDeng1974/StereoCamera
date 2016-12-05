//˫Ŀƥ��
//�汾:Version 3.1.1
//����˫Ŀ�궨�ļ���˫Ŀ����ͷ���ɵõĻ������ƥ�䲢�ܹ����ͼ����������
//������������ȡ��Ŀ
//��ע�����������ģ�Ϳ��ѵ�����

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <vector>
#include <stdio.h>
#include <iostream>

#include "LBF_api.h"

using namespace cv;
using namespace std;

static void saveXYZ(const char* filename, const Mat& mat)
{
	const double max_z =80;
	FILE* fp = fopen(filename, "wt");
	for (int y = 0; y < mat.rows; y++)
	{
		for (int x = 0; x < mat.cols; x++)
		{
			Vec3f point = mat.at<Vec3f>(y, x);
			if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
			fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
		}
	}
	fclose(fp);
}

void detectAndDraw(Mat& inputimg,Mat& dirimg,CascadeClassifier& cascade,
	CascadeClassifier& nestedCascade,
	double scale, bool tryflip)
{
	int i = 0;
	double t = 0;
	//�������ڴ����������������
	vector<Rect> faces, faces2;
	//����һЩ��ɫ��������ʾ��ͬ������
	const static Scalar colors[] = { CV_RGB(255,255,255) };
	//������С��ͼƬ���ӿ����ٶ�
	//int cvRound (double value) ��һ��double�͵��������������룬������һ����������
	Mat gray;
	Mat smallImg(cvRound(inputimg.rows / scale), cvRound(inputimg.cols / scale), CV_8UC1);
	//ת�ɻҶ�ͼ��Harr�������ڻҶ�ͼ
	cvtColor(inputimg, gray, CV_BGR2GRAY);
	//�ı�ͼ���С��ʹ��˫���Բ�ֵ
	resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);
	//�任���ͼ�����ֱ��ͼ��ֵ������
	equalizeHist(smallImg, smallImg);

	//����ʼ�ͽ�������˺�����ȡʱ�䣬������������㷨ִ��ʱ��
	t = (double)cvGetTickCount();

	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		| CV_HAAR_SCALE_IMAGE
		,
		Size(20, 20));
	//���ʹ�ܣ���תͼ��������
	if (tryflip)
	{
		flip(smallImg, smallImg, 1);
		cascade.detectMultiScale(smallImg, faces2, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(20, 20));
		for (vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++)
		{
			faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
		}
	}
	t = (double)cvGetTickCount() - t;
	//   qDebug( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
	cout << "detected number: " << faces.size() << " " << "detection time: " << t / ((double)cvGetTickFrequency()*1000.) << endl;
	for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++)
	{
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[0];
		int radius;

		double aspect_ratio = (double)r->width / r->height;
		if (0.75 < aspect_ratio && aspect_ratio < 1.3)
		{
			//��ʾ����ʱ����С֮ǰ��ͼ���ϱ�ʾ����������������ű��������ȥ
			rectangle(inputimg, cvPoint(cvRound(r->x*scale), cvRound(r->y*scale)), cvPoint(cvRound((r->x + r->width - 1)*scale), cvRound((r->y + r->height - 1)*scale)), color, 2, 8, 0);
		}
		else
		{
			center.x = cvRound((r->x + r->width*0.5)*scale);
			center.y = cvRound((r->y + r->height*0.5)*scale);
			radius = cvRound((r->width + r->height)*0.25*scale);
			circle(inputimg, center, radius, color, 3, 8, 0);
		}

		smallImgROI = smallImg(*r);
		//���ؼ���
		vector<Point2f> landmarks;
		detectKeyPoints(smallImgROI, landmarks);
		vector<Point2f>::iterator p_point = landmarks.begin();
		//cout << landmarks << endl;
		while (p_point != landmarks.end())
		{
			center.x = cvRound((r->x + p_point->x)*scale);
			center.y = cvRound((r->y + p_point->y)*scale);
			circle(dirimg, center, 3, color, 1, 8, 0);
			p_point++;
		}
	}
	cv::imshow("result", dirimg);
}

int main()
{
	//��ȡyml˫Ŀƥ���ļ�
	const char* intrinsic_filename = "data\\intrinsics.yml";
	const char* extrinsic_filename = "data\\extrinsics.yml";
	Rect roi1, roi2;
	Mat Q;
	float scale = 0.5;
	
	CascadeClassifier cascade, nestedCascade;
	Initial_Model("D:\\Work\\Projects\\multcamera\\FaceAlignment\\FaceAlignment\\model\\");
	cascade.load("D:\\Work\\Projects\\multcamera\\FaceAlignment\\FaceAlignment\\test\\opencv-detection\\haarcascade_frontalface_alt.xml");
	nestedCascade.load("haarcascade_eye_tree_eyeglasses.xml");
	
	// ��data�ļ��ж�ȡ�궨����
	FileStorage fs(intrinsic_filename, FileStorage::READ);
	Mat M1, D1, M2, D2;
	fs["cameraMatrixL"] >> M1;
	fs["cameraDistcoeffL"] >> D1;
	fs["cameraMatrixR"] >> M2;
	fs["cameraDistcoeffR"] >> D2;
	M1 *= scale;
	M2 *= scale;

	fs.open(extrinsic_filename, FileStorage::READ);
	Mat R, T, R1, P1, R2, P2;
	fs["R"] >> R;
	fs["T"] >> T;

	//���ƴ���
	Mat canvas;
	int w, h;
	w = 320;
	h = 240;
	canvas.create(h, w * 2, CV_8UC3);

	Mat disp, disp8, img1, img2;
	//����ƥ��ģʽ:STEREO_BM = 0, STEREO_SGBM = 1, STEREO_HH = 2, STEREO_VAR = 3
	int alg = 1;

	//ƥ�����
	int SADWindowSize = 1, numberOfDisparities =128;
	////////////////////5////////////////////////256/
	bool no_display = false;


	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16, 3);

	Mat left, right;
	//��������ͷ
	VideoCapture capleft(1);
	//��������ͷ
	VideoCapture capright(0);
	
	cout << "Press Q to quit the program" << endl;

	while (1)
	{
		capleft >> left;
		capright >> right;
		//����ͼ�񻭵�������
		//�õ�������һ����
		
		Mat canvasPart = canvas(Rect(0, 0, w, h));
		//��ͼ�����ŵ���canvasPartһ����С
		resize(left, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);

		//����ͼ�񻭵�������
		//��û�������һ����
		canvasPart = canvas(Rect(w, 0, w, h));
		resize(right, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);

		imshow("Capture", canvas);

		resize(left, img1, left.size() / 2, 0, 0, INTER_AREA);
		resize(right, img2, right.size() / 2, 0, 0, INTER_LINEAR);
		//img1 = left;
		//img2 = right;
		Size img_size = img1.size();

		//����������ͷ���ɵõĻ�����н���
		stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);

		Mat map11, map12, map21, map22;
		initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
		initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);

		Mat img1r, img2r;
		remap(img1, img1r, map11, map12, INTER_LINEAR);
		remap(img2, img2r, map21, map22, INTER_LINEAR);

		img1 = img1r;
		img2 = img2r;


		numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width / 8) + 15) & -16;

		sgbm->setPreFilterCap(63);
		int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
		sgbm->setBlockSize(sgbmWinSize);

		int cn = img1.channels();

		sgbm->setP1(8 * cn*sgbmWinSize*sgbmWinSize);
		sgbm->setP2(32 * cn*sgbmWinSize*sgbmWinSize);
		sgbm->setMinDisparity(0);
		sgbm->setNumDisparities(numberOfDisparities);
		sgbm->setUniquenessRatio(10);
		sgbm->setSpeckleWindowSize(100);
		sgbm->setSpeckleRange(32);
		sgbm->setDisp12MaxDiff(1);
		sgbm->setMode(StereoSGBM::MODE_SGBM);


		//��ʾ����ʱ��
		int64 t = getTickCount();
		sgbm->compute(img1, img2, disp);
		//medianBlur(disp, disp, 9);
		t = getTickCount() - t;
		cout << "Time elapsed: " << t * 1000 / getTickFrequency() << "ms" << endl;

		//disp = dispp.colRange(numberOfDisparities, img1p.cols);
		disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
		resize(disp8, disp8, canvasPart.size() * 2, 0, 0, INTER_LINEAR);
		imshow("Deepwindow", disp8);
		detectAndDraw(left,disp8, cascade, nestedCascade, 2, 0);
		if (cvWaitKey(10) == 'w')
		{

			printf("storing the point cloud...");
			Mat xyz;
			reprojectImageTo3D(disp, xyz, Q, true);
			saveXYZ("pointcloud.txt", xyz);
			printf("\n");
		};
		if (cvWaitKey(10) == 'q')
			break;
	}
	return 0;
}