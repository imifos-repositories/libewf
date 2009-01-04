/*
 * libewf offset table
 *
 * Copyright (c) 2006-2007, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the creator, related organisations, nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER, COMPANY AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined( _LIBEWF_OFFSET_TABLE_H )
#define _LIBEWF_OFFSET_TABLE_H

#include "libewf_includes.h"

#include "libewf_section_list.h"

#include "ewf_table.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define LIBEWF_OFFSET_TABLE libewf_offset_table_t
#define LIBEWF_OFFSET_TABLE_SIZE sizeof( LIBEWF_OFFSET_TABLE )
#define LIBEWF_OFFSET_TABLE_FILE_DESCRIPTOR_SIZE sizeof( int )
#define LIBEWF_OFFSET_TABLE_COMPRESSED_SIZE sizeof( uint8_t )
#define LIBEWF_OFFSET_TABLE_OFFSET_SIZE sizeof( off64_t )
#define LIBEWF_OFFSET_TABLE_SIZE_SIZE sizeof( size_t )
#define LIBEWF_OFFSET_TABLE_HASHED_SIZE sizeof( uint8_t )
#define LIBEWF_OFFSET_TABLE_SEGMENT_NUMBER_SIZE sizeof( uint16_t )
#define LIBEWF_OFFSET_TABLE_DIRTY_SIZE sizeof( uint8_t )

typedef struct libewf_offset_table libewf_offset_table_t;

struct libewf_offset_table
{
	/* Stores the amount of chunks in the table
	 * There is an offset per chunk in the table
	 */
	uint32_t amount;

	/* The last chunk that was defined
	 */
	uint32_t last;

	/* Dynamic array of file descriptor into the
	 * correct file, must already be opened by initialiser
	 */
	int *file_descriptor;

	/* Dynamic array of boolean that definines
	 * if the chunk is compressed
	 */
	uint8_t *compressed;

	/* Dynamic array of offsets
	 */
	off64_t *offset;

	/* Dynamic array of chunk sizes
	 */
	size_t *size;

	/* Dynamic array of boolean that definines
	 * if the chunk has been processed in the hash calculation
	 */
	uint8_t *hashed;

	/* Dynamic array of the segment numbers
	 */
	uint16_t *segment_number;

	/* Dynamic array of boolean that definines
	 * if the chunk is stored in the delta file
	 */
	uint8_t *dirty;
};

LIBEWF_OFFSET_TABLE *libewf_offset_table_alloc( uint32_t amount );
int libewf_offset_table_realloc( LIBEWF_OFFSET_TABLE *offset_table, uint32_t amount );
void libewf_offset_table_free( LIBEWF_OFFSET_TABLE *offset_table );
int8_t libewf_offset_table_set_values( LIBEWF_OFFSET_TABLE *offset_table, uint32_t chunk, int file_descriptor, uint8_t compressed, off64_t offset, size_t size, uint16_t segment_number, uint8_t dirty );

int libewf_offset_table_fill( LIBEWF_OFFSET_TABLE *offset_table, off64_t base_offset, EWF_TABLE_OFFSET *offsets, uint32_t amount_of_chunks, int file_descriptor, uint16_t segment_number, uint8_t error_tollerance );
int libewf_offset_table_calculate_last_offset( LIBEWF_OFFSET_TABLE *offset_table, LIBEWF_SECTION_LIST *section_list, int file_descriptor, uint16_t segment_number, uint8_t error_tollerance );
int libewf_offset_table_compare( LIBEWF_OFFSET_TABLE *offset_table1, LIBEWF_OFFSET_TABLE *offset_table2 );

#if defined( __cplusplus )
}
#endif

#endif

