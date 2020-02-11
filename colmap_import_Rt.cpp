#include <colmap.h>
#include <math.h>
#include <sys/stat.h>

using namespace colmap;

class WriteColampReco
{
    public :
        WriteColampReco(std::string& bundler_file_name, std::string& output_path);

        bool ReadBundler();
        bool WriteRec(Database& aDb);

    private:

        std::string mBundlerFile;
        std::string mOutput;
        std::vector<pba::CameraT> mCamera_data;
        std::vector<std::string> mNames;
};

WriteColampReco::WriteColampReco(std::string& bundler_file_name, std::string& output_path) :
    mBundlerFile(bundler_file_name),
    mOutput(output_path)
{
}


int main(int argc,char** argv)
{
    std::cout << "colmap import R,t,f,ppx,ppy to a reconstruction" << "\n";

    colmap::InitializeGlog(argv);
 
    std::string bundler_file="";
    std::string output_dir=""; 

    colmap::OptionManager options; 
    options.AddRequiredOption("bundler", &bundler_file, "must have .out extention");
    options.AddRequiredOption("output", &output_dir);
    options.AddDatabaseOptions(); 
    options.Parse(argc, argv);
    mkdir(output_dir.c_str(),0X7FFFFFFF);

    //read the database
    Database database(*options.database_path);

    WriteColampReco aExportRt(bundler_file,output_dir);
    
    //read the bundler file
    if (aExportRt.ReadBundler())
        return EXIT_FAILURE;

    //save to colamp reconstruction
    if (aExportRt.WriteRec(database))
        return EXIT_FAILURE;


    return EXIT_SUCCESS;
}

bool WriteColampReco::WriteRec(Database& aDb)
{
     
    std::string path = mOutput+"/images.txt";
    std::ofstream file(path, std::ios::trunc);
    CHECK(file.is_open()) << path;

    // Ensure that we don't loose any precision by storing in text.
    file.precision(17);

    file << "# Image list with two lines of data per image:" << std::endl;
    file << "#   IMAGE_ID, QW, QX, QY, QZ, TX, TY, TZ, CAMERA_ID, "
            "NAME"
        << std::endl;
    file << "#   POINTS2D[] as (X, Y, POINT3D_ID)" << std::endl; 
    file << "# Number of images: " << mNames.size()  
        << ", mean observations per image: "
        << 100 << std::endl; 

    for (int aK=0; aK<int(mNames.size()); aK++)
    {
        Image aIm = aDb.ReadImageWithName(mNames.at(aK));

        //float c[3];
        float t[3];
        float q[4];
        //mCamera_data.at(aK).GetCameraCenter(c);
        mCamera_data.at(aK).GetTranslation(t);
        mCamera_data.at(aK).GetQuaternionRotation(q);


        std::cout << "rotation:" << q[0] << " camera center:" << t[0] <<    " " << mNames.at(aK) << " " << aIm.ImageId() << "\n";
        
        std::ostringstream line;
        std::string line_string;

        line << aIm.ImageId()  << " ";

        // QVEC (qw, qx, qy, qz)
        const Eigen::Vector4d normalized_qvec =  NormalizeQuaternion(Eigen::Vector4d(q[0],q[1],q[2],q[3]));
        line << normalized_qvec(0) << " ";
        line << normalized_qvec(1) << " ";
        line << normalized_qvec(2) << " ";
        line << normalized_qvec(3) << " ";
        std::cout << "q=" << normalized_qvec << " t=" << t[0] << " " << t[1] << " " << t[2] << "\n";

        // TVEC
        line << t[0] << " ";
        line << t[1] << " ";
        line << t[2] << " ";

        line << aIm.CameraId() << " ";

        line << mNames.at(aK);

        file << line.str() << "\n";

        line.str("");
        line.clear();

        //empty line for 2d correspondences
        //file << "\n";        

        //update the image priors
        Eigen::Vector3d tVec(t[0],t[1],t[2]);
        aIm.SetQvecPrior(normalized_qvec);
        aIm.SetQvec(normalized_qvec);
        aIm.SetTvecPrior(tVec);
        aIm.SetTvec(tVec);
          
        /*std::vector<Eigen::Vector2d> aPt2dIm;
        FeatureKeypoints aFKpts = aDb.ReadKeypoints(aIm.ImageId()); 
        for (const FeatureKeypoint& point2D : aFKpts) 
        {
            line << point2D.x << " ";
            line << point2D.y << " "; 
            line << -1 << " ";

            aPt2dIm.push_back(Eigen::Vector2d(point2D.x,point2D.y));            
            
        } 

        line_string = line.str();
        line_string = line_string.substr(0, line_string.size() - 1);
        file << line_string << std::endl;  
 
        aIm.SetPoints2D(aPt2dIm);*/
        aDb.UpdateImage(aIm);
    }
    file.close();

    return EXIT_SUCCESS;
}

bool WriteColampReco::ReadBundler()
{
    std::ifstream in(mBundlerFile.c_str(),std::ifstream::in); 
    
    int rotation_parameter_num = 9;
    std::string token;
    while (in.peek() == '#') std::getline(in, token); 
    char listpath[1024], filepath[1024];
    strcpy(listpath, mBundlerFile.c_str()); 
    char* ext = strstr(listpath, ".out"); 
    strcpy(ext, "-list.txt\0");

     
    std::ifstream listin(listpath);
    if (!listin.is_open()) 
    {
        listin.close();
        listin.clear();
        char* slash = strrchr(listpath, '/');
        if (slash == NULL) slash = strrchr(listpath, '\\');
        slash = slash ? slash + 1 : listpath;
        strcpy(slash, "image_list.txt");
        listin.open(listpath);
    }
    if (listin) std::cout << "Using image list: " << listpath << '\n';


    // read # of cameras
    int ncam = 0, npoint = 0, nproj = 0;
    in >> ncam >> npoint;
    if (ncam <= 1) return false;
    std::cout << ncam << " cameras; " << npoint << " 3D points;\n";


    // read the camera parameters
    mCamera_data.resize(ncam);  // allocate the camera data
    mNames.resize(ncam);

    bool det_checked = false;
    for (int i = 0; i < ncam; ++i) 
    {
        float f, q[9], c[3], d[2];
        in >> f >> d[0] >> d[1];
        for (int j = 0; j < rotation_parameter_num; ++j) in >> q[j];
        in >> c[0] >> c[1] >> c[2];

        std::cout << "f " << f << " " << q[0] << " " << c[0] << "\n";
    

        mCamera_data[i].SetFocalLength(f);
        mCamera_data[i].SetInvertedR9T(q, c);
        mCamera_data[i].SetProjectionDistortion(d[0]);
     
 
        if (listin >> filepath && f != 0) 
        {
            char* slash = strrchr(filepath, '/');
            if (slash == NULL) slash = strchr(filepath, '\\');
            mNames[i] = (slash ? (slash + 1) : filepath);
            std::cout << "name=" << mNames[i] << "\n";
            std::getline(listin, token);

            if (!det_checked) 
            {
                float det = mCamera_data[i].GetRotationMatrixDeterminant();
                std::cout << "Check rotation matrix: " << det << '\n';
                det_checked = true;
            }
        } else 
        {
            mNames[i] = "unknown";
        }
    }

    return EXIT_SUCCESS;
}