#include <iostream>
#include <GitVersion.h>
#include "emmintrin.h"
#include "include/perf.h"
#include <sys/stat.h>
#include <string>
#include <unistd.h>
#include <utils/BMPImage.h>
#include <encoder/Encoder.h>
#include <decoder/Decoder.h>

using namespace std;

inline bool exists_image(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

int main(int argc, char** argv){
    cout<<"    ______           __        ______                __        __   "<<endl;
    cout<<"   / ____/___ ______/ /_      / ____/________ ______/ /_____ _/ /   "<<endl;
    cout<<"  / /_  / __ `/ ___/ __/_____/ /_  / ___/ __ `/ ___/ __/ __ `/ /    "<<endl;
    cout<<" / __/ / /_/ (__  ) /_/_____/ __/ / /  / /_/ / /__/ /_/ /_/ / /     "<<endl;
    cout<<"/_/    \\__,_/____/\\__/     /_/   /_/   \\__,_/\\___/\\__/\\__,_/_/"<<endl;
    cout<<endl;
    cout<<"Git version: "<<" "<<s_GIT_SHA1_HASH<<" "<<s_GIT_REFSPEC<<endl;
    cout<<endl;

    // Load some parameters
    bool usage = true;
    int verb;
    u_int32_t threshold = 100;
    string image_path;
    u_int32_t maxphases = 5;
    string outputFile="result.bmp";
    string testFile="log.txt";
    for(int i=1; i<argc && usage; i++) {
        string param(argv[i]);
        if (param == "-v" && i + 1 < argc)
            verb = atoi(argv[i + 1]);
        else if (param == "-t" && i + 1 < argc)
            threshold = atoi(argv[i + 1]);
        else if (param == "-p" && i + 1 < argc)
            maxphases = atoi(argv[i + 1]);
        else if(param=="-o" && i + 1 < argc){
            outputFile=argv[i + 1];
        }
        else if(param=="-f" && i + 1 < argc){
            testFile=argv[i + 1];
        }
        if (param.at(0) == '-') i++;
        else {
            image_path = param;
            usage = false;
        }
    }

    if (argc < 2) {
        std::cout << "Usage: fractal-compression <path-to-image>\n";
        return ERR_NO_IMAGE_PATH;
    }

    if (!exists_image(image_path.c_str())) {
        std::cout << "Provided image does not exist\n";
        return ERR_NO_IMAGE_PATH;
    }

    perf_init();

    BMPImage img(image_path.c_str());
    img.Load();
    Encoder enc;
    Transforms transforms;

    cycles_count_start ();
    enc.Encode(img, &transforms, threshold);
    int64_t encodeCycles = cycles_count_stop ();

    printf("BLA Image height: %d, width: %d\n", img.GetHeight(), img.GetWidth());
    BMPImage result(outputFile, img.GetHeight(), img.GetWidth(), transforms.channels);
    Decoder dec;
    cycles_count_start ();
    dec.Decode(&transforms, result, maxphases);
    int64_t decodeCycles = cycles_count_stop ();
    result.Save();

    // Calculate compression ratio
    int64_t transformationsSize=0;
    int64_t transformationNumber=0;
    for(int i=0;i<transforms.channels;++i){
        transformationNumber+=transforms.ch[i].elements;
    }

    transformationsSize=transformationNumber*sizeof(struct ifs_transformation);
    int64_t originalSize=img.getSize();
    double compressionRatio=(double)originalSize/(double)transformationsSize;

    cout<<"#### PERFORMANCE RESULTS #####"<<endl;
    cout<<"Encode cycles: "<<encodeCycles<<endl;
    cout<<"Decode cycles: "<<decodeCycles<<endl;
    cout<<"No. of transformations: "<<transformationNumber<<endl;
    cout<<"Image size: w: "<<result.GetWidth()<<" h: "<<result.GetHeight()<<endl;
    cout<<"Compression ratio: "<<compressionRatio<<endl;

    freopen(testFile.c_str(),"w",stdout);
    for (int channel = 0; channel < transforms.channels; channel++) {
        // Iterate over Transforms
        struct ifs_transformation_list iter = transforms.ch[channel];
        struct ifs_transformation* temp = iter.head;
        while(temp != NULL) {
            print_transformation(*temp);
            temp = temp->next;
        }
    }

    //Free memory
    ifs_trans_clear_list(&transforms);

    return 0;
}
