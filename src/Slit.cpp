#include "Slit.h"
#include <iostream>

Slit::Slit()
= default;

Slit::Slit(double w, double h, int slit_sampling){
    this->set_slit(w,h,slit_sampling);
}

void Slit::set_slit(double w, double h, int slit_sampling){
    this->w = w;
    this->h = h;
    this->ratio = h/w;
    this->slit_sampling = slit_sampling;
    this->w_px = slit_sampling;
    this->h_px = static_cast<int>(slit_sampling * this->ratio);

    #ifdef USE_GPU
    {
        cv::Mat ones =  cv::Mat::ones(round(this->h_px), this->w_px, CV_32FC1);
        this->slit_image = cv::gpu::GpuMat();
        this->slit_image.upload(ones);
    }
    #else
    {
        this->slit_image = Matrix(static_cast<size_t>(this->h_px), static_cast<size_t>(this->w_px));

//        cv::Point rook_points[1][3];
//        rook_points[0][0] = cv::Point( 0., 0. );
//        rook_points[0][1] = cv::Point( 0., this->h_px );
//        rook_points[0][2] = cv::Point( this->w_px, this->h_px );
//
//        const cv::Point* ppt[1] = { rook_points[0] };
//        int npt[] = { 3 };
//
//        cv::fillPoly(this->slit_image,ppt, npt,1.,1.,8);
//        cv::circle(this->slit_image, cv::Point2d(50.,50.), 50., 1., -1);
    }
    #endif
}

void Slit::show(){
//    cv::imshow("Slit Image", this->slit_image);
//    cv::waitKey(0);

}
