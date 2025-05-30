#include "libpar3.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// 0 = no packet yet, 1 = the packet exists already
int check_packet_exist(uint8_t *buf, size_t buf_size, uint8_t *packet, uint64_t packet_size)
{
	size_t offset;
	uint64_t this_size;

	offset = 0;
	while (offset + packet_size <= buf_size){
		// compare packet size
		memcpy(&this_size, buf + offset + 24, 8);
		if (this_size == packet_size){
			// compare checksums
			if (memcmp(buf + offset + 8, packet + 8, 16) == 0){
				return 1;
			}
		}

		offset += this_size;
	}

	return 0;
}

// It allocates memory for each packet type, and stores the packet.
// -2 = unknown type, -1 = the packet exists already, 0 = added, 1~ = error
int add_found_packet(PAR3_CTX *par3_ctx, uint8_t *packet)
{
	uint8_t *packet_type, *tmp_p;
	uint64_t packet_size;

	// read packet size
	memcpy(&packet_size, packet + 24, 8);

	// allocate memory for the packet type
	packet_type = packet + 40;
	if (memcmp(packet_type, "PAR CRE\0", 8) == 0){	// Creator Packet
		if (par3_ctx->creator_packet == NULL){
			par3_ctx->creator_packet = malloc(packet_size);
			if (par3_ctx->creator_packet == NULL){
				perror("Failed to allocate memory for Creator Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->creator_packet, packet, packet_size);
			par3_ctx->creator_packet_size = packet_size;
			par3_ctx->creator_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->creator_packet, par3_ctx->creator_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->creator_packet, par3_ctx->creator_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Creator Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->creator_packet = tmp_p;
			memcpy(par3_ctx->creator_packet + par3_ctx->creator_packet_size, packet, packet_size);
			par3_ctx->creator_packet_size += packet_size;
			par3_ctx->creator_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR COM\0", 8) == 0){	// Comment Packet
		if (par3_ctx->comment_packet == NULL){
			par3_ctx->comment_packet = malloc(packet_size);
			if (par3_ctx->comment_packet == NULL){
				perror("Failed to allocate memory for Comment Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->comment_packet, packet, packet_size);
			par3_ctx->comment_packet_size = packet_size;
			par3_ctx->comment_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->comment_packet, par3_ctx->comment_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->comment_packet, par3_ctx->comment_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Comment Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->comment_packet = tmp_p;
			memcpy(par3_ctx->comment_packet + par3_ctx->comment_packet_size, packet, packet_size);
			par3_ctx->comment_packet_size += packet_size;
			par3_ctx->comment_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR STA\0", 8) == 0){	// Start Packet
		if (par3_ctx->start_packet == NULL){
			par3_ctx->start_packet = malloc(packet_size);
			if (par3_ctx->start_packet == NULL){
				perror("Failed to allocate memory for Start Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->start_packet, packet, packet_size);
			par3_ctx->start_packet_size = packet_size;
			par3_ctx->start_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->start_packet, par3_ctx->start_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->start_packet, par3_ctx->start_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Start Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->start_packet = tmp_p;
			memcpy(par3_ctx->start_packet + par3_ctx->start_packet_size, packet, packet_size);
			par3_ctx->start_packet_size += packet_size;
			par3_ctx->start_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR FIL\0", 8) == 0){	// File Packet
		if (par3_ctx->file_packet == NULL){
			par3_ctx->file_packet = malloc(packet_size);
			if (par3_ctx->file_packet == NULL){
				perror("Failed to allocate memory for File Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->file_packet, packet, packet_size);
			par3_ctx->file_packet_size = packet_size;
			par3_ctx->file_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->file_packet, par3_ctx->file_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->file_packet, par3_ctx->file_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for File Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->file_packet = tmp_p;
			memcpy(par3_ctx->file_packet + par3_ctx->file_packet_size, packet, packet_size);
			par3_ctx->file_packet_size += packet_size;
			par3_ctx->file_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR DIR\0", 8) == 0){	// Directory Packet
		if (par3_ctx->dir_packet == NULL){
			par3_ctx->dir_packet = malloc(packet_size);
			if (par3_ctx->dir_packet == NULL){
				perror("Failed to allocate memory for Directory Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->dir_packet, packet, packet_size);
			par3_ctx->dir_packet_size = packet_size;
			par3_ctx->dir_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->dir_packet, par3_ctx->dir_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->dir_packet, par3_ctx->dir_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Directory Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->dir_packet = tmp_p;
			memcpy(par3_ctx->dir_packet + par3_ctx->dir_packet_size, packet, packet_size);
			par3_ctx->dir_packet_size += packet_size;
			par3_ctx->dir_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR ROO\0", 8) == 0){	// Root Packet
		if (par3_ctx->root_packet == NULL){
			par3_ctx->root_packet = malloc(packet_size);
			if (par3_ctx->root_packet == NULL){
				perror("Failed to allocate memory for Root Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->root_packet, packet, packet_size);
			par3_ctx->root_packet_size = packet_size;
			par3_ctx->root_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->root_packet, par3_ctx->root_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->root_packet, par3_ctx->root_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Root Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->root_packet = tmp_p;
			memcpy(par3_ctx->root_packet + par3_ctx->root_packet_size, packet, packet_size);
			par3_ctx->root_packet_size += packet_size;
			par3_ctx->root_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR EXT\0", 8) == 0){	// External Data Packet
		if (par3_ctx->ext_data_packet == NULL){
			par3_ctx->ext_data_packet = malloc(packet_size);
			if (par3_ctx->ext_data_packet == NULL){
				perror("Failed to allocate memory for External Data Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->ext_data_packet, packet, packet_size);
			par3_ctx->ext_data_packet_size = packet_size;
			par3_ctx->ext_data_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->ext_data_packet, par3_ctx->ext_data_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->ext_data_packet, par3_ctx->ext_data_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for External Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->ext_data_packet = tmp_p;
			memcpy(par3_ctx->ext_data_packet + par3_ctx->ext_data_packet_size, packet, packet_size);
			par3_ctx->ext_data_packet_size += packet_size;
			par3_ctx->ext_data_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR CAU\0", 8) == 0){	// Cauchy Matrix Packet
		if (par3_ctx->matrix_packet == NULL){
			par3_ctx->matrix_packet = malloc(packet_size);
			if (par3_ctx->matrix_packet == NULL){
				perror("Failed to allocate memory for Matrix Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->matrix_packet, packet, packet_size);
			par3_ctx->matrix_packet_size = packet_size;
			par3_ctx->matrix_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->matrix_packet, par3_ctx->matrix_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->matrix_packet, par3_ctx->matrix_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Matrix Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->matrix_packet = tmp_p;
			memcpy(par3_ctx->matrix_packet + par3_ctx->matrix_packet_size, packet, packet_size);
			par3_ctx->matrix_packet_size += packet_size;
			par3_ctx->matrix_packet_count++;
		}

	} else if (memcmp(packet_type, "PAR FFT\0", 8) == 0){	// FFT Matrix Packet
		if (par3_ctx->matrix_packet == NULL){
			par3_ctx->matrix_packet = malloc(packet_size);
			if (par3_ctx->matrix_packet == NULL){
				perror("Failed to allocate memory for Matrix Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->matrix_packet, packet, packet_size);
			par3_ctx->matrix_packet_size = packet_size;
			par3_ctx->matrix_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->matrix_packet, par3_ctx->matrix_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->matrix_packet, par3_ctx->matrix_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Matrix Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->matrix_packet = tmp_p;
			memcpy(par3_ctx->matrix_packet + par3_ctx->matrix_packet_size, packet, packet_size);
			par3_ctx->matrix_packet_size += packet_size;
			par3_ctx->matrix_packet_count++;
		}

	// UNIX Permissions Packet or FAT Permissions Packet
	} else if ( (memcmp(packet_type, "PAR UNX\0", 8) == 0)
				|| (memcmp(packet_type, "PAR FAT\0", 8) == 0) ){
		if (par3_ctx->file_system_packet == NULL){
			par3_ctx->file_system_packet = malloc(packet_size);
			if (par3_ctx->file_system_packet == NULL){
				perror("Failed to allocate memory for File System Packet");
				return RET_MEMORY_ERROR;
			}
			memcpy(par3_ctx->file_system_packet, packet, packet_size);
			par3_ctx->file_system_packet_size = packet_size;
			par3_ctx->file_system_packet_count = 1;
		} else if (check_packet_exist(par3_ctx->file_system_packet, par3_ctx->file_system_packet_size, packet, packet_size) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			tmp_p = realloc(par3_ctx->file_system_packet, par3_ctx->file_system_packet_size + packet_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for File System Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->file_system_packet = tmp_p;
			memcpy(par3_ctx->file_system_packet + par3_ctx->file_system_packet_size, packet, packet_size);
			par3_ctx->file_system_packet_size += packet_size;
			par3_ctx->file_system_packet_count++;
		}

	} else {	// Unknown packet type
		return -2;
/*
make context for large packets ?
Data Packet and Recovery Data Packet will be too large to store on memory.
*/
	}

	return 0;
}

// 0 = no packet yet, 1 = the packet exists already
static int check_item_exist(PAR3_PKT_CTX *list, uint64_t item_count, uint64_t id, uint64_t index, uint8_t *cmp_buf)
{
	while (item_count != 0){
		// compare InputSetID and index
		if ( (list->id == id) && (list->index == index) ){
			if (cmp_buf == NULL){	// Data Packet
				return 1;
			} else {	// Recovery Data Packet
				// comprare other values
				if ( (memcmp(list->root, cmp_buf, 16) == 0) && (memcmp(list->matrix, cmp_buf + 16, 16) == 0) ){
					return 1;
				}
			}
		}

		list++;
		item_count--;
	}

	return 0;
}

// It allocates memory for each packet type, and lists the packet.
// -2 = unknown type, -1 = the packet exists already, 0 = added, 1~ = error
int list_found_packet(PAR3_CTX *par3_ctx, uint8_t *packet, char *filename, int64_t offset)
{
	uint8_t *packet_type, cmp_buf[32];
	uint64_t set_id, index, count;
	PAR3_PKT_CTX *list;

	// allocate memory for the packet type
	packet_type = packet + 40;
	if (memcmp(packet_type, "PAR DAT\0", 8) == 0){	// Data Packet
		memcpy(&set_id, packet + 32, 8);	// InputSetID
		memcpy(&index, packet + 48, 8);		// Index of input block
		if (par3_ctx->data_packet_list == NULL){
			list = malloc(sizeof(PAR3_PKT_CTX));
			if (list == NULL){
				perror("Failed to allocate memory for Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->data_packet_list = list;
			list[0].id = set_id;
			memset(list[0].root, 0, 16);	// Zero fill unused values
			memset(list[0].matrix, 0, 16);
			list[0].index = index;
			list[0].name = filename;
			list[0].offset = offset;
			par3_ctx->data_packet_count = 1;
		} else if (check_item_exist(par3_ctx->data_packet_list, par3_ctx->data_packet_count, set_id, index, NULL) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			count = par3_ctx->data_packet_count;
			list = realloc(par3_ctx->data_packet_list, sizeof(PAR3_PKT_CTX) * (count + 1));
			if (list == NULL){
				perror("Failed to re-allocate memory for Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->data_packet_list = list;
			list[count].id = set_id;
			memset(list[count].root, 0, 16);	// Zero fill unused values
			memset(list[count].matrix, 0, 16);
			list[count].index = index;
			list[count].name = filename;
			list[count].offset = offset;
			par3_ctx->data_packet_count += 1;
		}

	} else if (memcmp(packet_type, "PAR REC\0", 8) == 0){	// Recovery Data Packet
		memcpy(&set_id, packet + 32, 8);		// InputSetID
		memcpy(cmp_buf, packet + 48, 16);		// checksum from Root packet
		memcpy(cmp_buf + 16, packet + 64, 16);	// checksum from Matrix packet
		memcpy(&index, packet + 80, 8);			// Index of recovery block
		if (par3_ctx->recv_packet_list == NULL){
			list = malloc(sizeof(PAR3_PKT_CTX));
			if (list == NULL){
				perror("Failed to allocate memory for Recovery Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->recv_packet_list = list;
			list[0].id = set_id;
			memcpy(list[0].root, cmp_buf, 16);
			memcpy(list[0].matrix, cmp_buf + 16, 16);
			list[0].index = index;
			list[0].name = filename;
			list[0].offset = offset;
			par3_ctx->recv_packet_count = 1;
		} else if (check_item_exist(par3_ctx->recv_packet_list, par3_ctx->recv_packet_count, set_id, index, cmp_buf) == 1){
			// If there is the packet already, just exit.
			return -1;
		} else {
			// Add this packet after other packets.
			count = par3_ctx->recv_packet_count;
			list = realloc(par3_ctx->recv_packet_list, sizeof(PAR3_PKT_CTX) * (count + 1));
			if (list == NULL){
				perror("Failed to re-allocate memory for Recovery Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->recv_packet_list = list;
			list[count].id = set_id;
			memcpy(list[count].root, cmp_buf, 16);
			memcpy(list[count].matrix, cmp_buf + 16, 16);
			list[count].index = index;
			list[count].name = filename;
			list[count].offset = offset;
			par3_ctx->recv_packet_count += 1;
		}

	} else {
		return -2;
	}

	return 0;
}

// Delete packets and move former, return new size.
static size_t adjust_packet_buf(uint8_t *buf, size_t buf_size, uint64_t *id_list, int id_count, uint32_t *new_count)
{
	int i;
	uint32_t count;
	size_t offset;
	uint64_t packet_size, this_id;

	count = 0;
	offset = 0;
	while (offset < buf_size){
		// read packet size
		memcpy(&packet_size, buf + offset + 24, 8);

		// check SetID
		memcpy(&this_id, buf + offset + 32, 8);
		for (i = 0; i < id_count; i++){
			if (id_list[i] == this_id)
				break;
		}
		if (i == id_count){	// When packet didn't match, delete it.
			memmove(buf + offset, buf + offset + packet_size, buf_size - offset - packet_size);
			buf_size -= packet_size;

		} else {	// goto next packet
			count++;
			offset += packet_size;
		}
	}
	*new_count = count;

	return buf_size;
}

// Delete items and move former, return new count.
static uint64_t adjust_packet_list(PAR3_PKT_CTX *list, uint64_t item_count, uint64_t *id_list, int id_count, uint8_t *root)
{
	int i;
	uint64_t this_id, item_index;

	item_index = 0;
	while (item_index < item_count){
		// check SetID
		this_id = list[item_index].id;
		for (i = 0; i < id_count; i++){
			if (id_list[i] == this_id){
				if (root == NULL){	// Data Packet
					break;
				} else {	// Recovery Data Packet
					// Use the recovery data after confirming checksum from Root Packet
					if (memcmp(list[item_index].root, root, 16) == 0){
						break;
					}
				}
			}
		}
		if (i == id_count){	// When packet didn't match, delete it.
			memmove(list + item_index, list + item_index + 1, sizeof(PAR3_PKT_CTX) * (item_count - item_index - 1));
			item_count--;

		} else {	// goto next packet
			item_index++;
		}
	}

	return item_index;
}

// Remove useless packets
static int remove_other_packet(PAR3_CTX *par3_ctx, uint64_t *id_list, int id_count)
{
	uint8_t *tmp_p;
	uint32_t new_count;
	size_t new_size;
	uint64_t item_count;
	PAR3_PKT_CTX *list;

	if (par3_ctx->creator_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->creator_packet, par3_ctx->creator_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->creator_packet);
			par3_ctx->creator_packet = NULL;
			par3_ctx->creator_packet_size = 0;
			par3_ctx->creator_packet_count = 0;
		} else if (new_size < par3_ctx->creator_packet_size){
			tmp_p = realloc(par3_ctx->creator_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Creator Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->creator_packet = tmp_p;
			par3_ctx->creator_packet_size = new_size;
			par3_ctx->creator_packet_count = new_count;
		}
	}
	if (par3_ctx->comment_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->comment_packet, par3_ctx->comment_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->comment_packet);
			par3_ctx->comment_packet = NULL;
			par3_ctx->comment_packet_size = 0;
			par3_ctx->comment_packet_count = 0;
		} else if (new_size < par3_ctx->comment_packet_size){
			tmp_p = realloc(par3_ctx->comment_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Comment Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->comment_packet = tmp_p;
			par3_ctx->comment_packet_size = new_size;
			par3_ctx->comment_packet_count = new_count;
		}
	}
	if (par3_ctx->start_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->start_packet, par3_ctx->start_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->start_packet);
			par3_ctx->start_packet = NULL;
			par3_ctx->start_packet_size = 0;
			par3_ctx->start_packet_count = 0;
		} else if (new_size < par3_ctx->start_packet_size){
			tmp_p = realloc(par3_ctx->start_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Start Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->start_packet = tmp_p;
			par3_ctx->start_packet_size = new_size;
			par3_ctx->start_packet_count = new_count;
		}
	}
	if (par3_ctx->matrix_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->matrix_packet, par3_ctx->matrix_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->matrix_packet);
			par3_ctx->matrix_packet = NULL;
			par3_ctx->matrix_packet_size = 0;
			par3_ctx->matrix_packet_count = 0;
		} else if (new_size < par3_ctx->matrix_packet_size){
			tmp_p = realloc(par3_ctx->matrix_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Matrix Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->matrix_packet = tmp_p;
			par3_ctx->matrix_packet_size = new_size;
			par3_ctx->matrix_packet_count = new_count;
		}
	}
	if (par3_ctx->file_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->file_packet, par3_ctx->file_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->file_packet);
			par3_ctx->file_packet = NULL;
			par3_ctx->file_packet_size = 0;
			par3_ctx->file_packet_count = 0;
		} else if (new_size < par3_ctx->file_packet_size){
			tmp_p = realloc(par3_ctx->file_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for File Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->file_packet = tmp_p;
			par3_ctx->file_packet_size = new_size;
			par3_ctx->file_packet_count = new_count;
		}
	}
	if (par3_ctx->dir_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->dir_packet, par3_ctx->dir_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->dir_packet);
			par3_ctx->dir_packet = NULL;
			par3_ctx->dir_packet_size = 0;
			par3_ctx->dir_packet_count = 0;
		} else if (new_size < par3_ctx->dir_packet_size){
			tmp_p = realloc(par3_ctx->dir_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Directory Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->dir_packet = tmp_p;
			par3_ctx->dir_packet_size = new_size;
			par3_ctx->dir_packet_count = new_count;
		}
	}
	if (par3_ctx->root_packet_size > 0){
		// Root Packet is only the last descendant SetID.
		new_size = adjust_packet_buf(par3_ctx->root_packet, par3_ctx->root_packet_size, (uint64_t *)(par3_ctx->set_id), 1, &new_count);
		if (new_size == 0){
			free(par3_ctx->root_packet);
			par3_ctx->root_packet = NULL;
			par3_ctx->root_packet_size = 0;
			par3_ctx->root_packet_count = 0;
		} else if (new_size < par3_ctx->root_packet_size){
			tmp_p = realloc(par3_ctx->root_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for Root Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->root_packet = tmp_p;
			par3_ctx->root_packet_size = new_size;
			par3_ctx->root_packet_count = new_count;
		}
	}
	if (par3_ctx->file_system_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->file_system_packet, par3_ctx->file_system_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->file_system_packet);
			par3_ctx->file_system_packet = NULL;
			par3_ctx->file_system_packet_size = 0;
			par3_ctx->file_system_packet_count = 0;
		} else if (new_size < par3_ctx->file_system_packet_size){
			tmp_p = realloc(par3_ctx->file_system_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for File System Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->file_system_packet = tmp_p;
			par3_ctx->file_system_packet_size = new_size;
			par3_ctx->file_system_packet_count = new_count;
		}
	}
	if (par3_ctx->ext_data_packet_size > 0){
		new_size = adjust_packet_buf(par3_ctx->ext_data_packet, par3_ctx->ext_data_packet_size, id_list, id_count, &new_count);
		if (new_size == 0){
			free(par3_ctx->ext_data_packet);
			par3_ctx->ext_data_packet = NULL;
			par3_ctx->ext_data_packet_size = 0;
			par3_ctx->ext_data_packet_count = 0;
		} else if (new_size < par3_ctx->ext_data_packet_size){
			tmp_p = realloc(par3_ctx->ext_data_packet, new_size);
			if (tmp_p == NULL){
				perror("Failed to re-allocate memory for External Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->ext_data_packet = tmp_p;
			par3_ctx->ext_data_packet_size = new_size;
			par3_ctx->ext_data_packet_count = new_count;
		}
	}

	if (par3_ctx->data_packet_count > 0){
		item_count = adjust_packet_list(par3_ctx->data_packet_list, par3_ctx->data_packet_count, id_list, id_count, NULL);
		if (item_count == 0){
			free(par3_ctx->data_packet_list);
			par3_ctx->data_packet_list = NULL;
			par3_ctx->data_packet_count = 0;
		} else if (item_count < par3_ctx->data_packet_count){
			list = realloc(par3_ctx->data_packet_list, sizeof(PAR3_PKT_CTX) * item_count);
			if (list == NULL){
				perror("Failed to re-allocate memory for Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->data_packet_list = list;
			par3_ctx->data_packet_count = item_count;
		}
	}
	if (par3_ctx->recv_packet_count > 0){
		if (par3_ctx->root_packet != NULL){
			tmp_p = par3_ctx->root_packet + 8;	// checksum from Root Packet
		} else {
			tmp_p = NULL;
		}
		item_count = adjust_packet_list(par3_ctx->recv_packet_list, par3_ctx->recv_packet_count, id_list, id_count, tmp_p);
		if (item_count == 0){
			free(par3_ctx->recv_packet_list);
			par3_ctx->recv_packet_list = NULL;
			par3_ctx->recv_packet_count = 0;
		} else if (item_count < par3_ctx->recv_packet_count){
			list = realloc(par3_ctx->recv_packet_list, sizeof(PAR3_PKT_CTX) * item_count);
			if (list == NULL){
				perror("Failed to re-allocate memory for Recovery Data Packet");
				return RET_MEMORY_ERROR;
			}
			par3_ctx->recv_packet_list = list;
			par3_ctx->recv_packet_count = item_count;
		}
	}

	return 0;
}

// check InputSetID of packets
int check_packet_set(PAR3_CTX *par3_ctx)
{
	if (par3_ctx->start_packet_count == 0){
		printf("Failed to find PAR3 Start Packet\n");
		return RET_INSUFFICIENT_DATA;
	}

	if (par3_ctx->start_packet_count == 1){
		// copy SetID from the packet
		memcpy(par3_ctx->set_id, par3_ctx->start_packet + 32, 8);
		if (par3_ctx->noise_level >= 2){
			printf("\n");
			printf("InputSetID = %02X %02X %02X %02X %02X %02X %02X %02X\n",
					par3_ctx->set_id[0], par3_ctx->set_id[1], par3_ctx->set_id[2], par3_ctx->set_id[3],
					par3_ctx->set_id[4], par3_ctx->set_id[5], par3_ctx->set_id[6], par3_ctx->set_id[7]);
		}

		// remove uesless packets by checking SetID
		if (remove_other_packet(par3_ctx, (uint64_t *)(par3_ctx->set_id), 1) != 0){
			return RET_MEMORY_ERROR;
		}

	} else {	// When there are multiple Start Packets, test "incremental backup".
		uint8_t *tmp_p;
		int i, id_count;
		size_t max, offset, packet_size;
		uint64_t *id_list, parent_id, this_id;

		id_list = malloc(sizeof(uint64_t) * par3_ctx->start_packet_count);
		if (id_list == NULL){
			perror("Failed to allocate memory for InputSetID");
			return RET_MEMORY_ERROR;
		}

		// check SetID of the first packet.
		id_count = 0;
		tmp_p = par3_ctx->start_packet;
		max = par3_ctx->start_packet_size;
		memcpy(&this_id, tmp_p + 32, 8);
		memcpy(&parent_id, tmp_p + 48 + 8, 8);
		memcpy(id_list + id_count, &this_id, 8);
		id_count++;

		if (parent_id == 0){	// This packet is the ancestor of PAR3 Sets.
			// search descendant Sets
			while (parent_id == 0){
				offset = 0;
				while (offset < max){
					// check parent's SetID
					if (memcmp(&this_id, tmp_p + offset + 48 + 8, 8) == 0){
						memcpy(&this_id, tmp_p + offset + 32, 8);
						memcpy(id_list + id_count, &this_id, 8);
						id_count++;

						parent_id = 0;	// search child again
						break;
					} else {
						parent_id = 1;
					}

					// goto next packet
					memcpy(&packet_size, tmp_p + offset + 24, 8);
					offset += packet_size;
				}
			}
			// use the last descendant's SetID
			memcpy(par3_ctx->set_id, &this_id, 8);

		} else {	// This packet is the child of another PAR3 Set.
			// use this SetID
			memcpy(par3_ctx->set_id, &this_id, 8);

			// search ancestor Sets
			while (parent_id != 0){
				offset = 0;
				while (offset < max){
					// check another SetID
					if (memcmp(&parent_id, tmp_p + offset + 32, 8) == 0){
						memcpy(id_list + id_count, &parent_id, 8);
						id_count++;

						memcpy(&parent_id, tmp_p + offset + 48 + 8, 8);
						// If parent_id isn't 0, search parent again.
						break;
					} else {
						parent_id = 0;
					}

					// goto next packet
					memcpy(&packet_size, tmp_p + offset + 24, 8);
					offset += packet_size;
				}
			}
		}

		if (par3_ctx->noise_level >= 2){
			printf("\n");
			// show SetIDs of PAR3 Sets.
			for (i = 0; i < id_count; i++){
				printf("InputSetID = %02"PRIx64" %02"PRIx64" %02"PRIx64" %02"PRIx64" %02"PRIx64" %02"PRIx64" %02"PRIx64" %02"PRIx64"\n",
						(id_list[i] & 0xFF), (id_list[i] >> 8) & 0xFF,
						(id_list[i] >> 16) & 0xFF, (id_list[i] >> 24) & 0xFF,
						(id_list[i] >> 32) & 0xFF, (id_list[i] >> 40) & 0xFF, (id_list[i] >> 48) & 0xFF, id_list[i] >> 56);
			}
		}

		// remove uesless packets by checking SetID
		if (remove_other_packet(par3_ctx, id_list, id_count) != 0){
			free(id_list);
			return RET_MEMORY_ERROR;
		}
		free(id_list);
	}

	if (par3_ctx->noise_level >= 1){
		printf("\nSet packet:\n");
		if (par3_ctx->creator_packet_count > 0)
			printf("Number of Creator Packet       =%3u (%4"PRId64" bytes)\n", par3_ctx->creator_packet_count, par3_ctx->creator_packet_size);
		if (par3_ctx->comment_packet_count > 0)
			printf("Number of Comment Packet       =%3u (%4"PRId64" bytes)\n", par3_ctx->comment_packet_count, par3_ctx->comment_packet_size);
		if (par3_ctx->start_packet_count > 0)
			printf("Number of Start Packet         =%3u (%4"PRId64" bytes)\n", par3_ctx->start_packet_count, par3_ctx->start_packet_size);
		if (par3_ctx->matrix_packet_count > 0)
			printf("Number of Matrix Packet        =%3u (%4"PRId64" bytes)\n", par3_ctx->matrix_packet_count, par3_ctx->matrix_packet_size);
		if (par3_ctx->file_packet_count > 0)
			printf("Number of File Packet          =%3u (%4"PRId64" bytes)\n", par3_ctx->file_packet_count, par3_ctx->file_packet_size);
		if (par3_ctx->dir_packet_count > 0)
			printf("Number of Directory Packet     =%3u (%4"PRId64" bytes)\n", par3_ctx->dir_packet_count, par3_ctx->dir_packet_size);
		if (par3_ctx->root_packet_count > 0)
			printf("Number of Root Packet          =%3u (%4"PRId64" bytes)\n", par3_ctx->root_packet_count, par3_ctx->root_packet_size);
		if (par3_ctx->file_system_packet_count > 0)
			printf("Number of File System Packet   =%3u (%4"PRId64" bytes)\n", par3_ctx->file_system_packet_count, par3_ctx->file_system_packet_size);
		if (par3_ctx->ext_data_packet_count > 0)
			printf("Number of External Data Packet =%3u (%4"PRId64" bytes)\n", par3_ctx->ext_data_packet_count, par3_ctx->ext_data_packet_size);
		if (par3_ctx->data_packet_count > 0)
			printf("Number of Data Packet          =%3"PRIu64"\n", par3_ctx->data_packet_count);
		if (par3_ctx->recv_packet_count > 0)
			printf("Number of Recovery Data Packet =%3"PRIu64"\n", par3_ctx->recv_packet_count);
	}

	return 0;
}

