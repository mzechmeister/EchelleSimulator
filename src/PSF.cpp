//
// Created by julian on 13.09.16.
//

#include <vector>
#include <iostream>
#include <highgui.h>
#include <opencv2/imgproc.hpp>
#include "PSF.h"
#include "H5Cpp.h"
#include "helper.h"


herr_t
file_info(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata)
{
    hid_t group;
    auto group_names=reinterpret_cast< std::vector<std::string>* >(opdata);
    group = H5Gopen2(loc_id, name, H5P_DEFAULT);

    group_names->push_back(name);
    // std::cout << "Name : " << name << std::endl;
    H5Gclose(group);
    return 0;
}

herr_t
dataset_info(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata)
{
    hid_t ds;
    auto dataset_names=reinterpret_cast< std::vector<std::string>* >(opdata);
    ds = H5Dopen2(loc_id, name, H5P_DEFAULT);

    dataset_names->push_back(name);
    // std::cout << "Name : " << name << std::endl;
    H5Dclose(ds);
    return 0;
}

herr_t read_psfs(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata)
{
    hid_t ds;
    ds = H5Dopen2(loc_id, name, H5P_DEFAULT);
    hid_t dspace = H5Dget_space(ds);

    const int ndims = H5Sget_simple_extent_ndims(dspace);
    hsize_t dims[ndims];
    H5Sget_simple_extent_dims(dspace, dims, NULL);

    double * buffer = new double[dims[0] * dims[1]];

    H5::DataSpace mspace1 = H5::DataSpace(2, dims);
    H5::DataSet dataset = H5::DataSet(ds);
    dataset.read( buffer, H5::PredType::NATIVE_INT, mspace1, dspace );

    auto psfs=reinterpret_cast< std::vector<cv::Mat>* >(opdata);
    cv::Mat psf(dims[0], dims[1], CV_32FC1);
    for(int i=0; i<dims[0]; ++i){
        for(int j=0; j<dims[1]; ++j){
            psf.at<double>(i,j) = buffer[i*dims[1]+j];
        }
    }


    psfs->push_back(psf);

    // std::cout << "Name : " << name << std::endl;
    H5Dclose(ds);
    return 0;
}

PSF::PSF(const std::string filename) {

    H5::H5File * file = new H5::H5File(filename, H5F_ACC_RDONLY);
    std::cout << std::endl << "Iterating over elements in the file" << std::endl;
    H5::Group * rootGr = new H5::Group(file->openGroup("/"));
    std::vector<std::string> group_names;
    // H5::Group *rootGr = new H5::Group (file->openGroup("/"));
    herr_t idx = H5Literate(rootGr->getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, file_info, &group_names);

    for(auto& gn : group_names)
    {
        int order = std::stoi(gn);
        std::vector<PSFdata> psf_vec;

        std::vector<std::string> wl_names;
        std::string dt_path = "/" + gn;
        H5::Group * wlGr = new H5::Group(file->openGroup(dt_path));


        herr_t idx2 = H5Literate(wlGr->getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, dataset_info, &wl_names);
        for(auto& w : wl_names){
            double wavelength = std::stod(w.c_str());
            std::vector< cv::Mat > raw_psfs;

            hid_t ds;

            H5::DataSet dataset = file->openDataSet(dt_path+"/"+w);
            H5::DataSpace dspace = dataset.getSpace();


            int ndims = dspace.getSimpleExtentNdims();
            hsize_t dims[ndims];
            ndims = dspace.getSimpleExtentDims( dims );


            double * buffer = new double[dims[0] * dims[1]];

            H5::DataSpace mspace1 = H5::DataSpace(2, dims);
            dataset.read( buffer, H5::PredType::NATIVE_DOUBLE, mspace1, dspace );


            cv::Mat psf((int) dims[0], (int) dims[1], CV_64FC1);
            for(int i=0; i<dims[0]; ++i){
                for(int j=0; j<dims[1]; ++j){
                    psf.at<double>(i,j) = buffer[i*dims[1]+j];
                }
            }
            // cv::cvtColor(psf,psf,CV_32FC1,0);
            PSFdata p(wavelength, psf);

            if ( this->psfs.find(order) == this->psfs.end() ) {
                // not found
                psf_vec.push_back(p);
                this->psfs.insert(std::make_pair(order, psf_vec));
            } else {
                this->psfs[order].push_back(p);
                // found
            }

//
//            this.psfs.
//            std::map<double, cv::Mat> inner;

//            double maxVal, minVal;
//            cv::minMaxLoc(psf, &minVal, &maxVal); //find minimum and maximum intensities
//            psf.convertTo(psf, CV_8UC1, 255/(maxVal - minVal), -minVal * 255/(maxVal - minVal));
//            cv::namedWindow("testwindow", CV_WINDOW_NORMAL);
//            cv::imshow("testwindow", psf);
//            cv::waitKey(0);
//
//            inner.insert(std::make_pair(wavelength, psf.clone()));
//            this->psfs.insert(std::make_pair(order, inner));
            psf.release();

        }

        std::sort(this->psfs[order].begin(), this->psfs[order].end());
    }
}

cv::Mat PSF::get_PSF(int order, double wavelength) {
    std::vector<double> distance;
    for(auto& psf : this->psfs[order])
    {
        distance.push_back(psf.wavelength -wavelength );
    }
    std::vector<size_t > idx;
    idx = compute_sort_order(distance);
    return this->interpolate_PSF(this->psfs[order][idx[0]].psf, this->psfs[order][idx[1]].psf,
                                 this->psfs[order][idx[0]].wavelength, this->psfs[order][idx[1]].wavelength, wavelength);
}

cv::Mat PSF::interpolate_PSF(cv::Mat psf1, cv::Mat psf2, double w1, double w2, double w) {
    double p1 = fabs((w-w1)/(w2-w1));
    double p2 = fabs((w-w2)/(w2-w1));
    double p_sum = p1+p2;
    p1 /= p_sum;
    p2 /= p_sum;
    cv::Mat comb_psf(psf1.rows, psf1.cols, psf1.type());
    double psf1_total = cv::sum(psf1)[0];
    double psf2_total = cv::sum(psf2)[0];

    int minX, maxX, minY, maxY;
    minX = comb_psf.cols;
    minY = comb_psf.rows;
    maxX = 0;
    maxY = 0;

    for(int i=0; i<comb_psf.rows; ++i){
        for(int j=0; j< comb_psf.cols; ++j){
            double val = (p2 * psf1.at<double>(i,j)/psf1_total + p1 * psf2.at<double>(i,j)/psf2_total);
            if(val>0.001){
                minX = j<minX ? j : minX;
                minY = i<minY ? i : minY;
                maxX = j>maxX ? j : maxX;
                maxY = i>maxY ? i : maxY;
            }
            comb_psf.at<double>(i,j) = val;
        }
    }

    int cenX = psf1.cols / 2;
    int cenY = psf1.rows / 2;

    int size_x = ((cenX-minX)>(maxX-cenX)) ? cenX-minX : maxX-cenX;
    int size_y = ((cenY-minY)>(maxY-cenY)) ? cenY-minY : maxY-cenY;



    return comb_psf.rowRange(cenY-size_y, cenY+size_y).colRange(cenX-size_x, cenX+size_x);

}
