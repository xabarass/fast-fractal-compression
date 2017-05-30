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

#ifdef COUNT_FLOPS
    struct global_op_count __GLOBAL_OP_COUNT;
#endif

#ifdef COUNT_DETAIL_CYCLES
    struct global_function_cycle_count __COUNT_DETAIL_CYCLES;
#endif
void init_counting_flops(){
#ifdef COUNT_FLOPS
    __GLOBAL_OP_COUNT.int_adds=0;
    __GLOBAL_OP_COUNT.int_mults=0;
    __GLOBAL_OP_COUNT.fp_adds=0;
    __GLOBAL_OP_COUNT.fp_mults=0;
#endif
}

void init_counting_cycles(){
#ifdef COUNT_DETAIL_CYCLES
    __COUNT_DETAIL_CYCLES.get_average_pixel_cycles=0;
    __COUNT_DETAIL_CYCLES.get_error_cycles=0;
    __COUNT_DETAIL_CYCLES.get_scale_factor_cycles=0;
    __COUNT_DETAIL_CYCLES.down_sample_cycles=0;
    __COUNT_DETAIL_CYCLES.ifs_transformation_execute_cycles=0;
#endif
}
void print_detailed_cycles(){
#ifdef COUNT_DETAIL_CYCLES
    cout<<"get_average_pixel_cycles: "<<__COUNT_DETAIL_CYCLES.get_average_pixel_cycles<<endl;
    cout<<"get_error_cycles: "<<__COUNT_DETAIL_CYCLES.get_error_cycles<<endl;
    cout<<"get_scale_factor_cycles: "<<__COUNT_DETAIL_CYCLES.get_scale_factor_cycles<<endl;
    cout<<"down_sample_cycles: "<<__COUNT_DETAIL_CYCLES.down_sample_cycles<<endl;
    cout<<"ifs_transformation_execute_cycles: "<<__COUNT_DETAIL_CYCLES.ifs_transformation_execute_cycles<<endl;

#endif
}

void print_op_count(const char* name){
#ifdef COUNT_FLOPS
    cout<<"### Performance count for "<<name<<endl;
    cout<<"int add: "<<__GLOBAL_OP_COUNT.int_adds<<endl;
    cout<<"int mul: "<<__GLOBAL_OP_COUNT.int_mults<<endl;
    cout<<"fp add: "<<__GLOBAL_OP_COUNT.fp_adds<<endl;
    cout<<"fp mul: "<<__GLOBAL_OP_COUNT.fp_mults<<endl;
#endif
}

void print_detail_cycles(const char* name){
#ifdef COUNT_DETAIL_CYCLES

#endif
}

int main(int argc, char** argv){
    cout<<"    ______           __        ______                __        __   "<<endl;
    cout<<"   / ____/___ ______/ /_      / ____/________ ______/ /_____ _/ /   "<<endl;
    cout<<"  / /_  / __ `/ ___/ __/_____/ /_  / ___/ __ `/ ___/ __/ __ `/ /    "<<endl;
    cout<<" / __/ / /_/ (__  ) /_/_____/ __/ / /  / /_/ / /__/ /_/ /_/ / /     "<<endl;
    cout<<"/_/    \\__,_/____/\\__/     /_/   /_/   \\__,_/\\___/\\__/\\__,_/_/"<<endl;
    cout<<endl;
    cout<<"Git version: "<<" "<<s_GIT_SHA1_HASH<<" "<<s_GIT_REFSPEC<<endl;
    cout<<"Build type:"<<UTILS_TYPE<<MATCHING_TYPE<<endl;

    // Load some parameters
    bool usage = true;
    int verb;
    u_int32_t threshold = 100;
    string image_path;
    u_int32_t maxphases = 5;
    string outputFile="result.bmp";
    string testFile="log.txt";
    bool decode=true;
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
        else if(param=="-d" && i+1<argc){
            decode=(string(argv[i+1])==string("1"))?true:false;
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

    // Encoding part
    Encoder enc;
    Transforms transforms;
    ifs_trans_init_transformations(&transforms, img.GetChannels());
    init_counting_flops();
    init_counting_cycles();
    cycles_count_start();

    enc.Encode(img, &transforms, threshold);

    int64_t encodeCycles = cycles_count_stop ();
    print_detailed_cycles();
    print_op_count("encoder");

    printf("Image height: %d, width: %d\n", img.GetHeight(), img.GetWidth());

    // Decoder part
    int64_t decodeCycles=0;
    if(decode){
        BMPImage result(outputFile, img.GetHeight(), img.GetWidth(), transforms.channels);
        Decoder dec;
        init_counting_flops();
        cycles_count_start ();
        dec.Decode(&transforms, result, maxphases);
        decodeCycles = cycles_count_stop ();
        print_op_count("decoder");
        result.Save();
    }else{
        cout<<"Not running decoding"<<endl;
    }

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
    cout<<"Image size: w: "<<img.GetWidth()<<" h: "<<img.GetHeight()<<endl;
    cout<<"Compression ratio: "<<compressionRatio<<endl;

    //Free memory
    ifs_trans_clear_list(&transforms);

    return 0;
}
