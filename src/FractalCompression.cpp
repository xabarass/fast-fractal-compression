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

    if (argc != 2) {
        std::cout << "Usage: fractal-compression <path-to-image>\n";
        return ERR_NO_IMAGE_PATH;
    }
    std::string image_path(argv[1]);

    if (!exists_image(image_path.c_str())) {
        std::cout << "Provided image does not exist\n";
        return ERR_NO_IMAGE_PATH;
    }


    perf_init();

    cycles_count_start ();
    BMPImage img(image_path.c_str());
    img.Load();
    BMPImage res("result.bmp", img.GetWidth(), img.GetHeight(), img.GetChannels());
    Encoder enc;
    Transforms transforms;
    enc.Encode(img, res, &transforms);
    Decoder dec;
    dec.Decode(&transforms, img.GetHeight(), img.GetWidth());
    img.Save();
    int64_t cycles = cycles_count_stop ();
    cout<<"Counted cycles: "<<cycles<<endl;

    return 0;
}
