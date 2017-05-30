#ifndef ENCODERMACROS_H
#define ENCODERMACROS_H

#define ASSIGN_IFS_VALUES(DST, FROM_X, FROM_Y, TO_X, TO_Y, TR_TYPE, SCALE, OFFSET, SIZE)\
    DST.from_x=FROM_X;\
    DST.from_y=FROM_Y;\
    DST.to_x=TO_X;\
    DST.to_y=TO_Y;\
    DST.transformation_type=TR_TYPE;\
    DST.scale=SCALE;\
    DST.offset=OFFSET;\
    DST.size=SIZE;

#define CALCULATE_ERR(SC, DM_X, DM_Y, DM_AVG, RB_X, RB_Y, RB_AVG, OFFSET, ERROR)\
    INCREMENT_FLOP_COUNT(2, 0, 0, 0)\
    SC=get_scale_factor(img->image_channels[0], img->width, DM_X, DM_Y, DM_AVG,\
            buffer, block_size, RB_X, RB_Y, RB_AVG, half_block_size);\
    OFFSET = (int)(RB_AVG - SC * (double)DM_AVG);\
\
    ERROR=get_error(buffer, block_size, RB_X, RB_Y, DM_AVG, img->image_channels[0],\
            img->width, DM_X, DM_Y, RB_AVG,\
            half_block_size, SC);


#define UPDATE_MIN_ERROR(DM_X, DM_Y)\
if(tmp_error1<error_1){\
    ASSIGN_IFS_VALUES(best_ifs_1, DM_X, DM_Y, rb_x0, rb_y0, transformation_type, scale_factor1, offset1, half_block_size);\
    error_1=tmp_error1;\
}\
if(tmp_error2<error_2){\
    ASSIGN_IFS_VALUES(best_ifs_2, DM_X, DM_Y, rb_x1, rb_y0, transformation_type, scale_factor2, offset2, half_block_size);\
    error_2=tmp_error2;\
}\
if(tmp_error3<error_3){\
    ASSIGN_IFS_VALUES(best_ifs_3, DM_X, DM_Y, rb_x0, rb_y1, transformation_type, scale_factor3, offset3, half_block_size);\
    error_3=tmp_error3;\
}\
if(tmp_error4<error_4){\
    ASSIGN_IFS_VALUES(best_ifs_4, DM_X, DM_Y, rb_x1, rb_y1, transformation_type, scale_factor4, offset4, half_block_size);\
    error_4=tmp_error4;\
}\

#define CALCULATE_MIN(X_1, Y_1, AVG_1, X_2, Y_2, AVG_2, X_3, Y_3, AVG_3, X_4, Y_4, AVG_4)\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_1, 0, 0, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_1, 0, 0, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_1, 0, 0, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_1, 0, 0, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_1,Y_1);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_2, half_block_size, 0, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_2, half_block_size, 0, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_2, half_block_size, 0, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_2, half_block_size, 0, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_2,Y_2);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_3, 0, half_block_size, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_3, 0, half_block_size, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_3, 0, half_block_size, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_3, 0, half_block_size, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_3,Y_3);\
\
    CALCULATE_ERR(scale_factor1, rb_x0, rb_y0, AVG_4, half_block_size, half_block_size, avarage_pix_1, offset1, tmp_error1);\
    CALCULATE_ERR(scale_factor2, rb_x1, rb_y0, AVG_4, half_block_size, half_block_size, avarage_pix_2, offset2, tmp_error2);\
    CALCULATE_ERR(scale_factor3, rb_x0, rb_y1, AVG_4, half_block_size, half_block_size, avarage_pix_3, offset3, tmp_error3);\
    CALCULATE_ERR(scale_factor4, rb_x1, rb_y1, AVG_4, half_block_size, half_block_size, avarage_pix_4, offset4, tmp_error4);\
\
    UPDATE_MIN_ERROR(X_4,Y_4);\


#endif // ENCODERMACROS_H
