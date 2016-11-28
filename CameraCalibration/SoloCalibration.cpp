//��Ŀ����ͷ�궨
//�汾��Version 2.1.0
//�ȶ�����ͷ���е����궨������yml�ļ���˫Ŀ�궨ʹ��
#include <opencv2/opencv.hpp>  
#include <highgui.hpp>  
#include "cv.h"  
#include <cv.hpp>  
#include <iostream> 
#include "SoloCalibration.h"
using namespace std;
using namespace cv;

const int imageWidth = 640;                             //����ͷ�ķֱ���  
const int imageHeight = 480;
const int boardWidth = 7;                               //����Ľǵ���Ŀ  
const int boardHeight = 5;                              //����Ľǵ�����  
const int boardCorner = boardWidth * boardHeight;       //�ܵĽǵ�����  
const int frameNumber = 5;                             //����궨ʱ��Ҫ���õ�ͼ��֡��  
const int squareSize = 35;                              //�궨��ڰ׸��ӵĴ�С ��λmm  
const Size boardSize = Size(boardWidth, boardHeight);   
Mat intrinsic;                                          //����ڲ���  
Mat distortion_coeff;                                   //����������  
vector<Mat> rvecs;                                        //��ת����  
vector<Mat> tvecs;                                        //ƽ������  
vector<vector<Point2f>> corners;                        //����ͼ���ҵ��Ľǵ�ļ��� ��objRealPoint һһ��Ӧ  
vector<vector<Point3f>> objsoloRealPoint;                   //����ͼ��Ľǵ��ʵ���������꼯��  


vector<Point2f> corner;                                   //ĳһ��ͼ���ҵ��Ľǵ�  



														  //����궨����ģ���ʵ����������
void calRealsoloPoint(vector<vector<Point3f>>& obj, int boardwidth, int boardheight, int imgNumber, int squaresize)
{
	//  Mat imgpoint(boardheight, boardwidth, CV_32FC3,Scalar(0,0,0));  
	vector<Point3f> imgpoint;
	for (int rowIndex = 0; rowIndex < boardheight; rowIndex++)
	{
		for (int colIndex = 0; colIndex < boardwidth; colIndex++)
		{
			//  imgpoint.at<Vec3f>(rowIndex, colIndex) = Vec3f(rowIndex * squaresize, colIndex*squaresize, 0);  
			imgpoint.push_back(Point3f(rowIndex * squaresize, colIndex * squaresize, 0));
		}
	}
	for (int imgIndex = 0; imgIndex < imgNumber; imgIndex++)
	{
		obj.push_back(imgpoint);
	}
}

//��������ĳ�ʼ���� Ҳ���Բ�����
void guessCameraParam(void)
{
	/*�����ڴ�*/
	intrinsic.create(3, 3, CV_64FC1);
	distortion_coeff.create(5, 1, CV_64FC1);

	/*
	fx 0 cx
	0 fy cy
	0 0  1
	*/
	intrinsic.at<double>(0, 0) = 256.8093262;   //fx         
	intrinsic.at<double>(0, 2) = 160.2826538;   //cx  
	intrinsic.at<double>(1, 1) = 254.7511139;   //fy  
	intrinsic.at<double>(1, 2) = 127.6264572;   //cy  

	intrinsic.at<double>(0, 1) = 0;
	intrinsic.at<double>(1, 0) = 0;
	intrinsic.at<double>(2, 0) = 0;
	intrinsic.at<double>(2, 1) = 0;
	intrinsic.at<double>(2, 2) = 1;

	/*
	k1 k2 p1 p2 p3
	*/
	distortion_coeff.at<double>(0, 0) = -0.193740;  //k1  
	distortion_coeff.at<double>(1, 0) = -0.378588;  //k2  
	distortion_coeff.at<double>(2, 0) = 0.028980;   //p1  
	distortion_coeff.at<double>(3, 0) = 0.008136;   //p2  
	distortion_coeff.at<double>(4, 0) = 0;          //p3  
}

void outputSoloParam(int num)
{
	/*��������*/
	//cvSave("cameraMatrix.xml", &intrinsic);  
	//cvSave("cameraDistoration.xml", &distortion_coeff);  
	//cvSave("rotatoVector.xml", &rvecs);  
	//cvSave("translationVector.xml", &tvecs);  
	/*�������*/
	cout << "fx :" << intrinsic.at<double>(0, 0) << endl << "fy :" << intrinsic.at<double>(1, 1) << endl;
	cout << "cx :" << intrinsic.at<double>(0, 2) << endl << "cy :" << intrinsic.at<double>(1, 2) << endl;
	cout << "k1 :" << distortion_coeff.at<double>(0, 0) << endl;
	cout << "k2 :" << distortion_coeff.at<double>(1, 0) << endl;
	cout << "p1 :" << distortion_coeff.at<double>(2, 0) << endl;
	cout << "p2 :" << distortion_coeff.at<double>(3, 0) << endl;
	cout << "p3 :" << distortion_coeff.at<double>(4, 0) << endl;
	
	if (num == 0)
	{
		FileStorage fs("../StereoCamera/data/CameraIntrinsicsR.yml", FileStorage::WRITE);
		if (fs.isOpened())
		{   
			fs << "cameraMatrixR" << intrinsic << "distCoeffR" << distortion_coeff;
			fs.release();
		}
		else
			cout << "Error: can not save the extrinsic parameters\n";
	}
	if (num == 1)
	{
		FileStorage fs("../StereoCamera/data/CameraIntrinsicsL.yml", FileStorage::WRITE);
		if (fs.isOpened())
		{
			fs << "cameraMatrixL" << intrinsic << "distCoeffL" << distortion_coeff;
			fs.release();
		}
		else
			cout << "Error: can not save the extrinsic parameters\n";
	}
	
}


int SoloCalibration()
{
	int Capturenum ;
	for (Capturenum = 0;Capturenum < 2;Capturenum++)
	{
		//������ͷ
		VideoCapture capture(Capturenum);
		// �������ͷ���Ƿ�ɹ�
		if (!capture.isOpened())
			return -1;
		cout << "Press W to analysis correct frame." << endl << "Press Q to quit the program." << endl;

		int goodFrameCount = 0;
		Mat img, grayImage;

		//ȡ��9���ʺϼ�������ͼƬ
		while (goodFrameCount < frameNumber)
		{
			//������ͷȡ��һ֡
			capture >> img;

			//��ʾһ֡����
			imshow("Capture", img);

			//��ȡ����W
			if (cvWaitKey(10) == 'w')
			{
				Mat rgbImage = img;
				//����Ƿ��ܹ��ҵ��ŵ�
				bool isFind = findChessboardCorners(rgbImage, boardSize, corner, 0);
				//���нǵ㶼���ҵ� ˵�����ͼ�����ʺϼ���
				if (isFind == true)
				{
					/*
					Size(5,5) �������ڵ�һ���С
					Size(-1,-1) ������һ��ߴ�
					TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1)������ֹ����
					*/
					cvtColor(img, grayImage, CV_BGR2GRAY);
					cornerSubPix(grayImage, corner, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
					//��ͼ���ϻ����ǵ�
					drawChessboardCorners(rgbImage, boardSize, corner, isFind);
					//��ʾ������ͼ��
					imshow("chessboard", rgbImage);
					//���ǵ���Ϣ����corner
					corners.push_back(corner);

					goodFrameCount++;
					cout << "The image No." << goodFrameCount << " is good!" << endl;
				}
				//���ͼ�񲻷��ϼ���׼
				else
				{
					cout << "The image No." << goodFrameCount + 1 << " is bad please try again!" << endl;
				}
			}
			//��ȡ����Q
			if (cvWaitKey(10) == 'q')
				return 0;
		}

		//ͼ��ɼ���� ��������ʼ����ͷ��У��
		//calibrateCamera()
		//������� objectPoints  �ǵ��ʵ����������
		//imagePoints   �ǵ��ͼ������
		//imageSize     ͼ��Ĵ�С
		//�������
		//cameraMatrix  ������ڲξ���
		//distCoeffs    ����Ļ������
		//rvecs         ��תʸ��(�����)
		//tvecs         ƽ��ʸ��(�������


		/*����ʵ�ʳ�ʼ���� ����calibrateCamera�� ���flag = 0 Ҳ���Բ���������*/
		guessCameraParam();
		cout << "guess successful" << endl;
		/*����ʵ�ʵ�У�������ά����*/
		calRealsoloPoint(objsoloRealPoint, boardWidth, boardHeight, frameNumber, squareSize);
		cout << "cal real successful" << endl;
		/*�궨����ͷ*/
		calibrateCamera(objsoloRealPoint, corners, Size(imageWidth, imageHeight), intrinsic, distortion_coeff, rvecs, tvecs, 0);
		cout << "calibration successful" << endl;
		/*���沢�������*/
		outputSoloParam(Capturenum);
		cout << "out successful" << endl;

		/*��ʾ����У��Ч��*/
		Mat cImage;
		undistort(img, cImage, intrinsic, distortion_coeff);
		imshow("Corret Image", cImage);
		cout << "Wait for Key" << endl;
		waitKey(0);
		destroyWindow("Corret Image");
	}
	destroyWindow("chessboard");
	destroyWindow("Capture");
	return 0;
}