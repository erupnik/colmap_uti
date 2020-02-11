#include <colmap.h>



using namespace colmap; 

int main(int argc, char** argv) {

    Timer timer;

    colmap::InitializeGlog(argv);
 
    std::string input_path;
    std::string image_list_path;
    std::string output_path;
    std::string camera_model = "OPENCV_FISHEYE";
    bool DoSfM = true;
    bool DoFeatures = true;

    colmap::OptionManager options;
    options.AddDatabaseOptions();
    options.AddRequiredOption("input_path", &input_path);
    options.AddRequiredOption("image_list_path", &image_list_path);
    options.AddRequiredOption("output_path", &output_path); 
    options.AddRequiredOption("camera_model",&camera_model);
    options.AddDefaultOption("DoSfM",&DoSfM,"Do SfM, default=true");
    options.AddDefaultOption("DoF",&DoFeatures,"Do feature extraction, default=true");
    options.AddImageOptions();
    options.AddExtractionOptions();
    options.AddSequentialMatchingOptions(); 
    options.AddBundleAdjustmentOptions();
    options.Parse(argc, argv);
 
    //is it necessary?
    Database database(*options.database_path);
 
    ImageReaderOptions reader_options = *options.image_reader;
    reader_options.database_path = *options.database_path;
    reader_options.image_path = *options.image_path;
    reader_options.camera_model = camera_model;
    reader_options.single_camera = true; 

    if (DoFeatures)
    {
        

        if (!image_list_path.empty()) 
        {
            reader_options.image_list = ReadTextFileLines(image_list_path);
        
            if (!reader_options.image_list.empty())
            {
                for (auto j : reader_options.image_list)
                std::cout << "image: " << j << "\n"; 
            }
            else
            { 
                std::cout << "emptyempty" << "\n";
                return EXIT_SUCCESS;
            } 
        }
        else 
            std::cout << "did not enter anywhere" << "\n";


        timer.Start();

        // Feature extraction
        std::cout << "==================== Feature extraction: " ;

        options.sift_extraction->use_gpu = false;  
        SiftFeatureExtractor feature_extractor(reader_options,
                                            *options.sift_extraction);
    
        feature_extractor.Start();
        feature_extractor.Wait(); 


        // Sequential matching
        std::cout << "==================== Feature matching: " ; 

        options.sift_matching->use_gpu = false;  
        SequentialFeatureMatcher feature_matcher(*options.sequential_matching,
                                                *options.sift_matching,
                                                *options.database_path);
    
        feature_matcher.Start();
        feature_matcher.Wait(); 
        std::cout << "==================== Feature extraction done in: " << "\n";
        timer.PrintMinutes();
    }      

    

    // Sfm
    if (DoSfM)
    { 
        timer.Start();
        std::cout << "==================== Structure from motion: " ;  

        ReconstructionManager reconstruction_manager; 

        IncrementalMapperController mapper( options.mapper.get(), *options.image_path,
                                            *options.database_path, &reconstruction_manager);

        std::cout << "mapper on" << "\n";
        Thread* active_thread_ = &mapper;
        mapper.Start();
        mapper.Wait();
        active_thread_ = nullptr;

        CreateDirIfNotExists(output_path);
        reconstruction_manager.Write(output_path, &options);

        std::cout << "==================== Sfm done in: " << "\n";
        timer.PrintMinutes();
    }
 

    return EXIT_SUCCESS;
}


  