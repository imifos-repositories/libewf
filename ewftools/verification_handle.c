/* 
 * Verification handle
 *
 * Copyright (c) 2006-2011, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include <libsystem.h>

#include "digest_context.h"
#include "digest_hash.h"
#include "ewfinput.h"
#include "md5.h"
#include "sha1.h"
#include "storage_media_buffer.h"
#include "verification_handle.h"

#define VERIFICATION_HANDLE_VALUE_SIZE			64
#define VERIFICATION_HANDLE_VALUE_IDENTIFIER_SIZE	32

#if !defined( USE_LIBEWF_GET_HASH_VALUE_MD5 ) && !defined( USE_LIBEWF_GET_MD5_HASH )
#define USE_LIBEWF_GET_HASH_VALUE_MD5
#endif

/* Initializes the verification handle
 * Returns 1 if successful or -1 on error
 */
int verification_handle_initialize(
     verification_handle_t **verification_handle,
     uint8_t calculate_md5,
     uint8_t calculate_sha1,
     liberror_error_t **error )
{
	static char *function = "verification_handle_initialize";

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( *verification_handle == NULL )
	{
		*verification_handle = memory_allocate_structure(
		                        verification_handle_t );

		if( *verification_handle == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create verification handle.",
			 function );

			goto on_error;
		}
		if( memory_set(
		     *verification_handle,
		     0,
		     sizeof( verification_handle_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear verification handle.",
			 function );

			memory_free(
			 *verification_handle );

			*verification_handle = NULL;

			return( -1 );
		}
		if( libewf_handle_initialize(
		     &( ( *verification_handle )->input_handle ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize input handle.",
			 function );

			goto on_error;
		}
#ifdef TODO
		/* TODO: have application determine limit value and set to value - 4 */
		if( libewf_handle_set_maximum_number_of_open_handles(
		     ( *verification_handle )->input_handle,
		     1000,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set maximum number of open handles.",
			 function );

			goto on_error;
		}
#endif
		( *verification_handle )->calculate_md5  = calculate_md5;
		( *verification_handle )->calculate_sha1 = calculate_sha1;

		if( ( ( *verification_handle )->calculate_md5 != 0 )
		 && ( md5_initialize(
		       &( ( *verification_handle )->md5_context ),
		       error ) != 1 ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize MD5 context.",
			 function );

			goto on_error;
		}
		if( ( ( *verification_handle )->calculate_sha1 != 0 )
		 && ( sha1_initialize(
		       &( ( *verification_handle )->sha1_context ),
		       error ) != 1 ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize SHA1 context.",
			 function );

			goto on_error;
		}
		( *verification_handle )->header_codepage = LIBEWF_CODEPAGE_ASCII;
	}
	return( 1 );

on_error:
	if( *verification_handle != NULL )
	{
		if( ( *verification_handle )->input_handle != NULL )
		{
			libewf_handle_free(
			 &( ( *verification_handle )->input_handle ),
			 NULL );
		}
		memory_free(
		 *verification_handle );

		*verification_handle = NULL;
	}
	return( -1 );
}

/* Frees the verification handle and its elements
 * Returns 1 if successful or -1 on error
 */
int verification_handle_free(
     verification_handle_t **verification_handle,
     liberror_error_t **error )
{
	static char *function = "verification_handle_free";
	int result            = 1;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( *verification_handle != NULL )
	{
		if( ( ( *verification_handle )->input_handle != NULL )
		 && ( libewf_handle_free(
		       &( ( *verification_handle )->input_handle ),
		       error ) != 1 ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free input handle.",
			 function );

			result = -1;
		}
		memory_free(
		 *verification_handle );

		*verification_handle = NULL;
	}
	return( result );
}

/* Signals the verification handle to abort
 * Returns 1 if successful or -1 on error
 */
int verification_handle_signal_abort(
     verification_handle_t *verification_handle,
     liberror_error_t **error )
{
	static char *function = "verification_handle_signal_abort";

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle != NULL )
	{
		if( libewf_handle_signal_abort(
		     verification_handle->input_handle,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to signal input handle to abort.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Opens the input of the verification handle
 * Returns 1 if successful or -1 on error
 */
int verification_handle_open_input(
     verification_handle_t *verification_handle,
     libcstring_system_character_t * const * filenames,
     int number_of_filenames,
     liberror_error_t **error )
{
	libcstring_system_character_t **libewf_filenames = NULL;
	static char *function                            = "verification_handle_open_input";
	size_t first_filename_length                     = 0;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( filenames == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filenames.",
		 function );

		return( -1 );
	}
	if( number_of_filenames <= 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid number of filenames.",
		 function );

		return( -1 );
	}
	if( number_of_filenames == 1 )
	{
		first_filename_length = libcstring_system_string_length(
		                         filenames[ 0 ] );

#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		if( libewf_glob_wide(
		     filenames[ 0 ],
		     first_filename_length,
		     LIBEWF_FORMAT_UNKNOWN,
		     &libewf_filenames,
		     &number_of_filenames,
		     error ) != 1 )
#else
		if( libewf_glob(
		     filenames[ 0 ],
		     first_filename_length,
		     LIBEWF_FORMAT_UNKNOWN,
		     &libewf_filenames,
		     &number_of_filenames,
		     error ) != 1 )
#endif
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to resolve filename(s).",
			 function );

			goto on_error;
		}
		filenames = (libcstring_system_character_t * const *) libewf_filenames;
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	if( libewf_handle_open_wide(
	     verification_handle->input_handle,
	     filenames,
	     number_of_filenames,
	     LIBEWF_OPEN_READ,
	     error ) != 1 )
#else
	if( libewf_handle_open(
	     verification_handle->input_handle,
	     filenames,
	     number_of_filenames,
	     LIBEWF_OPEN_READ,
	     error ) != 1 )
#endif
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open files.",
		 function );

		goto on_error;
	}
	if( verification_handle->header_codepage != LIBEWF_CODEPAGE_ASCII )
	{
		if( libewf_handle_set_header_codepage(
		     verification_handle->input_handle,
		     verification_handle->header_codepage,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set header codepage.",
			 function );

			goto on_error;
		}
	}
	if( libewf_handle_get_chunk_size(
	     verification_handle->input_handle,
	     &( verification_handle->chunk_size ),
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve chunk size.",
		 function );

		goto on_error;
	}
	if( libewf_filenames != NULL )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		if( libewf_glob_wide_free(
		     libewf_filenames,
		     number_of_filenames,
		     error ) != 1 )
#else
		if( libewf_glob_free(
		     libewf_filenames,
		     number_of_filenames,
		     error ) != 1 )
#endif
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free globbed filenames.",
			 function );

			return( -1 );
		}
		libewf_filenames = NULL;
	}
	return( 1 );

on_error:
	if( libewf_filenames != NULL )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		libewf_glob_wide_free(
		 libewf_filenames,
		 number_of_filenames,
		 NULL );
#else
		libewf_glob_free(
		 libewf_filenames,
		 number_of_filenames,
		 NULL );
#endif
		libewf_filenames = NULL;
	}
	return( -1 );
}

/* Closes the verification handle
 * Returns the 0 if succesful or -1 on error
 */
int verification_handle_close(
     verification_handle_t *verification_handle,
     liberror_error_t **error )
{
	static char *function = "verification_handle_close";

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( libewf_handle_close(
	     verification_handle->input_handle,
	     error ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_CLOSE_FAILED,
		 "%s: unable to close input handle.",
		 function );

		return( -1 );
	}
	return( 0 );
}

/* Prepares a buffer after reading the input of the verification handle
 * Returns the resulting buffer size or -1 on error
 */
ssize_t verification_handle_prepare_read_buffer(
         verification_handle_t *verification_handle,
         storage_media_buffer_t *storage_media_buffer,
         liberror_error_t **error )
{
	static char *function = "verification_handle_prepare_read_buffer";
	ssize_t process_count = 0;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( storage_media_buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid storage media buffer.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	storage_media_buffer->raw_buffer_data_size = storage_media_buffer->raw_buffer_size;

	process_count = libewf_handle_prepare_read_chunk(
	                 verification_handle->input_handle,
	                 storage_media_buffer->compression_buffer,
	                 storage_media_buffer->compression_buffer_data_size,
	                 storage_media_buffer->raw_buffer,
	                 &( storage_media_buffer->raw_buffer_data_size ),
	                 storage_media_buffer->is_compressed,
	                 storage_media_buffer->checksum,
	                 storage_media_buffer->process_checksum,
	                 error );

	if( process_count == -1 )
	{
		liberror_error_free(
		 error );

		/* Wipe the chunk if nescessary
		 */
		if( verification_handle->wipe_chunk_on_error != 0 )
		{
			if( ( storage_media_buffer->is_compressed != 0 )
			 && ( memory_set(
			       storage_media_buffer->raw_buffer,
			       0,
			       storage_media_buffer->raw_buffer_size ) == NULL ) )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_MEMORY,
				 LIBERROR_MEMORY_ERROR_SET_FAILED,
				 "%s: unable to wipe raw buffer.",
				 function );

				return( -1 );
			}
			if( memory_set(
			     storage_media_buffer->compression_buffer,
			     0,
			     storage_media_buffer->compression_buffer_size ) == NULL )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_MEMORY,
				 LIBERROR_MEMORY_ERROR_SET_FAILED,
				 "%s: unable to wipe compression buffer.",
				 function );

				return( -1 );
			}
		}
		process_count = verification_handle->chunk_size;

		/* Append a read error
		 */
		if( verification_handle_append_read_error(
		     verification_handle,
		     verification_handle->last_offset_read,
		     process_count,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
			 "%s: unable to append read error.",
			 function );

			return( -1 );
		}
	}
	if( storage_media_buffer->is_compressed == 0 )
	{
		storage_media_buffer->data_in_compression_buffer = 1;
	}
	else
	{
		storage_media_buffer->data_in_compression_buffer = 0;
	}
#else
	process_count = (ssize_t) storage_media_buffer->raw_buffer_data_size;
#endif
	verification_handle->last_offset_read += process_count;

	return( process_count );
}

/* Reads a buffer from the input of the verification handle
 * Returns the number of bytes written or -1 on error
 */
ssize_t verification_handle_read_buffer(
         verification_handle_t *verification_handle,
         storage_media_buffer_t *storage_media_buffer,
         size_t read_size,
         liberror_error_t **error )
{
	static char *function = "verification_handle_read_buffer";
	ssize_t read_count    = 0;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( storage_media_buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid storage media buffer.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	read_count = libewf_handle_read_chunk(
                      verification_handle->input_handle,
                      storage_media_buffer->compression_buffer,
                      storage_media_buffer->compression_buffer_size,
	              &( storage_media_buffer->is_compressed ),
	              &( storage_media_buffer->compression_buffer[ storage_media_buffer->raw_buffer_size ] ),
	              &( storage_media_buffer->checksum ),
	              &( storage_media_buffer->process_checksum ),
	              error );
#else
	read_count = libewf_handle_read_buffer(
                      verification_handle->input_handle,
                      storage_media_buffer->raw_buffer,
                      read_size,
	              error );
#endif

	if( read_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read storage media buffer.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	storage_media_buffer->compression_buffer_data_size = (size_t) read_count;
#else
	storage_media_buffer->raw_buffer_data_size         = (size_t) read_count;
#endif

	return( read_count );
}

/* Updates the integrity hash(es)
 * Returns 1 if successful or -1 on error
 */
int verification_handle_update_integrity_hash(
     verification_handle_t *verification_handle,
     storage_media_buffer_t *storage_media_buffer,
     size_t read_size,
     liberror_error_t **error )
{
	uint8_t *data         = NULL;
	static char *function = "verification_handle_update_integrity_hash";
	size_t data_size      = 0;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( storage_media_buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid storage media buffer.",
		 function );

		return( -1 );
	}
	if( ( read_size == 0 )
	 || ( read_size > (size_t) SSIZE_MAX ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid size value out of bounds.",
		 function );

		return( -1 );
	}
	if( storage_media_buffer_get_data(
	     storage_media_buffer,
	     &data,
	     &data_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine storage media buffer data.",
		 function );

		return( -1 );
	}
	if( verification_handle->calculate_md5 != 0 )
	{
		/* TODO check for return value */
		md5_update(
		 &( verification_handle->md5_context ),
		 data,
		 read_size,
		 error );

		if( ( error != NULL )
		 && ( *error != NULL ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to update MD5 digest hash.",
			 function );

			return( -1 );
		}
	}
	if( verification_handle->calculate_sha1 != 0 )
	{
		/* TODO check for return value */
		sha1_update(
		 &( verification_handle->sha1_context ),
		 data,
		 read_size,
		 error );

		if( ( error != NULL )
		 && ( *error != NULL ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to update SHA1 digest hash.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Retrieves several verification values
 * The chunk size is set to 0 if not available
 * Returns 1 if successful or -1 on error
 */
int verification_handle_get_values(
     verification_handle_t *verification_handle,
     size64_t *media_size,
     uint32_t *chunk_size,
     liberror_error_t **error )
{
	static char *function = "verification_handle_get_values";

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( chunk_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk size.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_media_size(
	     verification_handle->input_handle,
	     media_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve media size.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_bytes_per_sector(
	     verification_handle->input_handle,
	     &( verification_handle->bytes_per_sector ),
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve bytes per sector.",
		 function );

		return( -1 );
	}
	*chunk_size = verification_handle->chunk_size;

	return( 1 );
}


/* Retrieves the number of checksum errors
 * Returns 1 if successful or -1 on error
 */
int verification_handle_get_number_of_checksum_errors(
     verification_handle_t *verification_handle,
     uint32_t *number_of_errors,
     liberror_error_t **error )
{
	static char *function = "verification_handle_get_number_of_checksum_errors";

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( number_of_errors == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid number of errors.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_number_of_checksum_errors(
	     verification_handle->input_handle,
	     number_of_errors,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of checksum errors.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Sets the header codepage
 * Returns 1 if successful or -1 on error
 */
int verification_handle_set_header_codepage(
     verification_handle_t *verification_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error )
{
	static char *function = "verification_handle_set_header_codepage";
	int result            = 0;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	result = ewfinput_determine_header_codepage(
	          string,
	          &( verification_handle->header_codepage ),
	          error );

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine header codepage.",
		 function );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( verification_handle->input_handle != NULL )
		{
			if( libewf_handle_set_header_codepage(
			     verification_handle->input_handle,
			     verification_handle->header_codepage,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set header codepage.",
				 function );

				return( -1 );
			}
		}
	}
	return( result );
}

/* Sets the error handling values of the verification handle
 * Returns 1 if successful or -1 on error
 */
int verification_handle_set_error_handling_values(
     verification_handle_t *verification_handle,
     int wipe_chunk_on_error,
     liberror_error_t **error )
{
	static char *function = "verification_handle_set_error_handling_values";

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( libewf_handle_set_read_wipe_chunk_on_error(
	     verification_handle->input_handle,
	     (uint8_t) wipe_chunk_on_error,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set wipe chunk on error in input handle.",
		 function );

		return( -1 );
	}
	verification_handle->wipe_chunk_on_error = wipe_chunk_on_error;

	return( 1 );
}

#if defined( HAVE_LOW_LEVEL_FUNCTIONS )

/* Appends a read error to the output handle
 * Returns 1 if successful or -1 on error
 */
int verification_handle_append_read_error(
      verification_handle_t *verification_handle,
      off64_t start_offset,
      size_t number_of_bytes,
      liberror_error_t **error )
{
	static char *function      = "verification_handle_append_read_error";
	uint64_t start_sector      = 0;
	uint64_t number_of_sectors = 0;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->bytes_per_sector == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid verification handle - invalid bytes per sector value out of bounds.",
		 function );

		return( -1 );
	}
	start_sector      = start_offset / verification_handle->bytes_per_sector;
	number_of_sectors = number_of_bytes / verification_handle->bytes_per_sector;

	if( ( number_of_bytes % verification_handle->bytes_per_sector ) != 0 )
	{
		number_of_sectors += 1;
	}
	if( libewf_handle_append_checksum_error(
	     verification_handle->input_handle,
	     start_sector,
	     number_of_sectors,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append checksum error.",
		 function );

		return( -1 );
	}
	return( 1 );
}
#endif

/* Finalizes the verification handle
 * Returns 1 if successful or -1 on error
 */
int verification_handle_finalize(
     verification_handle_t *verification_handle,
     libcstring_system_character_t *stored_md5_hash_string,
     size_t stored_md5_hash_string_size,
     int *stored_md5_hash_available,
     libcstring_system_character_t *stored_sha1_hash_string,
     size_t stored_sha1_hash_string_size,
     int *stored_sha1_hash_available,
     liberror_error_t **error )
{
#if defined( USE_LIBEWF_GET_MD5_HASH )
        digest_hash_t stored_md5_hash[ DIGEST_HASH_SIZE_MD5 ];
#endif

	digest_hash_t calculated_md5_hash[ DIGEST_HASH_SIZE_MD5 ];
	digest_hash_t calculated_sha1_hash[ DIGEST_HASH_SIZE_SHA1 ];

	static char *function            = "verification_handle_finalize";
	size_t calculated_md5_hash_size  = DIGEST_HASH_SIZE_MD5;
	size_t calculated_sha1_hash_size = DIGEST_HASH_SIZE_SHA1;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->calculate_md5 != 0 )
	{
		/* Finalize the MD5 hash calculation
		 */
		if( md5_finalize(
		     &( verification_handle->md5_context ),
		     calculated_md5_hash,
		     &calculated_md5_hash_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to finalize MD5 hash.",
			 function );

			return( -1 );
		}
		if( digest_hash_copy_to_string(
		     calculated_md5_hash,
		     calculated_md5_hash_size,
		     verification_handle->md5_hash_string,
		     DIGEST_HASH_STRING_SIZE_MD5,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set calculated MD5 hash string.",
			 function );

			return( -1 );
		}
		if( stored_md5_hash_available == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			 "%s: invalid stored MD5 hash available value.",
			 function );

			return( -1 );
		}
#if defined( USE_LIBEWF_GET_MD5_HASH )
		*stored_md5_hash_available = libewf_handle_get_md5_hash(
					      verification_handle->input_handle,
					      md5_hash,
					      DIGEST_HASH_SIZE_MD5,
					      error );

		if( *stored_md5_hash_available == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine stored MD5 hash string.",
			 function );

			return( -1 );
		}
		if( digest_hash_copy_to_string(
		     md5_hash,
		     DIGEST_HASH_SIZE_MD5,
		     stored_md5_hash_string,
		     stored_md5_hash_string_size ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set stored MD5 hash string.",
			 function );

			return( -1 );
		}
#elif defined( USE_LIBEWF_GET_HASH_VALUE_MD5 )
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		*stored_md5_hash_available = libewf_handle_get_utf16_hash_value(
		                              verification_handle->input_handle,
		                              (uint8_t *) "MD5",
		                              3,
		                              (uint16_t *) stored_md5_hash_string,
		                              stored_md5_hash_string_size,
		                              error );
#else
		*stored_md5_hash_available = libewf_handle_get_utf8_hash_value(
		                              verification_handle->input_handle,
		                              (uint8_t *) "MD5",
		                              3,
		                              (uint8_t *) stored_md5_hash_string,
		                              stored_md5_hash_string_size,
		                              error );
#endif
		if( *stored_md5_hash_available == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine stored MD5 hash string.",
			 function );

			return( -1 );
		}
#endif
	}
	if( verification_handle->calculate_sha1 != 0 )
	{
		/* Finalize the SHA1 hash calculation
		 */
		if( sha1_finalize(
		     &( verification_handle->sha1_context ),
		     calculated_sha1_hash,
		     &calculated_sha1_hash_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to finalize SHA1 hash.",
			 function );

			return( -1 );
		}
		if( digest_hash_copy_to_string(
		     calculated_sha1_hash,
		     calculated_sha1_hash_size,
		     verification_handle->sha1_hash_string,
		     DIGEST_HASH_STRING_SIZE_SHA1,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create calculated SHA1 hash string.",
			 function );

			return( -1 );
		}
		if( stored_sha1_hash_available == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			 "%s: invalid stored SHA1 hash available value.",
			 function );

			return( -1 );
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		*stored_sha1_hash_available = libewf_handle_get_utf16_hash_value(
		                               verification_handle->input_handle,
		                               (uint8_t *) "SHA1",
		                               4,
		                               (uint16_t *) stored_sha1_hash_string,
		                               stored_sha1_hash_string_size,
		                               error );
#else
		*stored_sha1_hash_available = libewf_handle_get_utf8_hash_value(
		                               verification_handle->input_handle,
		                               (uint8_t *) "SHA1",
		                               4,
		                               (uint8_t *) stored_sha1_hash_string,
		                               stored_sha1_hash_string_size,
		                               error );
#endif
		if( *stored_sha1_hash_available == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine stored SHA1 hash string.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Print the additional hash values to a stream
 * Returns 1 if successful or -1 on error
 */
int verification_handle_additional_hash_values_fprint(
     verification_handle_t *verification_handle,
     FILE *stream,
     liberror_error_t **error )
{
	char hash_value_identifier[ VERIFICATION_HANDLE_VALUE_IDENTIFIER_SIZE ];
	libcstring_system_character_t hash_value[ VERIFICATION_HANDLE_VALUE_SIZE ];

	static char *function             = "verification_handle_additional_hash_values_fprint";
	size_t hash_value_identifier_size = VERIFICATION_HANDLE_VALUE_IDENTIFIER_SIZE;
	size_t hash_value_size            = VERIFICATION_HANDLE_VALUE_SIZE;
	uint32_t number_of_values         = 0;
	uint32_t hash_value_iterator      = 0;
	uint8_t print_header              = 1;
	int result                        = 1;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid stream.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_number_of_hash_values(
	     verification_handle->input_handle,
	     &number_of_values,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of hash values.",
		 function );

		return( -1 );
	}
	for( hash_value_iterator = 0;
	     hash_value_iterator < number_of_values;
	     hash_value_iterator++ )
	{
		if( libewf_handle_get_hash_value_identifier_size(
		     verification_handle->input_handle,
		     hash_value_iterator,
		     &hash_value_identifier_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve the hash value identifier size for index: %" PRIu32 ".",
			 function,
			 hash_value_iterator );

			result = -1;

			continue;
		}
		if( hash_value_identifier_size > VERIFICATION_HANDLE_VALUE_IDENTIFIER_SIZE )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: hash value identifier size value out of bounds for index: %" PRIu32 ".",
			 function,
			 hash_value_iterator );

			result = -1;

			continue;
		}
		if( libewf_handle_get_hash_value_identifier(
		     verification_handle->input_handle,
		     hash_value_iterator,
		     (uint8_t *) hash_value_identifier,
		     hash_value_identifier_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve the hash identifier for index: %" PRIu32 ".",
			 function,
			 hash_value_iterator );

			result = -1;

			continue;
		}
		if( ( verification_handle->calculate_md5 != 0 )
		 && ( libcstring_narrow_string_compare(
		       hash_value_identifier,
		       "MD5",
		       3 ) == 0 ) )
		{
			continue;
		}
		if( ( verification_handle->calculate_sha1 != 0 )
		 && ( libcstring_narrow_string_compare(
		       hash_value_identifier,
		       "SHA1",
		       4 ) == 0 ) )
		{
			continue;
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		if( libewf_handle_get_utf16_hash_value(
		     verification_handle->input_handle,
		     (uint8_t *) hash_value_identifier,
		     hash_value_identifier_size - 1,
		     (uint16_t *) hash_value,
		     hash_value_size,
		     error ) != 1 )
#else
		if( libewf_handle_get_utf8_hash_value(
		     verification_handle->input_handle,
		     (uint8_t *) hash_value_identifier,
		     hash_value_identifier_size - 1,
		     (uint8_t *) hash_value,
		     hash_value_size,
		     error ) != 1 )
#endif
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve the hash value for identifier: %s.",
			 function,
			 hash_value_identifier );

			result = -1;
		}
		else
		{
			if( print_header != 0 )
			{
				fprintf(
				 stream,
				 "\nAdditional hash values:\n" );

				print_header = 0;
			}
			fprintf(
			 stream,
			 "%s:\t%" PRIs_LIBCSTRING_SYSTEM "\n",
			 hash_value_identifier,
			 hash_value );
		}
	}
	return( result );
}

/* Print the checksum errors to a stream
 * Returns 1 if successful or -1 on error
 */
int verification_handle_checksum_errors_fprint(
     verification_handle_t *verification_handle,
     FILE *stream,
     liberror_error_t **error )
{
	libcstring_system_character_t *filename      = NULL;
	libcstring_system_character_t *last_filename = NULL;
	static char *function                        = "verification_handle_checksum_errors_fprint";
	size_t filename_size                         = 0;
	size_t last_filename_size                    = 0;
	uint64_t start_sector                        = 0;
	uint64_t last_sector                         = 0;
	uint64_t number_of_sectors                   = 0;
	uint32_t number_of_errors                    = 0;
	uint32_t error_index                         = 0;
	int result                                   = 1;

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( verification_handle->input_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid verification handle - missing input handle.",
		 function );

		return( -1 );
	}
	if( stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid stream.",
		 function );

		return( -1 );
	}
	if( libewf_handle_get_number_of_checksum_errors(
	     verification_handle->input_handle,
	     &number_of_errors,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of checksum errors.",
		 function );

		goto on_error;
	}
	if( number_of_errors > 0 )
	{
		fprintf( 
		stream,
		 "Sector validation errors:\n" );
		fprintf(
		 stream,
		 "\ttotal number: %" PRIu32 "\n",
		 number_of_errors );

		for( error_index = 0;
		     error_index < number_of_errors;
		     error_index++ )
		{
			if( libewf_handle_get_checksum_error(
			     verification_handle->input_handle,
			     error_index,
			     &start_sector,
			     &number_of_sectors,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve the checksum error: %" PRIu32 ".",
				 function,
				 error_index );

				start_sector      = 0;
				number_of_sectors = 0;

				result = -1;

				continue;
			}
			last_sector = start_sector + number_of_sectors - 1;

			fprintf(
			 stream,
			 "\tat sector(s): %" PRIu64 " - %" PRIu64 " (number: %" PRIu64 ")",
			 start_sector,
			 last_sector,
			 number_of_sectors );

			fprintf(
			 stream,
			 " in segment file(s):" );

			start_sector *= verification_handle->bytes_per_sector;
			last_sector  *= verification_handle->bytes_per_sector;

			while( start_sector <= last_sector )
			{
				if( libewf_handle_seek_offset(
				     verification_handle->input_handle,
				     (off64_t) start_sector,
				     SEEK_SET,
				     error ) == -1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_SEEK_FAILED,
					 "%s: unable to seek offset: %" PRIi64 ".",
					 function,
					 start_sector );

					goto on_error;
				}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
				if( libewf_handle_get_filename_size_wide(
				     verification_handle->input_handle,
				     &filename_size,
				     error ) != 1 )
#else
				if( libewf_handle_get_filename_size(
				     verification_handle->input_handle,
				     &filename_size,
				     error ) != 1 )
#endif
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve filename size.",
					 function );

					goto on_error;
				}
				filename = libcstring_system_string_allocate(
				            filename_size ); 


				if( filename == NULL )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_MEMORY,
					 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
					 "%s: unable to create filename.",
					 function );

					goto on_error;
				}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
				if( libewf_handle_get_filename_wide(
				     verification_handle->input_handle,
				     filename,
				     256,
				     error ) != 1 )
#else
				if( libewf_handle_get_filename(
				     verification_handle->input_handle,
				     filename,
				     256,
				     error ) != 1 )
#endif
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_GET_FAILED,
					 "%s: unable to retrieve filename.",
					 function );

					goto on_error;
				}
				if( last_filename == NULL )
				{
					fprintf(
					 stream,
					 " %s",
					 filename );

					last_filename      = filename;
					last_filename_size = filename_size;
				}
				else if( ( last_filename_size != filename_size )
				      || ( memory_compare(
				            last_filename,
				            filename,
				            filename_size ) != 0 ) )
				{
					fprintf(
					 stream,
					 ", %s",
					 filename );

					memory_free(
					 last_filename );

					last_filename      = filename;
					last_filename_size = filename_size;
				}
				else
				{
					memory_free(
					 filename );
				}
				filename      = NULL;
				filename_size = 0;

				start_sector += verification_handle->chunk_size;
			}
			memory_free(
			 last_filename );

			last_filename      = NULL;
			last_filename_size = 0;

			fprintf(
			 stream,
			 "\n" );
		}
		fprintf(
		 stream,
		 "\n" );
	}
	return( result );

on_error:
	if( last_filename != NULL )
	{
		memory_free(
		 last_filename );
	}
	if( filename != NULL )
	{
		memory_free(
		 filename );
	}
	return( -1 );
}

