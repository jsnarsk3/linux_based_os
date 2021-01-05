#include "filesys.h"

uint32_t* filesys_ptr = NULL;       /* ptr to current open file system                      */
dentry_t open_dentry;               /* struct containing last opened dentry                 */
dentry_t read_dentry;               /* struct containing last read dentry                   */
bblock_t bblock;                    /* struct containing boot block info for file system    */
file_desc_t file_desc;              /* struct for file descriptor                           */
/* filename buffer for read subroutines */
static int8_t filename_buf[NAME_LEN] __attribute__((aligned(FOUR_KB)));

/*
 * init_filesys
 * DESCRIPTION: saves pointer to filesystem and initiliazes all structs associated with file operations
 * INPUTS: ptr: pointer to starting address of filesystem
 * OUTPUTS: none
 * RETURN VALUE: none
 */
void init_filesys(uint32_t* ptr) {
    int idx;

    /* save pointer to filesys */
    filesys_ptr = (uint32_t*) ptr;

    /* fill in boot block structure */
    bblock.n_dir_entries = *(filesys_ptr + 0);
    bblock.N = *(filesys_ptr + 1);
    bblock.D = *(filesys_ptr + 2);
    bblock.reserved = 0;

    /* init open_dentry struct */
    for (idx = 0; idx < NAME_LEN; idx++) {
        open_dentry.filename[idx] = 0;
        if (idx < RES_LEN) {
            open_dentry.reserved[idx] = 0;
        }
    }
    open_dentry.filetype = 0;
    open_dentry.inode_num = 0;

    /* init read_dentry struct */
    for (idx = 0; idx < NAME_LEN; idx++) {
        read_dentry.filename[idx] = 0;
        if (idx < RES_LEN) {
            read_dentry.reserved[idx] = 0;
        }
    }
    read_dentry.filetype = 0;
    read_dentry.inode_num = 0;
}

/*
 * get_read_dentry
 * DESCRIPTION: returns read_dentry struct for testing purposes
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: address of read_dentry
 */
dentry_t* get_read_dentry() {
    return &read_dentry;
}

/*
 * get_open_dentry
 * DESCRIPTION: returns open_dentry struct for testing purposes
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: address of open_dentry
 */
dentry_t* get_open_dentry() {
    return &open_dentry;
}

/*
 * get_file_desc
 * DESCRIPTION: returns file_desc struct for testing purposes
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: address of file_desc
 */

file_desc_t* get_file_desc() {
    return &file_desc;
}

/*
 * get_file_size
 * DESCRIPTION: returns file_size from input dentry struct, for testing purposes
 * INPUTS: dentry: struct to get size of
 * OUTPUTS: none
 * RETURN VALUE: size of specified dentry struct
 */
int32_t get_file_size(dentry_t* dentry) {
    if (dentry == NULL) {
        return -1;
    }

    uint32_t* size_ptr;

    if (dentry->filetype != REG_FILE) {
        return 0;
    }

    size_ptr = filesys_ptr + (FOUR_KB/4)*((dentry->inode_num) + 1);

    return *size_ptr;
}

/*
 * read_dentry_by_name
 * DESCRIPTION: copies dentry struct from memory to input dentry
 * INPUTS:  fname: filename of src dentry
 *          dentry: pointer to dest dentry
 * OUTPUTS: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    int32_t dir_it;
    int32_t name_it;
    uint32_t str_length;

    int8_t* name_ptr = (int8_t *) filesys_ptr;
    int8_t* temp;
    uint32_t* dentry_ptr;

    /* check validity of ptrs */
    if (fname == NULL || dentry == NULL || name_ptr == NULL) {
        return -1;
    }

    if (strlen((int8_t*) fname) > NAME_LEN) {
        return -1;
    }

    /* iterate through directory and search for fname */
    for (dir_it = 1; dir_it <= bblock.n_dir_entries; dir_it++) {
        /* obtain string length at current directory entry and copy filename to buffer */
        str_length = strlen((name_ptr + DENTRY_SIZE*dir_it));
        temp = strncpy(filename_buf, name_ptr + DENTRY_SIZE*dir_it, (int32_t) str_length);
        /* add ending 0 so (for debugging) */
        if (str_length < NAME_LEN) {
            filename_buf[str_length] = 0;
        }

        /* if dentry filename and input fname match, break */
        //min_str_length = (str_length < NAME_LEN) ? str_length : NAME_LEN;
        if (strncmp((int8_t*) fname, filename_buf, NAME_LEN) == 0) {
            dentry_ptr = filesys_ptr + (DENTRY_SIZE/4)*dir_it;
            break;
        }
    }
    /* if the filename wasn't found, return failure */
    if (dir_it > bblock.n_dir_entries) {
        return -1;
    }

    /* else fill open_dentry struct and return success */
    temp = strncpy((int8_t*) open_dentry.filename, filename_buf, (int32_t) str_length);
    /* zero pad name */
    for (name_it = str_length; name_it < NAME_LEN; name_it++) {
        dentry->filename[name_it] = 0;
    }
    /* copy file type to open_dentry */
    dentry->filetype = *(dentry_ptr + FN_OFFSET);
    /* ignore inode numbers for non-regular files (filetype = 0, 1); else copy to open_dentry */
    if (dentry->filetype == REG_FILE) {
        dentry->inode_num = *(dentry_ptr + FT_OFFSET);
    }
    else {
        dentry->inode_num = 0;
    }

    /* return success */
    return 0;
}

/*
 * read_dentry_by_index
 * DESCRIPTION: copies dentry struct from memory to input dentry
 * INPUTS:  index: index in boot block of source dentry
 *          dentry: pointer to dest dentry
 * OUTPUTS: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    int32_t name_it;
    uint32_t str_length;
    int8_t* temp;

    int8_t* name_ptr;
    uint32_t* dentry_ptr;

    /* check validity of ptrs */
    if (dentry == NULL) {
        return -1;
    }

    /* check index against bounds; if outside, return failure */
    if (index < 0 || index >= bblock.n_dir_entries) {
        return -1;
    }

    /* create pointers */
    name_ptr = (int8_t *) filesys_ptr;
    dentry_ptr = filesys_ptr + (DENTRY_SIZE/4)*(index + 1);

    /* copy string at current directory entry to open dentry struct */
    str_length = strlen((name_ptr + DENTRY_SIZE*(index + 1)));
    temp = strncpy((int8_t*) dentry->filename, name_ptr + DENTRY_SIZE*(index + 1), (int32_t) str_length);
    /* zero pad name */
    for (name_it = str_length; name_it < NAME_LEN; name_it++) {
        dentry->filename[name_it] = 0;
    }
    /* copy file type to open_dentry */
    dentry->filetype = *(dentry_ptr + FN_OFFSET);
    /* ignore inode numbers for non-regular files (filetype = 0, 1); else copy to open_dentry */
    if (dentry->filetype == REG_FILE) {
        dentry->inode_num = *(dentry_ptr + FT_OFFSET);
    }
    else {
        dentry->inode_num = 0;
    }

    /* return success */
    return 0;
}

/*
 * read_data
 * INPUTS: inode:   inode number for the file
 *         offset:  byte offset from current data block
 *         buf:     buffer array to copy bytes to
 *         length:  number of bytes to copy to buf
 * OUTPUTS: none
 * RETURN VALUE:    -1 if failure to read file
 *                  0 upon completion of reading entire file
 *                  else, returns the number of bytes read
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length, int32_t fd) {
    uint32_t file_size;                     /* size of file being read                                          */
    uint32_t dnode_num;                     /* current data block number                                        */
    uint32_t* dnode_ptr;                    /* pointer to current data block                                    */
    uint8_t* byte_ptr;                      /* pointer to current byte in data current data block               */
    uint32_t inode_idx;                     /* idx of current datablock num in inode, saved file desc flags     */
    uint32_t it;                            /* iterator for buffer operations                                   */

    /* check validity of ptrs */
    if (buf == NULL) {
      return -1;
    }
    /* check the validity of inode val */
    if (inode < 0 || inode >= bblock.N) {
      return -1;
    }

    /* get the filesize of the inode */
    file_size = *(filesys_ptr + (FOUR_KB/4)*(inode + 1));
    /* return success if at end of file w.r.t number of bytes read*/
    if (offset >= file_size) {
        return 0;
    }

    /* get current index in inode for data block indexing; return success if at end of inode block */
    if (fd != -1){
		inode_idx = ((cur_pcb->file_array)[fd].flags & I_IDX_MASK) >> I_IDX_SHIFT;
	}
	else{
		inode_idx = 0;
	}
    if (inode_idx >= FOUR_KB/4) {
        return 0;
    }

    /* get dnode_num */
    dnode_num = *(filesys_ptr + (FOUR_KB/4)*(inode + 1) + inode_idx);
    /* use it to get byte ptr */
    dnode_ptr = filesys_ptr + (FOUR_KB/4)*(bblock.N + dnode_num + 1);
    byte_ptr = (uint8_t*) dnode_ptr + (offset % FOUR_KB);

    /* copy length bytes to buffer */
    for (it = 0; it < length; it++, offset++, byte_ptr++) {
        /* assert we haven't read all bytes */
        if (offset >= file_size) {
            buf[it] = 0;
            return it;
        }
        /* if file_pos is a multiple of 4kB, must go to next data block */
        /* skip checking if at position 0 */
        if ((offset != 0) && (offset % FOUR_KB == 0)) {
            /* reset offset and increment index in inode */
            //cur_offset = 0;
            inode_idx++;
            /* if at end of inode, return */
            if (inode_idx >= FOUR_KB/4) {
                buf[it] = 0;
                return it;
            }
            /* update current dnode number and dnode_ptr */
            dnode_num = *(filesys_ptr + (FOUR_KB/4)*(inode + 1) + inode_idx);
            dnode_ptr = filesys_ptr + (FOUR_KB/4)*(bblock.N + dnode_num + 1);
            /* update byte ptr to start of next data block*/
            byte_ptr = (uint8_t*) dnode_ptr;
        }
        /* while current byte denotes end of data block */
        /* also checks if dnode_num is valid in while conditions */
        while (dnode_num < 0 || dnode_num >= bblock.D) {
            /* reset offset and increment index in inode */
            //cur_offset = 0;
            inode_idx++;
            /* if at end of inode, return */
            if (inode_idx >= FOUR_KB/4) {
                buf[it] = 0;
                return it;
            }
            /* update current dnode number and dnode_ptr */
            dnode_num = *(filesys_ptr + (FOUR_KB/4)*(inode + 1) + inode_idx);
            dnode_ptr = filesys_ptr + (FOUR_KB/4)*(bblock.N + dnode_num + 1);
            /* update byte ptr to start of next data block*/
            byte_ptr = (uint8_t*) dnode_ptr;
        }
        /* copy to buf once byte is valid */
        buf[it] = *byte_ptr;
    }
    /* set end charcter for strings for it if numbytes requested is less than size of buf*/
	if(it < length){
		buf[it] = 0;
	}

    /* update file_pos and inode_idx flag */
	if (fd != -1){
		(cur_pcb->file_array)[fd].flags &= I_IDX_CLEAR;
		(cur_pcb->file_array)[fd].flags |= inode_idx << I_IDX_SHIFT;
	}

    return length;
}

/*
 * file_read
 * DESCRIPTION: read operation for file system
 * INPUTS: buf - buffer array
 * OUTPUTS: none
 * RETURN VALUE: -1 if invalid, 0 if success
 */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes) {
    int32_t ret_val;

    /* check validity of ptrs */
    if (buf == NULL) {
        return -1;
    }

    /* check if a file is open and is of regular type */
    if ((((cur_pcb->file_array)[fd].flags & USE_MASK) == 0) || ((((cur_pcb->file_array)[fd].flags & TYPE_MASK) >> TYPE_SHIFT) != REG_FILE)) {
        return -1;
    }

    ret_val = read_data((cur_pcb->file_array)[fd].inode, (cur_pcb->file_array)[fd].file_pos, buf, nbytes, fd);
    (cur_pcb->file_array)[fd].file_pos += ret_val;
    return ret_val;
}

/*
 * file_write
 * DESCRIPTION: write operation for file system
 * INPUTS: fd:  index of file to write to (unused in CP2)
 *         buf: source of data to write
 *         nbytes: how many bytes of data to write to file
 * OUTPUTS: none
 * RETURN VALUE: -1 for read-only
 */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/*
 * file_open
 * DESCRIPTION: open operation for file system
 * INPUTS: filename - the name of the file
 * OUTPUTS: none
 * RETURN VALUE:    -1 on failure
 *                  0 otherwise
 */
int32_t file_open (const uint8_t* filename) {
    uint32_t i;

	/* Find an empty file descriptor (should correspond to same one found in sys_open) */
	for(i = 2; i < 8; i++){
		// Check each entry in the file array for an unused entry
		if((cur_pcb->file_array)[i].flags % 2 == 0){
			break;
		}
	}
	/* No empty file descriptor found */
	if (i >= 8) {
		return -1;
	}
    /* assert read_dentry_by_name is successful */
    if (read_dentry_by_name(filename, &open_dentry) == -1) {
        return -1;
    }

    /* assert file is of regular type */
    if (open_dentry.filetype != REG_FILE) {
        return -1;
    }

    (cur_pcb->file_array)[i].inode = open_dentry.inode_num;        // set inode num
    (cur_pcb->file_array)[i].file_pos = 0;                         // start at oth byte
    (cur_pcb->file_array)[i].flags = 0x00000001;                   // mark as in use
    (cur_pcb->file_array)[i].flags |= REG_FILE << TYPE_SHIFT;      // mark as reg type
    (cur_pcb->file_array)[i].flags |= 0x1 << I_IDX_SHIFT;          // set inode idx to 1 (indexes to first data block in inode)

    return 0;
}

/*
 * file_close
 * DESCRIPTION: close operation for file system
 * INPUTS: fd:  index of file to close (unused in CP2)
 * OUTPUTS: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t file_close (int32_t fd) {
    if (fd <= 1 || fd >= 8) {
        return -1;
    }

    if ((((cur_pcb->file_array)[fd].flags & TYPE_MASK) >> TYPE_SHIFT) != REG_FILE) {
        return -1;
    }

    if((cur_pcb->file_array)[fd].flags % 2 == 0){
        return -1;
    }

    (cur_pcb->file_array)[fd].flags = 0;
	int i;
	// Clear open and read dentry
	for(i = 0; i < 32; i++){
		open_dentry.filename[i] = 0;
		read_dentry.filename[i] = 0;
	}
    return 0;
}

/*
 * dir_read
 * DESCRIPTION: read operation for directory files
 * INPUTS: fd:  index of file to read (unused in CP2)
 *         buf: where to write data to
 *         nbytes: how many bytes of data to write to buf
 * OUTPUTS: none
 * RETURN VALUE: number of bytes written to buffer,
 *              0 if whole directory has been read,
 *              or -1 if fails to read file
 */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes) {
    int32_t cnt = 0;
    uint8_t* buf_ptr = (uint8_t*) buf;

    int32_t dr_buf_idx = 0;             /* current buffer index for dir_read function           */

    /* check validity of ptrs */
    if (buf == NULL) {
        return -1;
    }
    /* bounds check file directory */
    if (fd <= 1 || fd >= 8) {
        return -1;
    }

    /* assert dir is in use (for CP2 just check if struct is in use and of dir filetype) */
    if ((((cur_pcb->file_array)[fd].flags & USE_MASK) == 0) || (((cur_pcb->file_array)[fd].flags >> 1) % 2 == 0)) {
        return -1;
    }

    /* of at end of directory, done */
    if ((cur_pcb->file_array)[fd].file_pos >= bblock.n_dir_entries) {
        return 0;
    }

    /* read file name */
    if (read_dentry_by_index((cur_pcb->file_array)[fd].file_pos, &read_dentry) == -1) {
        return -1;
    }

    while ((cnt < nbytes) && (dr_buf_idx < NAME_LEN)) {
        /* break if at end of filename */
        if (read_dentry.filename[dr_buf_idx] == 0) {
            break;
        }
        /* else copy to buffer and increment cnt, dr_buf_idx */
        buf_ptr[dr_buf_idx] = read_dentry.filename[dr_buf_idx];
        dr_buf_idx++;
        cnt++;
    }

    /* if at end of filename or at max buffer length, reset and return done copying */
    if ((dr_buf_idx >= NAME_LEN) || (read_dentry.filename[dr_buf_idx] == 0)) {
        buf_ptr[dr_buf_idx] = 0;
    }

    /* update file position and return number of bytes read */
    (cur_pcb->file_array)[fd].file_pos++;
    return cnt;
}

/*
 * dir_write
 * DESCRIPTION: write operation for directory files
 * INPUTS: fd:  index of file to write to (unused in CP2)
 *         buf: source of data to write
 *         nbytes: how many bytes of data to write to file
 * OUTPUTS: none
 * RETURN VALUE: -1; always fails because this is a read-only file system
 */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/*
 * dir_open
 * DESCRIPTION: open operation for directory files
 * INPUTS: filename: name of file to open
 * OUTPUTS: none
 * RETURN VALUE: 0 if successful, else -1
 */
int32_t dir_open (const uint8_t* filename) {
    /* check validity of ptrs */
    if (filename == NULL) {
        return -1;
    }

	// Find an empty file descriptor
	int i, file_idx;
	file_idx = -1;
	for(i = 2; i<8;i++){
		// Check each entry in the file array for an unused entry
		if((cur_pcb->file_array)[i].flags % 2 == 0){
			file_idx = i;
			break;
		}
	}
	// No empty file descriptor found
	if (file_idx == -1){
		return -1;
	}

    /* read dentry corresponding to filename; return failure if call fails */
    if (read_dentry_by_name(filename, &open_dentry) == -1) {
        return -1;
    }

    /* assert file is of directory type */
    if (open_dentry.filetype != DIR_FILE) {
        return -1;
    }

    (cur_pcb->file_array)[file_idx].inode = open_dentry.inode_num;
    (cur_pcb->file_array)[file_idx].file_pos = 0;                     // start at beginning
    (cur_pcb->file_array)[file_idx].flags = 1;                        // mark as in use
    (cur_pcb->file_array)[file_idx].flags = ((cur_pcb->file_array)[file_idx].flags) | (DIR_FILE << 1);    // mark as dir type

    return 0;
}

/*
 * dir_close
 * DESCRIPTION: close operation for directory files
 * INPUTS: fd:  index of file to close (unused in CP2)
 * OUTPUTS: none
 * RETURN VALUE: 0 if successful, else -1
 */
int32_t dir_close (int32_t fd) {
    /* bounds check file directory */
    if (fd <= 1 || fd >= 8) {
        return -1;
    }

    if ((((cur_pcb->file_array)[fd].flags & TYPE_MASK) >> TYPE_SHIFT) != DIR_FILE) {
        return -1;
    }

    /* assert there is an open file_desc */
    if ((cur_pcb->file_array)[fd].flags % 2 == 0) {
        return -1;
    }

    /* clear in-use flag to 0 */
    (cur_pcb->file_array)[fd].flags = 0;
    return 0;
}
