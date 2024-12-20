#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define SEAM_ROWS 0
#define SEAM_COLS 1

Mat filtreGaussien(const Mat &image);
Mat filtreGradient(const Mat &image);
int **matriceCumulativeCols(const Mat &image);
int **matriceCumulativeRows(const Mat &image);
int *findWayCols(const Mat &image, int **m_cumul);
int *findWayRows(const Mat &image, int **m_cumul);
inline void removePixelAndShiftLeftGray(Mat &image, int row, int col);
inline void removePixelAndShiftUpGray(Mat &image, int row, int col);
inline Mat suppressionSeamGray(const Mat &image, const int *way, int seam_type);
inline void removePixelAndShiftLeftColor(Mat &image, int row, int col);
inline void removePixelAndShiftUpColor(Mat &image, int row, int col);
inline Mat suppressionSeamColor(const Mat &image, const int *way,
                                int seam_type);
inline Mat imageSeamed(const Mat &image, const int *way, int seam_type);
Mat seamCarving(Mat image, Mat image_gray, int NB_TOUR, const string &nomImage,
                const string &repertoire, int seam_type);

#endif