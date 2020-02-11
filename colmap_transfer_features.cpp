#include <colmap.h>
#include <sys/stat.h>

using namespace colmap; 
 

int main(int argc,char** argv)
{
    std::cout << "colmap_export_features_main" << "\n";

    colmap::InitializeGlog(argv);

    std::string aNewDbName = "Db_New.db";
    std::string aPat2Transfer="";
    double aFocal=0;
    double aPPx=0, aPPy=0;
    double aSzX=0, aSzY=0;

    colmap::OptionManager options;
    options.AddRequiredOption("new_db", &aNewDbName); //name of new database
    options.AddRequiredOption("Pattern", &aPat2Transfer); //pattern to localise the images you want to transfer
    options.AddDefaultOption("focal", &aFocal); 
    options.AddDefaultOption("ppx", &aPPx);
    options.AddDefaultOption("ppy", &aPPy); 
    options.AddDefaultOption("szx", &aSzX);
    options.AddDefaultOption("szy", &aSzY);
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    //read the database
    Database aDb(*options.database_path);

    
    Database aDbNew(aNewDbName); 

    //for some reason I must initialise the database with a camera
    Camera aNewCam;
    aNewCam.SetModelIdFromName("OPENCV_FISHEYE");
     
    //aNewCam.SetCameraId(100); 

    // set the intrinsics
    aNewCam.SetFocalLength(aFocal);
    aNewCam.SetFocalLengthX(aFocal);
    aNewCam.SetFocalLengthY(aFocal);
    aNewCam.SetPrincipalPointX(aPPx);
    aNewCam.SetPrincipalPointY(aPPy);
    aNewCam.SetWidth(aSzX);
    aNewCam.SetHeight(aSzY);
    aNewCam.SetCameraId(aDbNew.WriteCamera(aNewCam));  
     

    //read all matches
    std::vector<std::pair<image_pair_t,FeatureMatches> > aMatchSet = aDb.ReadAllMatches();

    for (auto pair : aMatchSet)
    {
        //get images ids and names
        image_t aImId1, aImId2;
        Image aIm1,aIm2;
        aDb.PairIdToImagePair(pair.first,&aImId1,&aImId2);
 
        aIm1 = aDb.ReadImage(aImId1);
        aIm2 = aDb.ReadImage(aImId2); 

                  
        std::string WhichCamera1 = aIm1.Name().substr(aIm1.Name().size() - aPat2Transfer.size(), aIm1.Name().size() - 1);
        std::string WhichCamera2 = aIm2.Name().substr(aIm2.Name().size() - aPat2Transfer.size(), aIm2.Name().size() - 1);
         

        if (WhichCamera1 == aPat2Transfer)
        {
            if (WhichCamera2 == aPat2Transfer)
            {
                    

                //check if cameras exist in the database
                if (! aDbNew.ExistsCamera(aIm1.CameraId()))
                { 
                    aDbNew.WriteCamera(aDb.ReadCamera(aIm1.CameraId())); 
                }
                if (! aDbNew.ExistsCamera(aIm2.CameraId()))
                { 
                    aDbNew.WriteCamera(aDb.ReadCamera(aIm2.CameraId())); 
                } 

                
  
    
                //it is necessary to pass by variables 
                Image aIm1Ndb;
                if (! aDbNew.ExistsImageWithName(aIm1.Name()))
                { 
                    
                    aIm1Ndb.SetName(aIm1.Name()); 
                    //aIm1Ndb.SetCameraId(aIm1.CameraId());
                    aIm1Ndb.SetCameraId(aNewCam.CameraId());
                    aIm1Ndb.SetQvecPrior(Eigen::Vector4d(0.1, 0.2, 0.3, 0.4));
                    aIm1Ndb.SetTvecPrior(Eigen::Vector3d(0.1, 0.2, 0.3));
                    aIm1Ndb.SetImageId(aDbNew.WriteImage(aIm1Ndb));

                    //get and assign all features for that image
                    FeatureKeypoints aFeatureIm1 = aDb.ReadKeypoints(aImId1); 
                    aDbNew.WriteKeypoints(aIm1Ndb.ImageId()  ,aFeatureIm1);
                }
                else //get the image
                {
                    aIm1Ndb = aDbNew.ReadImageWithName(aIm1.Name());
                }
                 
                Image aIm2Ndb;
                if (! aDbNew.ExistsImageWithName(aIm2.Name()))
                { 
                    
                    aIm2Ndb.SetName(aIm2.Name()); 
                    //aIm2Ndb.SetCameraId(aIm2.CameraId());
                    aIm2Ndb.SetCameraId(aNewCam.CameraId());
                    aIm2Ndb.SetQvecPrior(Eigen::Vector4d(0.1, 0.2, 0.3, 0.4));
                    aIm2Ndb.SetTvecPrior(Eigen::Vector3d(0.1, 0.2, 0.3));
                    aIm2Ndb.SetImageId(aDbNew.WriteImage(aIm2Ndb));

                    //get and assign all features for that image
                    FeatureKeypoints aFeatureIm2 = aDb.ReadKeypoints(aImId2); 
                    aDbNew.WriteKeypoints(aIm2Ndb.ImageId()  ,aFeatureIm2);

                }
                else //get the image
                {
                    aIm2Ndb = aDbNew.ReadImageWithName(aIm2.Name());
                }
                 
                //get and assign matches  
                FeatureMatches aFeatMatches = aDb.ReadMatches(aImId1,aImId2); 
                aDbNew.WriteMatches(aIm1Ndb.ImageId() ,aIm2Ndb.ImageId() ,aFeatMatches);  
                
                std::cout << aIm1.Name() << " " << aIm2.Name() << "    YESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSsss"<< "\n";

            }
            
                     
        }   

        
 
    }
    aDbNew.Close();
    aDb.Close();

    return EXIT_SUCCESS;

}