#include <colmap.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <boost/filesystem/operations.hpp>

using namespace colmap;

static std::vector<std::string> list_dir(const char* path)
{
    std::vector<std::string> aRes;

    struct  dirent *entry;
    DIR *dir = opendir(path);

    if (dir==NULL)
    {
        return aRes;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        aRes.push_back(entry->d_name);
    }

    return aRes;
}

int main(int argc, char** argv)
{
    std::cout << "colmap_import_features_batch" << "\n";

    colmap::InitializeGlog(argv);

    std::string aImagePath;
    std::vector<std::string>  aImageList;
    std::string aHomName;
    double aFocal=0;
    double aPPx=0, aPPy=0;
    double aSzX=0, aSzY=0;
    bool Do2ViewGeom=false;
    bool OnlyCreateImsInDB = false;
    std::string CamModel = "SIMPLE_RADIAL";

    colmap::OptionManager options;
    options.AddRequiredOption("homol", &aHomName);
    options.AddRequiredOption("image_path", &aImagePath);
    options.AddDefaultOption("focal", &aFocal);
    options.AddDefaultOption("ppx", &aPPx);
    options.AddDefaultOption("ppy", &aPPy);
    options.AddDefaultOption("szx", &aSzX);
    options.AddDefaultOption("szy", &aSzY);
    options.AddDefaultOption("2ViewGeom", &Do2ViewGeom);
    options.AddDefaultOption("OnlyCreateIms", &OnlyCreateImsInDB);
    options.AddDefaultOption("CamModel", &CamModel);
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    aImageList = GetFileList(aImagePath);

    //read the database
    Database database(*options.database_path);


    if (OnlyCreateImsInDB)
    {
        /* Create a camera in the database */
        Camera camera;
        camera.InitializeWithName(CamModel, aFocal, aSzX, aSzY);
        camera.SetPrincipalPointX(aPPx);
        camera.SetPrincipalPointY(aPPy);
        camera.SetFocalLength(aFocal);
        camera.SetCameraId(database.WriteCamera(camera));


        /* Create images in the database */
        for (auto aIm1 : aImageList)
        {
            if ( (aIm1.find("png") != std::string::npos) || (aIm1.find("tif") != std::string::npos) || (aIm1.find("jpg") != std::string::npos))
            {
                aIm1 = aIm1.substr(2);
                auto aIm1Base = boost::filesystem::basename(aIm1) + boost::filesystem::extension(aIm1);

                Image aImCol;
                if (! database.ExistsImageWithName(aIm1Base))
                {
                    aImCol.SetName(aIm1Base);
                    aImCol.SetCameraId(camera.CameraId());
                    //aImCol.SetQvecPrior(Eigen::Vector4d(0.1, 0.2, 0.3, 0.4));
                    //aImCol.SetTvecPrior(Eigen::Vector3d(0.1, 0.2, 0.3));

                    aImCol.SetImageId(database.WriteImage(aImCol));

                    std::cout << aImCol.ImageId() << " " << aIm1Base << "\n";
                }
            }
        }
    }

    if (!OnlyCreateImsInDB)
    {
        /* Keypoints / matches for the entire dataset */
        std::map<std::string,FeatureKeypoints*> aFKPtsMap;
        std::vector<FeatureMatches> aFMatchesVec;
        std::vector<std::pair<int,int>> aFMatchesImId;


        /* For each image in the list */
        int aNb=0;
        for (auto aIm1 : aImageList)
        {
            if ( (aIm1.find("png") != std::string::npos) || (aIm1.find("tif") != std::string::npos) || (aIm1.find("jpg") != std::string::npos))
            {
                std::cout << aIm1 << "\n";
                aIm1 = aIm1.substr(2);

                auto aIm1Base = boost::filesystem::basename(aIm1) + boost::filesystem::extension(aIm1);


                //std::cout << boost::filesystem::basename(aIm1) << " ewelinaaa" << "\n";
                std::string aPastisDir = aHomName + "/Pastis" + aIm1Base;
                std::vector<std::string> aMatchedFiles = GetFileList(aPastisDir);

                //initialise the map for Im1 if necesary
                if (aFKPtsMap[aIm1Base] == NULL)
                    aFKPtsMap[aIm1Base] = new FeatureKeypoints;

                //read existing featurepoints (whether empty or not)
                FeatureKeypoints* aFKPts1 =  aFKPtsMap[aIm1Base];



                /* For a pair of images */
                for (auto aMatchFile : aMatchedFiles)
                {
                    std::size_t aPosIm2 = aMatchFile.find_last_of("/\\");
                    std::string aIm2 = aMatchFile.substr(aPosIm2+1);
                    aIm2 = aIm2.substr(0,aIm2.size()-4);//remove txt or dat

                    //if this match is part of image_path pattern
                    if (std::find(aImageList.begin(), aImageList.end(), aImagePath+aIm2) != aImageList.end())
                    {
                      std::cout << aIm1Base << " " <<  aIm2  << "\n";

                      //to be able to read the image ids of the current pair
                      Image aIm1Col = database.ReadImageWithName(aIm1Base);
                      Image aIm2Col = database.ReadImageWithName(aIm2);

                      //leave if symmetric features already exist
                      std::vector<std::pair<int,int>>::iterator aFMatchesImIdIter;
                      aFMatchesImIdIter = std::find(aFMatchesImId.begin(),aFMatchesImId.end(),std::pair<int,int>(aIm2Col.ImageId(),aIm1Col.ImageId()));

                      //if symmetric does not exist
                      if (aFMatchesImIdIter == aFMatchesImId.end())
                      {


                          //initialise the map for Im2 if necesary
                          if (aFKPtsMap[aIm2] == NULL)
                              aFKPtsMap[aIm2] = new FeatureKeypoints;

                          //read existing featurepoints  (whether empty or not)
                          FeatureKeypoints* aFKPts2 =  aFKPtsMap[aIm2];

                          //temp var to save matches
                          FeatureMatches aFMatches;

                          //single keypoints
                          FeatureKeypoint aKP1;
                          FeatureKeypoint aKP2;
                          double Pds;



                          //iterate the file lines
                          std::ifstream aFIn(aMatchFile.c_str(),std::ifstream::in);
                          while (aFIn.good())
                          {
                              aFIn >> aKP1.x >> aKP1.y  >> aKP2.x  >> aKP2.y >> Pds ;

                              //update the global features for the current pair
                              aFKPts1->push_back(aKP1);
                              aFKPts2->push_back(aKP2);

                              //save the global matches
                              aFMatches.push_back( FeatureMatch(aFKPts1->size()-1,aFKPts2->size()-1) );

                              //std::cout << aKP1.x << " " << aKP1.y << " " << aKP2.x << " " << aKP2.y <<  " " << aFKPts1->size()-1 << " " << aFKPts2->size()-1 << "\n";

                          }
                          //save the id's of the current pair

                          aFMatchesImId.push_back(std::pair<int,int>(aIm1Col.ImageId(),aIm2Col.ImageId()));
                          //save the matches for the current pair
                          aFMatchesVec.push_back(aFMatches);

                          aFIn.close();


                          /* Calculate two-view geometry */
                          if (Do2ViewGeom)
                          {


                              TwoViewGeometry two_view_geometry;
                              TwoViewGeometry::Options two_view_geometry_options;

                              camera_t camerat1 = aIm1Col.CameraId();
                              camera_t camerat2 = aIm2Col.CameraId();

                              Camera camera1 = database.ReadCamera(camerat1);
                              Camera camera2 = database.ReadCamera(camerat2);

                              std::cout << " 2ViewGeom=> " << camera1.CameraId() << " F=" << camera1.FocalLength() << " " << camera2.CameraId()  << " F=" << camera2.FocalLength() << "\n";
                              //getchar();

                              two_view_geometry_options.min_num_inliers = 10;
                              two_view_geometry_options.ransac_options.max_error = 4;
                              two_view_geometry_options.ransac_options.confidence = 0.999;
                              two_view_geometry_options.ransac_options.max_num_trials = 10000;
                              two_view_geometry_options.ransac_options.min_inlier_ratio = 0.25;

                              two_view_geometry.config = TwoViewGeometry::CALIBRATED;
                              //two_view_geometry.inlier_matches = aFMatches;
                              //two_view_geometry.Estimate(
                              //                camera1, FeatureKeypointsToPointsVector(*aFKPts1), camera2,
                              //                FeatureKeypointsToPointsVector(*aFKPts2), aFMatches,
                              //                two_view_geometry_options);
                              two_view_geometry.EstimateCalibrated(camera1,
                                                FeatureKeypointsToPointsVector(*aFKPts1),
                                                camera2,
                                                FeatureKeypointsToPointsVector(*aFKPts2),
                                                aFMatches,
                                                two_view_geometry_options);
                              std::cout << "E " << two_view_geometry.E << ", inliers=" << two_view_geometry.inlier_matches.size() << "\n";

                              //if (two_view_geometry.inlier_matches.size() > 0)
                              {
                                  database.WriteTwoViewGeometry(aIm1Col.ImageId(), aIm2Col.ImageId(),
                                                                two_view_geometry);
                              }



                          }
                      }
                    }
                }
            }
        }


        //clean database


        /* Update the databse with keypoints */
        for (auto aFKs : aFKPtsMap)
        {

            if (database.ExistsImageWithName(aFKs.first))
            {
                Image aImCol = database.ReadImageWithName(aFKs.first);
                std::cout << "image exists " <<  aImCol.ImageId() << " saving keypoints \n";
                database.WriteKeypoints(aImCol.ImageId(), *(aFKs.second));
            }
            else
            {
                std::cout << "Image " << aFKs.first << " does not exist in the database." << "\n";
            }

        }

        /* Update the databse with matches */
        std::cout << aFMatchesImId.size() << " " << aFMatchesVec.size() << "\n";
        if (aFMatchesImId.size() == aFMatchesVec.size())
            for (int aK=0; aK<int(aFMatchesVec.size()); aK++)
            {
                std::cout << "match " <<  aFMatchesImId.at(aK).first << " " << aFMatchesImId.at(aK).second << " saving matches \n";
                database.WriteMatches(aFMatchesImId.at(aK).first,aFMatchesImId.at(aK).second,aFMatchesVec.at(aK));
            }


    }
    database.Close();


    return EXIT_SUCCESS;
}
