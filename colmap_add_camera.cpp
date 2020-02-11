#include <colmap.h>
#include <sys/stat.h>

using namespace colmap; 

int main(int argc, char** argv) 
{
    std::cout << "colmap modify intrinsics in the database and reconstruction (optional)" << "\n";
    
    colmap::InitializeGlog(argv);
 
    std::string input_path="";
    std::string output_path="";
    std::string camera_name="";
    camera_t camera_id;
    double aFocal=0;
    double aPPx=0, aPPy=0;
    double aSzX=0, aSzY=0;
    std::string aImgSubstr2Update="";
    bool PrintOnlyNumMatches=false;

    colmap::OptionManager options; 
    options.AddDatabaseOptions();
    options.AddRequiredOption("cam_id", &camera_id); 
    options.AddRequiredOption("cam_name", &camera_name);
    options.AddDefaultOption("focal", &aFocal); 
    options.AddDefaultOption("ppx", &aPPx);
    options.AddDefaultOption("ppy", &aPPy); 
    options.AddDefaultOption("szx", &aSzX);
    options.AddDefaultOption("szy", &aSzY);
    options.AddDefaultOption("numMatch", &PrintOnlyNumMatches);
    options.AddDefaultOption("input_path", &input_path);
    options.AddDefaultOption("output_path", &output_path);
     
    options.Parse(argc, argv);
    mkdir(output_path.c_str(),0X7FFFFFFF);
 
    //read the database
    Database database(*options.database_path);

    if (PrintOnlyNumMatches)
    {
        std::cout << "Num of matches=" << database.NumMatches() << "\n";

        return EXIT_SUCCESS;
    }


    Reconstruction aSparseReconstr;
    if (input_path!="" && output_path!="")
    {
        // load the sparse reconstruction
        aSparseReconstr.Read(input_path);
    
        // get all cameras
        EIGEN_STL_UMAP(camera_t, class Camera) aCams = aSparseReconstr.Cameras();


        // verify that the id of the camera to add does not exist  
        for (auto i : aCams)  
            if (camera_id==i.first)
            {
                std::cout << "cam_id already exists; change the identifier" << "\n";
                return EXIT_SUCCESS;
            }
             
    }
    // create the new camera  
    Camera aNewCam;
    //aNewCam.SetCameraId(camera_id);
    aNewCam.SetModelIdFromName(camera_name);

    // set the intrinsics
    aNewCam.SetFocalLength(aFocal);
    aNewCam.SetFocalLengthX(aFocal);
    aNewCam.SetFocalLengthY(aFocal);
    aNewCam.SetPrincipalPointX(aPPx);
    aNewCam.SetPrincipalPointY(aPPy);
    aNewCam.SetWidth(aSzX);
    aNewCam.SetHeight(aSzY);
    aNewCam.SetCameraId(database.WriteCamera(aNewCam));  
    //database.WriteCamera(aNewCam);

    // add the new camera
    if (input_path!="" && output_path!="")
    {
        aSparseReconstr.AddCamera(aNewCam);
        //aSparseReconstr.WriteText(output_path);
        aSparseReconstr.Write(output_path);
    }


    return EXIT_SUCCESS;
}