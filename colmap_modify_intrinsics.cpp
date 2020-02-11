#include <colmap.h>
#include <sys/stat.h>

using namespace colmap; 

int main(int argc, char** argv) 
{
    std::cout << "colmap modify intrinsics" << "\n";
    
    colmap::InitializeGlog(argv);
 
    std::string input_path="";
    std::string output_path="";
    camera_t camera_id; 
    std::string aImgSubstr2Update="";

    colmap::OptionManager options; 
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("output_path", &output_path);
    options.AddDefaultOption("cam_id", &camera_id);  
    options.AddDefaultOption("substr", &aImgSubstr2Update);
    options.Parse(argc, argv);
    mkdir(output_path.c_str(),0X7FFFFFFF);
 

    // load the sparse reconstruction
    Reconstruction aSparseReconstr;
    aSparseReconstr.Read(input_path);
 
    // get all cameras
    EIGEN_STL_UMAP(camera_t, class Camera) aCams = aSparseReconstr.Cameras();


    // verify that the camera to change exists
    bool aFoundCam = false;
    for (auto i : aCams)  
        if (camera_id==i.first)
            aFoundCam=true;
        
    if (!aFoundCam)
    {
        std::cout << "cam_id not found" << "\n";
        return EXIT_SUCCESS;
    }       
 
    

    // re-assign camera identifiers for a subset of images
    if (aImgSubstr2Update.size())
    {
 
        //assign the new camera
        EIGEN_STL_UMAP(image_t, class Image) aImagesAll = aSparseReconstr.Images();

        for (auto aImPair : aImagesAll)
        {
            Image& aIm = aSparseReconstr.Image(aImPair.first);

                
                std::string WhichCamera = aIm.Name().substr(aIm.Name().size() - aImgSubstr2Update.size(), aIm.Name().size() - 1);
                 
                if (WhichCamera == aImgSubstr2Update)
                {
                    aIm.SetCameraId(camera_id);
                    //std::cout << "which camera " << WhichCamera << "\n";
                }
        }

    }

    //aSparseReconstr.WriteText(output_path);
    aSparseReconstr.Write(output_path);

    return EXIT_SUCCESS;
}