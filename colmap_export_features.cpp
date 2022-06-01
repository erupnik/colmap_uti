#include <colmap.h>
#include <sys/stat.h>

using namespace colmap; 

bool SaveCoords(Database &aDb,std::vector<Image> &aImSet);
bool SaveTracks(Database &aDb,std::vector<std::pair<image_pair_t,FeatureMatches> > &aMatchSet);
bool SaveHomol(Database &aDb,std::vector<std::pair<image_pair_t,FeatureMatches> >& aMS,std::string& aSH);

int main(int argc,char** argv)
{
    std::cout << "colmap_export_features_main" << "\n";

    colmap::InitializeGlog(argv);

    std::string aSH="";

    colmap::OptionManager options;
    options.AddRequiredOption("SH", &aSH);
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    //read the database
    Database database(*options.database_path);

    //read all images  
    //std::vector<Image> aImSet = database.ReadAllImages();
    
    //read all matches
    std::vector<std::pair<image_pair_t,FeatureMatches> > aMatchSet = database.ReadAllMatches();
    

    //save Homol
    if (! (SaveHomol(database,aMatchSet,aSH)))
        return EXIT_FAILURE;


    //save as coords 
/*     if (!SaveCoords(&aImSet))
        return false; 
    //save tracks
    if (!SaveTracks(&aMatchSet))
        return false; */ 
    //                                   (first is camera id, second is points id)
    //std::map<int,std::vector<std::pair<int,int> > > aTracks;
    //typedef std::map<int,Pt2dr > tKeyPt;
    //std::map<int,tKeyPt >
    
    return EXIT_SUCCESS;
}

bool SaveHomol(Database &aDb,std::vector<std::pair<image_pair_t,FeatureMatches> >& aMS,std::string& aSH)
{
    std::string aPathHomol = "Homol"+aSH+"/";
    mkdir(aPathHomol.c_str(),0X7FFFFFFF);
      
    for (auto pair : aMS)
    {

        //get images ids and names
        image_t aImId1, aImId2;
        Image aIm1,aIm2;
        aDb.PairIdToImagePair(pair.first,&aImId1,&aImId2);

        aIm1 = aDb.ReadImage(aImId1);
        aIm2 = aDb.ReadImage(aImId2);
        std::string aIm1Name = aIm1.Name();
        std::string aIm2Name = aIm2.Name();

        std::string slash = "/";
        size_t pos_slash = aIm1Name.find(slash);

        if( pos_slash != std::string::npos)
        {
            aIm1Name.replace(pos_slash, slash.size(), "-");
        }
        pos_slash = aIm2Name.find(slash);
        if( pos_slash != std::string::npos)
        {
            aIm2Name.replace(pos_slash, slash.size(), "-");
        }

        //read keypoints
        FeatureKeypoints aFeatureIm1 = aDb.ReadKeypoints(aImId1);
        FeatureKeypoints aFeatureIm2 = aDb.ReadKeypoints(aImId2);
        //std::cout << "Size pts " << aFeatureIm1.size() << " " <<  aFeatureIm2.size() << "\n";

        //open files for writing
        std::string aPastis1 = aPathHomol+"Pastis"+aIm1Name;
        std::string aPastis2 = aPathHomol+"Pastis"+aIm2Name;

        mkdir(aPastis1.c_str(),0X7FFFFFFF);
        mkdir(aPastis2.c_str(),0X7FFFFFFF);

        std::string aPathFile12 = aPastis1+"/"+aIm2Name+".txt";
        std::string aPathFile21 = aPastis2+"/"+aIm1Name+".txt";

        std::ofstream file12(aPathFile12, std::ios::trunc);
        std::ofstream file21(aPathFile21, std::ios::trunc);
        CHECK(file12.is_open()) << aPathFile12;
        CHECK(file21.is_open()) << aPathFile21;
        file12.precision(17);
        file21.precision(17);

        

        for (auto feature : pair.second)
        { 
            //std::cout << feature.point2D_idx1 << " " << feature.point2D_idx2 <<   "\n";
            //std::cout << aFeatureIm1[feature.point2D_idx1].x << " " << aFeatureIm1[feature.point2D_idx1].y << "\n";

            file12 << aFeatureIm1[feature.point2D_idx1].x << " " << aFeatureIm1[feature.point2D_idx1].y << " " 
                   << aFeatureIm2[feature.point2D_idx2].x << " " << aFeatureIm2[feature.point2D_idx2].y << "\n";

            file21 << aFeatureIm2[feature.point2D_idx2].x << " " << aFeatureIm2[feature.point2D_idx2].y << " "
                   << aFeatureIm1[feature.point2D_idx1].x << " " << aFeatureIm1[feature.point2D_idx1].y << "\n";
 
        }

        file12.close();
        file21.close();



        //


    }

    

    return true;
}
 
bool SaveCoords(Database &aDb,std::vector<Image> &aImSet)
{
    std::ofstream file("coords.txt", std::ios::trunc);
    CHECK(file.is_open()) << "coords.txt";
     
    file.precision(17);

    
    for (auto aIm : aImSet)
    {
        std::ostringstream line;
        std::string line_string;

       // FeatureKeypoints aDb.ReadKeypoints()
        /*std::cout << "dddddd " << aIm.first << "\n";
        line << aIm.first << "\n";

        line_string = line.str();
        line_string = line_string.substr(0, line_string.size() - 1);
        file << line_string << std::endl;*/
    }

    file.close();

    return true;
}

bool SaveTracks(Database &aDb,std::vector<std::pair<image_pair_t,FeatureMatches> > &aMatchSet)
{
    
    //
    //number of keypoints
    int aNbKP = aDb.NumMatches();
    std::cout << "aNbKP=" << aNbKP << "\n";

    for (auto aPair : aMatchSet)
    {

        std::cout << "dhdhdhd" << "\n";
    }

    return true;
}

    /*Camera camera;
    camera.SetCameraId(database.WriteCamera(camera));
    Image image;
    image.SetName("test");
    image.SetCameraId(camera.CameraId());
    image.SetImageId(database.WriteImage(image));
    BOOST_CHECK_EQUAL(database.NumKeypoints(), 0);
    BOOST_CHECK_EQUAL(database.NumKeypointsForImage(image.ImageId()), 0);
    const FeatureKeypoints keypoints = FeatureKeypoints(10);
    database.WriteKeypoints(image.ImageId(), keypoints);*/

    /*Camera camera;
    camera.SetCameraId(database.WriteCamera(camera));
    Image image;
    image.SetName("test");
    image.SetCameraId(camera.CameraId());
    image.SetImageId(database.WriteImage(image));
    BOOST_CHECK_EQUAL(database.NumDescriptors(), 0);
    BOOST_CHECK_EQUAL(database.NumDescriptorsForImage(image.ImageId()), 0);
    const FeatureDescriptors descriptors = FeatureDescriptors::Random(10, 128);
    database.WriteDescriptors(image.ImageId(), descriptors);*/

    /*
    const image_t image_id1 = 1;
    const image_t image_id2 = 2;
    const FeatureMatches matches = FeatureMatches(1000);
    database.WriteMatches(image_id1, image_id2, matches);
    */
