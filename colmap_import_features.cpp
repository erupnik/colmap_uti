#include <colmap.h>
#include <sys/stat.h>

using namespace colmap; 

int main(int argc,char** argv)
{
    std::cout << "colmap_import_features_main" << "\n";

    colmap::InitializeGlog(argv);

    std::string aIm1Name;
    std::string aIm2Name;
    std::string aHomName; 

    colmap::OptionManager options;
    options.AddRequiredOption("homol", &aHomName);
    options.AddRequiredOption("image1", &aIm1Name);
    options.AddRequiredOption("image2", &aIm2Name);
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    //read the database
    Database database(*options.database_path);
    
    Camera camera;
    camera.InitializeWithName("SIMPLE_PINHOLE", 1.0, 1, 1);
    camera.SetCameraId(database.WriteCamera(camera));  

    Image aIm1;
    aIm1.SetName(aIm1Name);
    //aIm1.SetImageId(0);
    //Image aIm1 = database.ReadImageWithName(aIm1Name);
    aIm1.SetCameraId(camera.CameraId());
    aIm1.SetQvecPrior(Eigen::Vector4d(0.1, 0.2, 0.3, 0.4));
    aIm1.SetTvecPrior(Eigen::Vector3d(0.1, 0.2, 0.3));

    aIm1.SetImageId(database.WriteImage(aIm1));

 
    Image aIm2;
    aIm2.SetName(aIm2Name);
    //aIm2.SetImageId(1);
    //Image aIm2 = database.ReadImageWithName(aIm2Name);
    aIm2.SetCameraId(camera.CameraId());
    aIm2.SetQvecPrior(Eigen::Vector4d(0.1, 0.2, 0.3, 0.4));
    aIm2.SetTvecPrior(Eigen::Vector3d(0.1, 0.2, 0.3));
    aIm2.SetImageId(database.WriteImage(aIm2));
 
  
  /*  aIm1.SetCameraId(camera.CameraId());
    aIm1.SetImageId(database.WriteImage(aIm1));

    aIm2.SetCameraId(camera.CameraId());
    aIm2.SetImageId(database.WriteImage(aIm2)); */

    FeatureMatches aFeatMatches;
    FeatureKeypoints aFeatKP1;
    FeatureKeypoints aFeatKP2;

    std::ifstream aFIn(aHomName.c_str(),std::ifstream::in);
 

     
    int aKPId=0;
    while (aFIn.good())
    {
         
        FeatureKeypoint aKP1;
        FeatureKeypoint aKP2; 

        //double aPond;
        //aFIn >> aKP1.x >> aKP1.y  >> aKP2.x  >> aKP2.y  >> aPond ;
        aFIn >> aKP1.x >> aKP1.y  >> aKP2.x  >> aKP2.y  ;
        
        std::cout << aKP1.x << " " << aKP1.y << " " << aKP2.x << " " << aKP2.y << "\n"; 
        

        aFeatKP1.push_back(aKP1);
        aFeatKP2.push_back(aKP2);

        aFeatMatches.push_back( FeatureMatch(aKPId,aKPId) );
        //FeatureMatch(const point2D_t point2D_idx1, const point2D_t point2D_idx2)
        
        aKPId++;
        std::cout << aKPId << " " << aIm1.ImageId()  << " " << aIm2.ImageId()  << " " << "\n";
    }
    std::cout << aKPId << " " << aIm1.ImageId()  << " " << aIm2.ImageId()  << " " << "\n";
    
    
    database.WriteKeypoints(aIm1.ImageId()  ,aFeatKP1);
    database.WriteKeypoints(aIm2.ImageId()  ,aFeatKP2);
    database.WriteMatches(aIm1.ImageId() ,aIm2.ImageId() ,aFeatMatches);

    database.Close();
    aFIn.close();
    /*
    for all  



    */

/* 
// Write a new entry in the database. The user is responsible for making sure
  // that the entry does not yet exist. For image pairs, the order of
  // `image_id1` and `image_id2` does not matter.
  void WriteKeypoints(const image_t image_id,
                      const FeatureKeypoints& keypoints) const;
  void WriteDescriptors(const image_t image_id,
                        const FeatureDescriptors& descriptors) const;
  void WriteMatches(const image_t image_id1, const image_t image_id2,
                    const FeatureMatches& matches) const; 

*/


    return EXIT_SUCCESS;
}