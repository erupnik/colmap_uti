#include <colmap.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <boost/filesystem/operations.hpp>

#include <eigen3/Eigen/Geometry>


using namespace colmap;



int main(int argc, char** argv)
{
    std::string aImagePattern;
    std::string aImageDir="./";
    std::string aExt = ".ppm";
    std::string aExtIm = ".tif";
    Eigen::Vector3d aOffset;
    aOffset << 0.0, 0.0, 0.0;

    colmap::OptionManager options;
    options.AddRequiredOption("image_pattern", &aImagePattern);
    options.AddRequiredOption("image_dir", &aImageDir);
    options.AddDefaultOption("file_ext",&aExt);
    options.AddDefaultOption("im_ext",&aExtIm);
    options.AddDefaultOption("offx",&aOffset(0));
    options.AddDefaultOption("offy",&aOffset(1));
    options.AddDefaultOption("offz",&aOffset(2));
    options.AddDatabaseOptions();
    options.Parse(argc, argv);

    std::vector<std::string> aPoseList = GetFileList(aImagePattern);
    std::vector<std::string> aImageList = GetFileList(aImageDir);

    //read the database
    Database database(*options.database_path);

    // open file to save the images.txt
    std::string aImagesTxt = "images.txt";
    std::ofstream pFileIm(aImagesTxt, std::ios::trunc);
    CHECK(pFileIm.is_open()) << aImagesTxt;
    pFileIm.precision(15);

    // Z rotation MatrixX
    Eigen::Matrix<double, 3, 3> aRotZ;
    aRotZ(0,0) = 1.0;
    aRotZ(1,1) = -1.0;
    aRotZ(2,2) = -1.0;

    for (auto aPName : aPoseList)
    {
        //std::cout << boost::filesystem::extension(aPName) << "\n";

        if (aExt == boost::filesystem::extension(aPName))
        {

            // get image name
            auto aImName = boost::filesystem::basename(aPName) + aExtIm;
            //std::cout << aImageDir+aImName << " " << aImageList[0] << "\n";

            if (std::find(aImageList.begin(), aImageList.end(), aImageDir+aImName) != aImageList.end())
            {
              std::cout << aImageDir+aImName << "\n";

              // read camera and image ids
              Image aImCol = database.ReadImageWithName(aImName);
              camera_t aCamId = aImCol.CameraId();
              Camera Calib = database.ReadCamera(aCamId);

              Eigen::Matrix3d aK = Eigen::MatrixXd::Zero(3,3);
              aK(0,0) = aK(1,1) = Calib.FocalLength();
              aK(0,2) = Calib.PrincipalPointX();
              aK(1,2) = Calib.PrincipalPointY();
              aK(2,2) = 1.0;
              Eigen::Matrix3d aKInv = aK.inverse();

              Eigen::Matrix<double, 3, 4> aP;
              Eigen::Matrix<double, 3, 4> aPNorm;
              Eigen::Matrix<double, 3, 3> aR;


              FILE * pFile = fopen (aPName.c_str() , "r");
              char aLine[100];

              //1st line
              std::fgets (aLine, 100, pFile);
              int aNb=sscanf(aLine,"%lf %lf %lf %lf", &aP(0,0), &aP(0,1), &aP(0,2), &aP(0,3));

              //2nd line
              std::fgets (aLine, 100, pFile);
              aNb=sscanf(aLine,"%lf %lf %lf %lf", &aP(1,0), &aP(1,1), &aP(1,2), &aP(1,3));

              //3rd line
              std::fgets (aLine, 100, pFile);
              aNb=sscanf(aLine,"%lf %lf %lf %lf", &aP(2,0), &aP(2,1), &aP(2,2), &aP(2,3));

              //remove calibration matrix
              aPNorm = aKInv * aP;

              aR = aPNorm.leftCols<3>();


              std::cout << "C_MM=" << - aR.inverse() * aPNorm.col(3) << "\n";
              /*std::cout << "R" << aR.row(0) << "\n";
              std::cout << "R" << aR.row(1) << "\n";
              std::cout << "R" << aR.row(2) << "\n";
              getchar();*/

              //Eigen::Vector4d Qvec = RotationMatrixToQuaternion(aR);
              //Eigen::Vector3d Tvec = (aPNorm.col(3) - aOffset);

              Eigen::Matrix<double, 3, 3> aMatTmp = (aRotZ*aR*aRotZ).transpose();
              Eigen::Vector4d Qvec = RotationMatrixToQuaternion( aMatTmp );
              Eigen::Vector3d Tvec =  - (aRotZ * aMatTmp * aRotZ).inverse() *  (- aR.inverse()*(aPNorm.col(3) - aOffset));

              //Qvec = RotationMatrixToQuaternion[(RotZ.transpose * RotF * RotZ).transpose ]
              //T = -(RotZ* (RotZ.transpose * RotF * RotZ).transpose  *RotZ).inverse() * - RotF^-1 * TF
              /*std::cout << "T " << aPNorm.col(3) << "\n";
              std::cout << "R" << (aRotZ*aR*aRotZ).row(0) << "\n";
              std::cout << "R" << (aRotZ*aR*aRotZ).row(1) << "\n";
              std::cout << "R" << (aRotZ*aR*aRotZ).row(2) << "\n";*/


              //Tvec = -(aR.inverse() * Tvec);
              //std::cout << "Center " << Tvec.transpose() << "\n";
              //std::cout << aImCol.ImageId() << " " << Qvec.transpose() << " " << Tvec.transpose() << " " << aCamId << " " << aImName << "\n";
              //getchar();


              //#   IMAGE_ID, QW, QX, QY, QZ, TX, TY, TZ, CAMERA_ID, NAME
              pFileIm << aImCol.ImageId() << " " << Qvec(0) << " " << Qvec(1) << " " << Qvec(2) << " " << Qvec(3) << " " << Tvec(0) << " " << Tvec(1) << " " << Tvec(2) << " " << aCamId << " " << aImName << "\n";
              pFileIm << "\n";

            }


            /*
                RotM = rotation MicMac
                C = perspective center MicMac
                Tvec - translation Colmap
                Qvec - rotation Colmap
                RotF - rotation Fusiello
                TF - translation Fusiello

                MM to colmap
                Rc = (RotZ.transpose * RotM * RotZ).transpose
                Tc = -(RotZ*Rc*RotZ).inverse() * C


                Fusiello to MicMac, I assume calibration is removed
                C = - RotF^-1 * TF
                RotM = RotF

                Fusiello to MicMac to Colmap
                Qvec = RotationMatrixToQuaternion[(RotZ.transpose * RotF * RotZ).transpose ]
                T = -(RotZ* (RotZ.transpose * RotF * RotZ).transpose  *RotZ).inverse() * - RotF^-1 * TF

==========================
                OLD Micmac to Colmap
                Qvec^t = RotationMatrixToQuaternion(RotM)
                T = - RotM^-1 * C

            */
            /*
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
            */
        }
    }
    pFileIm.close();



    return EXIT_SUCCESS;
}
