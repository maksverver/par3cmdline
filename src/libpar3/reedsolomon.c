#include "libpar3.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "galois.h"
#include "hash.h"
#include "reedsolomon.h"


// Create all recovery blocks from one input block.
void rs_create_one_all(PAR3_CTX *par3_ctx, int x_index)
{
	void *gf_table;
	uint8_t *work_buf, *buf_p;
	uint8_t gf_size;
	int first_num, element;
	int y_index, y_R;
	int recovery_block_count;
	size_t region_size;

	recovery_block_count = (int)(par3_ctx->recovery_block_count);
	first_num = (int)(par3_ctx->first_recovery_block);
	gf_size = par3_ctx->gf_size;
	gf_table = par3_ctx->galois_table;
	work_buf = par3_ctx->work_buf;
	buf_p = par3_ctx->block_data;

	// For every recovery block
	region_size = (par3_ctx->block_size + 4 + 3) & ~3;
	for (y_index = 0; y_index < recovery_block_count; y_index++){
		// Calculate Matrix elements
		if (par3_ctx->gf_size == 2){	// 16-bit Galois Field
			y_R = 65535 - (y_index + first_num);
			element = gf16_reciprocal(gf_table, x_index ^ y_R);	// inv( x_index ^ y_R )

			// If x_index == 0, just put values.
			// If x_index > 0, add values on previous values.
			gf16_region_multiply(gf_table, work_buf, element, region_size, buf_p, x_index);

		} else {	// 8-bit Galois Field
			y_R = 255 - (y_index + first_num);
			element = gf8_reciprocal(gf_table, x_index ^ y_R);	// inv( x_index ^ y_R )

			// If x_index == 0, just put values.
			// If x_index > 0, add values on previous values.
			gf8_region_multiply(gf_table, work_buf, element, region_size, buf_p, x_index);
		}
		//printf("x = %d, R = %d, y_R = %d, element = %d\n", x_index, y_index + first_num, y_R, element);

		buf_p += region_size;
	}
}

// Create all recovery blocks from all input blocks.
void rs_create_all(PAR3_CTX *par3_ctx, size_t region_size, uint64_t progress_total, uint64_t progress_step)
{
	void *gf_table;
	uint8_t *block_data, *input_p, *recv_p;
	uint8_t gf_size;
	int first_num, element;
	int x_index, y_index, y_R;
	int block_count, recovery_block_count;
	int progress_old, progress_now;
	time_t time_old, time_now;

	block_count = (int)(par3_ctx->block_count);
	recovery_block_count = (int)(par3_ctx->recovery_block_count);
	first_num = (int)(par3_ctx->first_recovery_block);
	gf_size = par3_ctx->gf_size;
	gf_table = par3_ctx->galois_table;
	block_data = par3_ctx->block_data;
	recv_p = block_data + region_size * block_count;

	if ( (par3_ctx->noise_level >= 0) && (par3_ctx->noise_level <= 1) ){
		progress_old = 0;
		time_old = time(NULL);
	}

	// For every recovery block
	for (y_index = 0; y_index < recovery_block_count; y_index++){
		input_p = block_data;

		// For every input block
		for (x_index = 0; x_index < block_count; x_index++){

			// Calculate Matrix elements
			if (par3_ctx->gf_size == 2){	// 16-bit Galois Field
				y_R = 65535 - (y_index + first_num);
				element = gf16_reciprocal(gf_table, x_index ^ y_R);	// inv( x_index ^ y_R )

				// If x_index == 0, just put values.
				// If x_index > 0, add values on previous values.
				gf16_region_multiply(gf_table, input_p, element, region_size, recv_p, x_index);

			} else {	// 8-bit Galois Field
				y_R = 255 - (y_index + first_num);
				element = gf8_reciprocal(gf_table, x_index ^ y_R);	// inv( x_index ^ y_R )

				// If x_index == 0, just put values.
				// If x_index > 0, add values on previous values.
				gf8_region_multiply(gf_table, input_p, element, region_size, recv_p, x_index);
			}
			//printf("x = %d, R = %d, y_R = %d, element = %d\n", x_index, y_index + first_num, y_R, element);

			input_p += region_size;
		}

		// Print progress percent
		if ( (par3_ctx->noise_level >= 0) && (par3_ctx->noise_level <= 1) ){
			progress_step += block_count;
			time_now = time(NULL);
			if (time_now != time_old){
				time_old = time_now;
				progress_now = (int)((progress_step * 1000) / progress_total);
				if (progress_now != progress_old){
					progress_old = progress_now;
					printf("%d.%d%%\r", progress_now / 10, progress_now % 10);	// 0.0% ~ 100.0%
				}
			}
		}

		recv_p += region_size;
	}
}


// Construct matrix for Cauchy Reed-Solomon, and solve linear equation.
int rs_compute_matrix(PAR3_CTX *par3_ctx, uint64_t lost_count)
{
	int ret;
	size_t alloc_size, region_size;

	// Only when it uses Reed-Solomon Erasure Codes.
	if ((par3_ctx->ecc_method & 1) == 0)
		return RET_LOGIC_ERROR;

	if (par3_ctx->gf_size == 2){	// 16-bit Galois Field
		par3_ctx->galois_table = gf16_create_table(par3_ctx->galois_poly);

	} else if (par3_ctx->gf_size == 1){	// 8-bit Galois Field
		par3_ctx->galois_table = gf8_create_table(par3_ctx->galois_poly);

	} else {
		printf("Galois Field (0x%X) isn't supported.\n", par3_ctx->galois_poly);
		return RET_LOGIC_ERROR;
	}
	if (par3_ctx->galois_table == NULL){
		printf("Failed to create tables for Galois Field (0x%X)\n", par3_ctx->galois_poly);
		return RET_MEMORY_ERROR;
	}

	// Make matrix
	if (par3_ctx->gf_size == 2){	// 16-bit Reed-Solomon Codes
		// Either functions should work.
		// As blocks are more, Gaussian elimination become too slow.
		//ret = rs16_gaussian_elimination(par3_ctx, (int)lost_count);
		ret = rs16_invert_matrix_cauchy(par3_ctx, (int)lost_count);
		if (ret != 0)
			return ret;

	} else if (par3_ctx->gf_size == 1){	// 8-bit Reed-Solomon Codes
		// Either functions should work.
		// Gaussian elimination is enough fast for a few blocks.
		ret = rs8_gaussian_elimination(par3_ctx, (int)lost_count);
		//ret = rs8_invert_matrix_cauchy(par3_ctx, (int)lost_count);
		if (ret != 0)
			return ret;
	}

	// Set memory alignment of block data to be 4.
	// Increase at least 1 byte as checksum.
	region_size = (par3_ctx->block_size + 4 + 3) & ~3;

	// Limited memory usage
	alloc_size = region_size * lost_count;
	if ( (par3_ctx->memory_limit > 0) && (alloc_size > par3_ctx->memory_limit) )
		return 0;

	// Allocate memory to keep lost blocks
	par3_ctx->block_data = malloc(alloc_size);
	//par3_ctx->block_data = NULL;	// For testing another method
	if (par3_ctx->block_data != NULL){
		par3_ctx->ecc_method |= 0x8000;	// Keep all lost blocks on memory
		if (par3_ctx->noise_level >= 2){
			printf("\nAligned size of block data = %zu\n", region_size);
			printf("Keep all lost blocks on memory (%zu * %"PRIu64" = %zu)\n", region_size, lost_count, alloc_size);
		}
	}

	return 0;
}

// Recover all lost input blocks from one block.
void rs_recover_one_all(PAR3_CTX *par3_ctx, int x_index, int lost_count)
{
	void *gf_table, *matrix;
	uint8_t *work_buf, *buf_p;
	uint8_t gf_size;
	int y_index, factor;
	int block_count;
	size_t region_size;

	block_count = (int)(par3_ctx->block_count);
	gf_size = par3_ctx->gf_size;
	gf_table = par3_ctx->galois_table;
	matrix = par3_ctx->matrix;
	work_buf = par3_ctx->work_buf;
	buf_p = par3_ctx->block_data;

	// For every lost block
	region_size = (par3_ctx->block_size + 4 + 3) & ~3;
	for (y_index = 0; y_index < lost_count; y_index++){
		if (gf_size == 2){
			factor = ((uint16_t *)matrix)[ block_count * y_index + x_index ];
			gf16_region_multiply(gf_table, work_buf, factor, region_size, buf_p, 1);
		} else {
			factor = ((uint8_t *)matrix)[ block_count * y_index + x_index ];
			gf8_region_multiply(gf_table, work_buf, factor, region_size, buf_p, 1);
		}
		//printf("%d-th lost block += input block[%d] * %2x\n", y_index, x_index, factor);

		buf_p += region_size;
	}
}

// Recover all lost input blocks from all blocks.
void rs_recover_all(PAR3_CTX *par3_ctx, size_t region_size, int lost_count, uint64_t progress_total, uint64_t progress_step)
{
	void *gf_table, *matrix;
	uint8_t *block_data, *buf_p, *input_p, *recv_p;
	uint8_t gf_size;
	int *lost_id;
	int x_index, y_index, lost_index, factor;
	int block_count;
	int progress_old, progress_now;
	time_t time_old, time_now;

	block_count = (int)(par3_ctx->block_count);
	gf_size = par3_ctx->gf_size;
	gf_table = par3_ctx->galois_table;
	matrix = par3_ctx->matrix;
	lost_id = par3_ctx->recv_id_list + lost_count;
	block_data = par3_ctx->block_data;
	recv_p = block_data + region_size * block_count;

	if ( (par3_ctx->noise_level >= 0) && (par3_ctx->noise_level <= 1) ){
		progress_old = 0;
		time_old = time(NULL);
	}

	// For every lost block
	for (y_index = 0; y_index < lost_count; y_index++){
		buf_p = block_data + region_size * lost_id[y_index];
		input_p = block_data;

		// For every available input block
		lost_index = 0;
		for (x_index = 0; x_index < block_count; x_index++){
			if (x_index == lost_id[lost_index]){
				lost_index++;
				input_p += region_size;
				continue;
			}

			if (gf_size == 2){
				factor = ((uint16_t *)matrix)[ block_count * y_index + x_index ];
				gf16_region_multiply(gf_table, input_p, factor, region_size, buf_p, 1);
			} else {
				factor = ((uint8_t *)matrix)[ block_count * y_index + x_index ];
				gf8_region_multiply(gf_table, input_p, factor, region_size, buf_p, 1);
			}

			input_p += region_size;
		}

		// For every using recovery block
		for (lost_index = 0; lost_index < lost_count; lost_index++){
			x_index = lost_id[lost_index];

			if (gf_size == 2){
				factor = ((uint16_t *)matrix)[ block_count * y_index + x_index ];
				gf16_region_multiply(gf_table, input_p, factor, region_size, buf_p, 1);
			} else {
				factor = ((uint8_t *)matrix)[ block_count * y_index + x_index ];
				gf8_region_multiply(gf_table, input_p, factor, region_size, buf_p, 1);
			}

			input_p += region_size;
		}

		// Print progress percent
		if ( (par3_ctx->noise_level >= 0) && (par3_ctx->noise_level <= 1) ){
			progress_step += block_count;
			time_now = time(NULL);
			if (time_now != time_old){
				time_old = time_now;
				progress_now = (int)((progress_step * 1000) / progress_total);
				if (progress_now != progress_old){
					progress_old = progress_now;
					printf("%d.%d%%\r", progress_now / 10, progress_now % 10);	// 0.0% ~ 100.0%
				}
			}
		}
	}
}

