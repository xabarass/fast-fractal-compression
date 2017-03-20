#include <iostream>
#include <GitVersion.h>
#include "emmintrin.h"
#include "include/perf.h"

#include <utils/BMPImage.h>
#include <encoder/Encoder.h>

using namespace std;

int main(){
    cout<<"    ______           __        ______                __        __   "<<endl;
    cout<<"   / ____/___ ______/ /_      / ____/________ ______/ /_____ _/ /   "<<endl;
    cout<<"  / /_  / __ `/ ___/ __/_____/ /_  / ___/ __ `/ ___/ __/ __ `/ /    "<<endl;
    cout<<" / __/ / /_/ (__  ) /_/_____/ __/ / /  / /_/ / /__/ /_/ /_/ / /     "<<endl;
    cout<<"/_/    \\__,_/____/\\__/     /_/   /_/   \\__,_/\\___/\\__/\\__,_/_/"<<endl;
    cout<<endl;
    cout<<"Git version: "<<" "<<s_GIT_SHA1_HASH<<" "<<s_GIT_REFSPEC<<endl;
    cout<<endl;

    perf_init();

    cycles_count_start ();
    BMPImage img("test_images/lena.bmp");
    img.Load();
    Encoder enc;
    enc.Encode(img);
    img.Save();
    int64_t cycles = cycles_count_stop ();
    cout<<"Counted cycles: "<<cycles<<endl;

    return 0;
}
