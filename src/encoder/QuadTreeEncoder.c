#include "QuadTreeEncoder.h"

#define MODULE_NAME "QuadTreeEncoder"

/*******
 * Local params used to tweak sizes of elements in cache
 */
#define CHUNK_SIZE 1
#define MAX_RANGE_BLOCKS ((BUFFER_SIZE/2)*(BUFFER_SIZE/2))
#define MAX_RANGE_BLOCKS_PER_CHUNK (MAX_RANGE_BLOCKS*CHUNK_SIZE)
#define CHUNK_LINE ((BUFFER_SIZE/2)*CHUNK_SIZE)

ERR_RET qtree_encode(struct Transforms* transformations, struct image_data* src, struct encoder_params params,
                     u_int32_t threshold_param){

    struct image_data img;
    u_int32_t width=src->width;
    u_int32_t height=src->height;
    u_int32_t size=width*height;
    u_int32_t threshold=threshold_param;    //Temporary fixed value!

    if (width % 32 != 0 || height % 32 != 0)
    {
        return ERR_IMAGE_WRONG_SIZE;
    }

    if(params.use_ycbcr){
        if(src->color_mode!=COLOR_MODE_YCbCr){
            convert_from_RGB_to_YCbCr(src);
        }
        transformations->color_mode=COLOR_MODE_YCbCr;
    }else{
        transformations->color_mode=COLOR_MODE_RGB;
    }

    //We do our crazy stuff here
    /*
     * Make sense because you are only working with two channels
     * [0] -> Original channel data
     * [1] -> Downsampled channel data
     */
    // We allocate only one channel other one is just a pointer to original
    init_image_data(&img, width, height, 1);
    img.image_channels[1]=img.image_channels[0];
    transformations->channels=src->channels;

    unsigned total_chunks=(img.width/BUFFER_SIZE)*(img.height/BUFFER_SIZE);

    for (size_t channel=0; channel<src->channels; channel++){
        printf("Channel: %d ==================================\n", channel);
        img.image_channels[0]=src->image_channels[channel];
        down_sample(img.image_channels[0], width, 0,0, width/2, img.image_channels[1]);
        transformations->ch[channel].head=NULL;
        transformations->ch[channel].tail=NULL;
        transformations->ch[channel].elements=0;

        if (channel >= 1 && params.use_ycbcr)
            threshold *= 2;

        unsigned current_chunk=0;
        while(current_chunk<total_chunks){
            unsigned blocks_in_chunk=total_chunks-current_chunk;
            if(blocks_in_chunk>CHUNK_SIZE)
                blocks_in_chunk=CHUNK_SIZE;

            find_matches_for(&img, transformations->ch+channel,
                             current_chunk, blocks_in_chunk, threshold);

            current_chunk+=blocks_in_chunk;
        }

        if (channel >= 1 && params.use_ycbcr)
            threshold /= 2;
    }

    img.image_channels[0]=img.image_channels[1];
    clear_image_data(&img);

    return ERR_SUCCESS;
}

void print_cleared_blocks(bool* finished){
    int elements=0;

    for(int i=0;i<CHUNK_SIZE;++i){
        for(int j=0;j<MAX_RANGE_BLOCKS; ++j){
            printf("%d ",finished[elements++]);
        }
        printf("\n");
    }
    printf("\n");

}

static inline int get_block_index(int i, int j, int block_size){
    block_size/=2;
    i*=block_size;
    j*=block_size;

    int block_index=i%2;
    block_index+=(i/8)*64; i%=8;
    block_index+=(i/4)*16; i%=4;
    block_index+=(i/2)*4; i%=4;

    int block_offset=(j%2)*2;
    block_offset+=(j/4)*32; j%=4;
    block_offset+=(j/2)*8;

    block_index+=block_offset;

    return block_index;
}

ERR_RET find_matches_for(struct image_data* img, struct ifs_transformation_list* transformations,
                         u_int32_t from_range_block, u_int32_t block_rows, u_int32_t threshold){

//    printf("====== Starting new block from range block: %d\n", from_range_block);

    static pixel_value buffer[BUFFER_SIZE*BUFFER_SIZE];
    static u_int32_t range_avarages[MAX_RANGE_BLOCKS_PER_CHUNK];
    static double best_errors[MAX_RANGE_BLOCKS_PER_CHUNK];
    static struct ifs_transformation tmp_range_transformations[MAX_RANGE_BLOCKS_PER_CHUNK];
    static bool finished_blocks[MAX_RANGE_BLOCKS_PER_CHUNK];

    memset(finished_blocks, 0, sizeof(bool)*MAX_RANGE_BLOCKS_PER_CHUNK);

    int total_block_elements=block_rows*MAX_RANGE_BLOCKS;

    int block_columns=1;
    int blocks_remaining=block_rows;
    for(unsigned block_size=BUFFER_SIZE; block_size>=2 && blocks_remaining; block_size/=2){
//        printf("Reducing block size: %d\n",block_size);
        int block_coordinate=from_range_block*BUFFER_SIZE;
        int jump_size=(block_size/2)*(block_size/2);

        for(int j=0; j<block_columns; ++j){
            block_coordinate=from_range_block*BUFFER_SIZE;
            for(int i=0; i<block_rows; ++i){
                int block_index=get_block_index(i,j,block_size);

                if(!finished_blocks[block_index]){
                    int from_x=block_coordinate%img->width;
                    int from_y=(block_coordinate/img->width)*BUFFER_SIZE+j*block_size;

                    get_average_pixel(img->image_channels[0], img->width, from_x, from_y, block_size,
                            range_avarages+(block_index));

//                    printf("Avarage pixel for %d %d is %d, block index: %d, i,j: (%d, %d)\n",from_x, from_y, range_avarages[block_index], block_index,i,j);

                    best_errors[block_index]=1e9;
                }

                block_coordinate+=block_size;
            }
        }

        for(size_t y=0; y<img->height; y+=block_size*2)
        {
            for (size_t x=0; x<img->width; x+=block_size*2)
            {
                for(int transformation_type=0; transformation_type<SYM_MAX; ++transformation_type)
                {
                    struct ifs_transformation ifs={
                        .from_x=x,
                        .from_y=y,
                        .to_x=0,
                        .to_y=0,
                        .size=block_size,
                        .transformation_type=transformation_type,
                        .scale=1.0,
                        .offset=0
                    };

                    ifs_transformation_execute(&ifs, img->image_channels[1], img->width/2, buffer, block_size, true);
                    u_int32_t domain_avg;
                    get_average_pixel(buffer, block_size, 0, 0, block_size, &domain_avg);

                    block_coordinate=from_range_block*BUFFER_SIZE;

                    for(int j=0; j<block_columns; ++j){
                        block_coordinate=from_range_block*BUFFER_SIZE;
                        for(int i=0; i<block_rows; ++i){
                            int block_index=get_block_index(i,j,block_size);

                            if(!finished_blocks[block_index]){
                                int from_x=block_coordinate%img->width;
                                int from_y=(block_coordinate/img->width)*BUFFER_SIZE+j*block_size;

                                double scale_factor;
                                get_scale_factor(img->image_channels[0], img->width, from_x, from_y, domain_avg,
                                        buffer, block_size, 0 ,0, range_avarages[block_index], block_size, &scale_factor);
                                int offset = (int)(range_avarages[block_index] - scale_factor * (double)domain_avg);

                                double error;
                                get_error(buffer, block_size, 0,0, domain_avg, img->image_channels[0],
                                        img->width, from_x, from_y, range_avarages[block_index],
                                        block_size, scale_factor, &error);

//                                printf("Comparing rb: [%d, %d] with db: [%d, %d] block size: %d error: %g, range avg %d block index: %d\n",from_x, from_y, x, y, block_size, error, range_avarages[block_index], block_index);

                                if(error<best_errors[block_index]){
                                    tmp_range_transformations[block_index].from_x=x;
                                    tmp_range_transformations[block_index].from_y=y;
                                    tmp_range_transformations[block_index].to_x=from_x;
                                    tmp_range_transformations[block_index].to_y=from_y;
                                    tmp_range_transformations[block_index].transformation_type=transformation_type;
                                    tmp_range_transformations[block_index].scale=scale_factor;
                                    tmp_range_transformations[block_index].offset=offset;
                                    tmp_range_transformations[block_index].size=ifs.size;

                                    best_errors[block_index]=error;

                                    if(error<threshold){
//                                        printf("\tFound good transformation rb: [%d, %d] db: [%d, %d] block size: %d error: %g\n", from_x, from_y, x, y, block_size, error);

                                        ifs_trans_push_back(transformations, tmp_range_transformations+block_index);
                                        for(int k=0;k<jump_size;++k){
                                            finished_blocks[block_index+k]=true;
                                        }

//                                        print_cleared_blocks(finished_blocks);
                                        blocks_remaining--;
                                    }
                                }
                            }

                            block_coordinate+=block_size;
                        }
                    }

                    if(!transformation_type)
                        break;
                }
            }
        }

        block_columns*=2;
        block_rows*=2;
        blocks_remaining*=4;
    }
    for(int i=0;i<total_block_elements;++i){
        if(!finished_blocks[i]){
            ifs_trans_push_back(transformations, tmp_range_transformations+i);
        }
    }

    return ERR_SUCCESS;
}

ERR_RET print_best_transformation(struct ifs_transformation best_ifs, double best_err) {
    printf("to=(%d, %d)\n", best_ifs.to_x, best_ifs.to_y);
    printf("from=(%d, %d)\n", best_ifs.from_x, best_ifs.from_y);
    printf("best error=%lf\n", best_err);
    printf("best symmetry=%d\n", best_ifs.transformation_type);
    printf("best offset=%d\n", best_ifs.offset);
    printf("best scale=%lf\n", best_ifs.scale);
    return ERR_SUCCESS;
}
