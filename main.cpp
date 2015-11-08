#include <opencv2/opencv.hpp>
#include <opencv2/legacy/compat.hpp>

using namespace std;

IplImage* equalize_hist(IplImage **inputImage) {

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~inputImage_histogram~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	int size_ = 256;
	float range[2] = {0, 255};
	float* ranges[1] = {range};
	//创建一个维数为1，维数尺寸为256的直方图
	CvHistogram *hist = cvCreateHist(1, &size_, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist(inputImage, hist);

	int width_ = 255;
	int height_ = 300;
	int scale_ = 2;
	IplImage *input_image_hist = cvCreateImage(cvSize(width_*scale_, height_), 8, 1);
	//黑底
	cvRectangle(input_image_hist, cvPoint(0, 0), cvPoint(width_*scale_, height_), CV_RGB(0, 0, 0), CV_FILLED);

	//得到直方图的最大值
	float max = 0;
	cvGetMinMaxHistValue(hist, NULL, &max, NULL, NULL);

	vector<float> p;
	float sum = 0;
	for (int i = 0; i <= 255; i ++) {
		//返回相应bin中的值的浮点数
		float hist_value = cvQueryHistValue_1D(hist, i);
		sum += hist_value;
		p.push_back(hist_value);
		//按比率显示高度
		int realHeight_ = cvRound((hist_value / max) * height_);
		CvPoint p1 = cvPoint(i * scale_, height_ - 1);
		CvPoint p2 = cvPoint((i + 1) * scale_ - 1, height_ - realHeight_);
		cvRectangle(input_image_hist, p1, p2, cvScalar(255, 255, 255, 0), CV_FILLED); 
	}
	cvShowImage("input_image_hist", input_image_hist);

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~create my outputImage~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	//累积概率分布
	float count = 0;
	for (int i = 0; i <= 255; i ++) {
		count += p[i] / sum;
		p[i] = count;
	}
	//创建一个图
	int result_width = (*inputImage)->width;
	int result_height = (*inputImage)->height;
	IplImage *outputImage= cvCreateImage(cvSize(result_width, result_height), IPL_DEPTH_8U, 1);
	
	//均衡化
	for (int i = 0; i < result_height; i ++)
		for (int j = 0; j < result_width; j ++) {
			double v = cvGetReal2D( (*inputImage), i, j );
			double temp = p[(int)v]*255;
			cvSetReal2D( outputImage, i, j, temp);
		}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~outputImage_histogram~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	CvHistogram *hist2 = cvCreateHist(1, &size_, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist(&outputImage, hist2);

	IplImage *output_image_hist = cvCreateImage(cvSize(width_*scale_, height_), 8, 1);
	cvRectangle(output_image_hist, cvPoint(0, 0), cvPoint(width_*scale_, height_), CV_RGB(0, 0, 0), CV_FILLED);

	float max2 = 0;
	cvGetMinMaxHistValue(hist2, NULL, &max2, NULL, NULL);
	for (int i = 0; i <= 255; i ++) {
		float hist_value = cvQueryHistValue_1D(hist2, i);
		int realHeight_ = cvRound((hist_value / max2) * height_);
		CvPoint p1 = cvPoint(i * scale_, height_ - 1);
		CvPoint p2 = cvPoint((i + 1) * scale_ - 1, height_ - realHeight_);
		cvRectangle(output_image_hist, p1, p2, cvScalar(255, 255, 255, 0), CV_FILLED); 
	}
	cvShowImage("output_image_hist", output_image_hist);

	return outputImage;
}

IplImage* filter2d(IplImage *inputImage, vector<float> filter) {
	int filter_size = sqrt(filter.size());

	int inputImage_width = inputImage -> width;
	int inputImage_height = inputImage -> height;

	//补m-1圈0
	int m = filter_size - 1;
	IplImage* copy_of_inputImage = cvCreateImage(cvSize(inputImage_width + m * 2, inputImage_height + m * 2), IPL_DEPTH_8U, 1);
	int zero_width = copy_of_inputImage -> width;
	int zero_height = copy_of_inputImage -> height;

	for (int i = 0; i < zero_height; i ++) {
		for (int j = 0; j < zero_width; j ++) {
			if ((i < (zero_height - m) && i > (m - 1)) && (j < (zero_width - m) && j > (m - 1))) {
					double v = cvGetReal2D( inputImage, i-m, j-m );
					cvSetReal2D( copy_of_inputImage, i, j, v);
			}
			else  {
				cvSetReal2D( copy_of_inputImage, i, j, 0);
			}
		}
	}

	int start = filter_size / 2;
	IplImage* outputImage = cvCreateImage(cvSize(inputImage_width, inputImage_height), IPL_DEPTH_8U, 1);
	for (int i = m; i < zero_height - m; i ++) {
		for (int j = m; j < zero_width - m; j ++) {
			double temp = 0;
			int count = 0;
			for (int k1 = 0 - start; k1 <= start; k1 ++) {
				for (int k2 = 0 - start; k2 <= start; k2 ++) {
					double v = cvGetReal2D( copy_of_inputImage, i + k1, j + k2);
					temp += filter[count ++] * v;
				}
			}
			cvSetReal2D( outputImage, i - m, j - m, temp);
		}
	}
	return outputImage;
}


int main( int argc, char** argv ) {	

	/*~~~~~~~~~~~~~~~~~~~~~First Part-Equalization~~~~~~~~~~~~~~~~~~~~~~~~~~*/
         	IplImage *input_equalize = cvLoadImage("07.png", -1);
         	IplImage *output_equalize = equalize_hist(&input_equalize);
         	cvShowImage("input_equalize",input_equalize);
         	cvShowImage("output_equalize",output_equalize);

         	/*~~~~~~~~~~~~~~~~~~~~~~~~Second Part-Filter~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
         	IplImage *input_filter = cvLoadImage("07.png", -1);

         	//filter1 3*3
         	vector<float> filter1;
         	for (int i = 0; i < 9; i ++)filter1.push_back((1+0.0)/9);
         	IplImage *output_filter1 = filter2d(input_filter, filter1);
         	//cvShowImage("output_filter1 3*3",output_filter1);

         	//filter2 7*7
         	vector<float> filter2;
         	for (int i = 0; i < 49; i ++)filter2.push_back((1+0.0)/49);
         	IplImage *output_filter2 = filter2d(input_filter, filter2);
         	//cvShowImage("output_filter2 7*7",output_filter2);

         	//filter3 11*11
         	vector<float> filter3;
         	for (int i = 0; i < 121; i ++)filter3.push_back((1+0.0)/121);
         	IplImage *output_filter3 = filter2d(input_filter, filter3);
         	//cvShowImage("output_filter3 11*11",output_filter3);

         	//filter4 Laplacian filter 3*3
         	vector<float> filter4;
         	float n;
         	for (int i = 0; i < 9; i ++) {
         		if (i == 4)n = 8;
         		else n = -1;
         		filter4.push_back(n);
         	}
         	IplImage *output_filter4 = filter2d(input_filter, filter4);
         	//cvShowImage("output_filter4 Laplacian filter 3*3",output_filter4);

         	//filter5 high-boost filtering
         	IplImage *src_image = cvLoadImage("07.png", -1);
         	int width = src_image -> width;
         	int height = src_image -> height;
         	//g mask (x, y)
         	for (int i = 0; i < height; i ++) {
         		for (int j = 0; j < width; j ++) {
         			double v1 = cvGetReal2D( output_filter1, i, j);
         			double v2 = cvGetReal2D( src_image, i, j);
			cvSetReal2D( src_image, i, j, v2 - v1);
         		}
         	}
         	cvShowImage("high-boost filtering v2 - v1", src_image);
         	//g(x, y) = f (x, y) + k ∗ g mask (x, y)
         	for (int i = 0; i < height; i ++) {
         		for (int j = 0; j < width; j ++) {
         			double v1 = cvGetReal2D( input_filter, i, j);
         			double v2 = 2 * cvGetReal2D( src_image, i, j);
			cvSetReal2D( src_image, i, j, v2 +v1);
         		}
         	}
         	cvShowImage("high-boost filtering v2 + v1", src_image);

         	cvWaitKey(0);
	return 0;
}


