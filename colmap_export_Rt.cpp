#include <colmap.h>
#include <sys/stat.h>

using namespace colmap;

int main(int argc,char** argv)
{
    std::cout << "colmap export R,t,f,ppx,ppy" << "\n";

    colmap::InitializeGlog(argv);
 
    std::string input_path="";
    std::string output_path=""; 

    colmap::OptionManager options; 
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path); 
    options.Parse(argc, argv);
    mkdir(output_path.c_str(),0X7FFFFFFF);

     // load the sparse reconstruction
    Reconstruction aSparseReconstr;
    aSparseReconstr.Read(input_path);
 
    // get all cameras
    EIGEN_STL_UMAP(camera_t, class Camera) aCams = aSparseReconstr.Cameras();


    //iterate over all images
    EIGEN_STL_UMAP(image_t, class Image) aImagesAll = aSparseReconstr.Images();




    for (auto aImPair : aImagesAll)
    {
        Image&   aIm = aSparseReconstr.Image(aImPair.first);
        camera_t aImCamId = aIm.CameraId();
   
        //camera file
        std::string aPathFile = output_path+"/" + aIm.Name() + ".camera"; 
        std::ofstream file(aPathFile, std::ios::trunc); 
        CHECK(file.is_open()) << aPathFile; 
        file.precision(17); 

        Eigen::Matrix3d aR = QuaternionToRotationMatrix(aIm.Qvec()).transpose();
        Eigen::Vector3d at = - aR*aIm.Tvec();
        
        file << aCams[aImCamId].FocalLengthX() << " 0 " << aCams[aImCamId].PrincipalPointX() << "\n";
        file << "0 " << aCams[aImCamId].FocalLengthY() << " " << aCams[aImCamId].PrincipalPointY() << "\n";
        file << "0 0 1\n0 0 0\n";
        file << aR.row(0) << "\n";
        file << aR.row(1) << "\n";
        file << aR.row(2) << "\n";
        file << at(0) << " " << at(1) << " " << at(2) << "\n";
        file << aCams[aImCamId].Width() << " " << aCams[aImCamId].Height() << "\n";     


        file.close();

    }

    return EXIT_SUCCESS;
}

//within Imae classe => Eigen::Vector4d& Qvec()
//Eigen::Matrix3d QuaternionToRotationMatrix(const Eigen::Vector4d& qvec)