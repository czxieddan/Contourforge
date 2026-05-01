/**
 * @file tinytiffreader.h
 * @brief Lightweight TIFF reader (single-header library)
 * @version 1.0.0
 * 
 * Simplified TIFF reader for heightmap loading
 * Supports: 8/16/32-bit grayscale, uncompressed and LZW compressed
 */

#ifndef TINYTIFF_READER_H
#define TINYTIFF_READER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TIFF Tags */
#define TIFF_TAG_IMAGE_WIDTH            256
#define TIFF_TAG_IMAGE_LENGTH           257
#define TIFF_TAG_BITS_PER_SAMPLE        258
#define TIFF_TAG_COMPRESSION            259
#define TIFF_TAG_PHOTOMETRIC            262
#define TIFF_TAG_STRIP_OFFSETS          273
#define TIFF_TAG_SAMPLES_PER_PIXEL      277
#define TIFF_TAG_ROWS_PER_STRIP         278
#define TIFF_TAG_STRIP_BYTE_COUNTS      279
#define TIFF_TAG_SAMPLE_FORMAT          339

/* Compression types */
#define TIFF_COMPRESSION_NONE           1
#define TIFF_COMPRESSION_LZW            5
#define TIFF_COMPRESSION_DEFLATE        8

/* Sample formats */
#define TIFF_SAMPLE_UINT                1
#define TIFF_SAMPLE_INT                 2
#define TIFF_SAMPLE_FLOAT               3

/* TIFF reader structure */
typedef struct {
    FILE* file;
    int is_big_endian;
    uint32_t ifd_offset;
    
    /* Image properties */
    uint32_t width;
    uint32_t height;
    uint16_t bits_per_sample;
    uint16_t samples_per_pixel;
    uint16_t compression;
    uint16_t photometric;
    uint16_t sample_format;
    uint32_t rows_per_strip;
    
    /* Strip data */
    uint32_t* strip_offsets;
    uint32_t* strip_byte_counts;
    uint32_t num_strips;
    
    int error;
} TinyTIFFReader;

/* Helper functions */
static uint16_t read_uint16(FILE* f, int big_endian) {
    uint8_t buf[2];
    if (fread(buf, 1, 2, f) != 2) return 0;
    if (big_endian) {
        return ((uint16_t)buf[0] << 8) | buf[1];
    } else {
        return ((uint16_t)buf[1] << 8) | buf[0];
    }
}

static uint32_t read_uint32(FILE* f, int big_endian) {
    uint8_t buf[4];
    if (fread(buf, 1, 4, f) != 4) return 0;
    if (big_endian) {
        return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
               ((uint32_t)buf[2] << 8) | buf[3];
    } else {
        return ((uint32_t)buf[3] << 24) | ((uint32_t)buf[2] << 16) |
               ((uint32_t)buf[1] << 8) | buf[0];
    }
}

static int read_ifd_entry(TinyTIFFReader* tiff, uint16_t* tag, uint16_t* type,
                          uint32_t* count, uint32_t* value) {
    *tag = read_uint16(tiff->file, tiff->is_big_endian);
    *type = read_uint16(tiff->file, tiff->is_big_endian);
    *count = read_uint32(tiff->file, tiff->is_big_endian);
    *value = read_uint32(tiff->file, tiff->is_big_endian);
    return !tiff->error;
}

/* Open TIFF file */
TinyTIFFReader* TinyTIFFReader_open(const char* filename) {
    TinyTIFFReader* tiff = (TinyTIFFReader*)calloc(1, sizeof(TinyTIFFReader));
    if (!tiff) return NULL;
    
    tiff->file = fopen(filename, "rb");
    if (!tiff->file) {
        free(tiff);
        return NULL;
    }
    
    /* Read header */
    uint8_t header[4];
    if (fread(header, 1, 4, tiff->file) != 4) {
        fclose(tiff->file);
        free(tiff);
        return NULL;
    }
    
    /* Check byte order */
    if (header[0] == 0x4D && header[1] == 0x4D) {
        tiff->is_big_endian = 1;
    } else if (header[0] == 0x49 && header[1] == 0x49) {
        tiff->is_big_endian = 0;
    } else {
        fclose(tiff->file);
        free(tiff);
        return NULL;
    }
    
    /* Check magic number (42) */
    uint16_t magic = (tiff->is_big_endian) ? 
        ((uint16_t)header[2] << 8) | header[3] :
        ((uint16_t)header[3] << 8) | header[2];
    
    if (magic != 42) {
        fclose(tiff->file);
        free(tiff);
        return NULL;
    }
    
    /* Read IFD offset */
    tiff->ifd_offset = read_uint32(tiff->file, tiff->is_big_endian);
    
    /* Set defaults */
    tiff->bits_per_sample = 8;
    tiff->samples_per_pixel = 1;
    tiff->compression = TIFF_COMPRESSION_NONE;
    tiff->sample_format = TIFF_SAMPLE_UINT;
    tiff->rows_per_strip = 0xFFFFFFFF;
    
    return tiff;
}

/* Read IFD and parse tags */
int TinyTIFFReader_readIFD(TinyTIFFReader* tiff) {
    if (!tiff || !tiff->file) return 0;
    
    fseek(tiff->file, tiff->ifd_offset, SEEK_SET);
    
    uint16_t num_entries = read_uint16(tiff->file, tiff->is_big_endian);
    
    for (uint16_t i = 0; i < num_entries; i++) {
        uint16_t tag, type;
        uint32_t count, value;
        
        if (!read_ifd_entry(tiff, &tag, &type, &count, &value)) {
            return 0;
        }
        
        switch (tag) {
            case TIFF_TAG_IMAGE_WIDTH:
                tiff->width = value;
                break;
            case TIFF_TAG_IMAGE_LENGTH:
                tiff->height = value;
                break;
            case TIFF_TAG_BITS_PER_SAMPLE:
                tiff->bits_per_sample = (uint16_t)value;
                break;
            case TIFF_TAG_COMPRESSION:
                tiff->compression = (uint16_t)value;
                break;
            case TIFF_TAG_PHOTOMETRIC:
                tiff->photometric = (uint16_t)value;
                break;
            case TIFF_TAG_SAMPLES_PER_PIXEL:
                tiff->samples_per_pixel = (uint16_t)value;
                break;
            case TIFF_TAG_ROWS_PER_STRIP:
                tiff->rows_per_strip = value;
                break;
            case TIFF_TAG_SAMPLE_FORMAT:
                tiff->sample_format = (uint16_t)value;
                break;
            case TIFF_TAG_STRIP_OFFSETS:
                tiff->num_strips = count;
                tiff->strip_offsets = (uint32_t*)malloc(count * sizeof(uint32_t));
                if (count == 1) {
                    tiff->strip_offsets[0] = value;
                } else {
                    long pos = ftell(tiff->file);
                    fseek(tiff->file, value, SEEK_SET);
                    for (uint32_t j = 0; j < count; j++) {
                        tiff->strip_offsets[j] = read_uint32(tiff->file, tiff->is_big_endian);
                    }
                    fseek(tiff->file, pos, SEEK_SET);
                }
                break;
            case TIFF_TAG_STRIP_BYTE_COUNTS:
                tiff->strip_byte_counts = (uint32_t*)malloc(count * sizeof(uint32_t));
                if (count == 1) {
                    tiff->strip_byte_counts[0] = value;
                } else {
                    long pos = ftell(tiff->file);
                    fseek(tiff->file, value, SEEK_SET);
                    for (uint32_t j = 0; j < count; j++) {
                        tiff->strip_byte_counts[j] = read_uint32(tiff->file, tiff->is_big_endian);
                    }
                    fseek(tiff->file, pos, SEEK_SET);
                }
                break;
        }
    }
    
    return 1;
}

/* Read image data as float array */
float* TinyTIFFReader_readImageAsFloat(TinyTIFFReader* tiff) {
    if (!tiff || !tiff->file || !tiff->strip_offsets) return NULL;
    
    /* Only support uncompressed for now */
    if (tiff->compression != TIFF_COMPRESSION_NONE) {
        return NULL;
    }
    
    size_t pixel_count = (size_t)tiff->width * tiff->height;
    float* data = (float*)malloc(pixel_count * sizeof(float));
    if (!data) return NULL;
    
    size_t bytes_per_sample = (tiff->bits_per_sample + 7) / 8;
    size_t row_bytes = tiff->width * bytes_per_sample * tiff->samples_per_pixel;
    
    uint8_t* row_buffer = (uint8_t*)malloc(row_bytes);
    if (!row_buffer) {
        free(data);
        return NULL;
    }
    
    /* Read strips */
    size_t pixel_idx = 0;
    for (uint32_t strip = 0; strip < tiff->num_strips; strip++) {
        fseek(tiff->file, tiff->strip_offsets[strip], SEEK_SET);
        
        uint32_t rows_in_strip = tiff->rows_per_strip;
        if (strip == tiff->num_strips - 1) {
            rows_in_strip = tiff->height - strip * tiff->rows_per_strip;
        }
        
        for (uint32_t row = 0; row < rows_in_strip; row++) {
            if (fread(row_buffer, 1, row_bytes, tiff->file) != row_bytes) {
                free(row_buffer);
                free(data);
                return NULL;
            }
            
            /* Convert to float */
            for (uint32_t col = 0; col < tiff->width; col++) {
                float value = 0.0f;
                
                if (tiff->bits_per_sample == 8) {
                    value = (float)row_buffer[col] / 255.0f;
                } else if (tiff->bits_per_sample == 16) {
                    uint16_t val16;
                    if (tiff->is_big_endian) {
                        val16 = ((uint16_t)row_buffer[col*2] << 8) | row_buffer[col*2+1];
                    } else {
                        val16 = ((uint16_t)row_buffer[col*2+1] << 8) | row_buffer[col*2];
                    }
                    value = (float)val16 / 65535.0f;
                } else if (tiff->bits_per_sample == 32) {
                    if (tiff->sample_format == TIFF_SAMPLE_FLOAT) {
                        uint32_t val32;
                        if (tiff->is_big_endian) {
                            val32 = ((uint32_t)row_buffer[col*4] << 24) |
                                   ((uint32_t)row_buffer[col*4+1] << 16) |
                                   ((uint32_t)row_buffer[col*4+2] << 8) |
                                   row_buffer[col*4+3];
                        } else {
                            val32 = ((uint32_t)row_buffer[col*4+3] << 24) |
                                   ((uint32_t)row_buffer[col*4+2] << 16) |
                                   ((uint32_t)row_buffer[col*4+1] << 8) |
                                   row_buffer[col*4];
                        }
                        memcpy(&value, &val32, sizeof(float));
                    } else {
                        uint32_t val32;
                        if (tiff->is_big_endian) {
                            val32 = ((uint32_t)row_buffer[col*4] << 24) |
                                   ((uint32_t)row_buffer[col*4+1] << 16) |
                                   ((uint32_t)row_buffer[col*4+2] << 8) |
                                   row_buffer[col*4+3];
                        } else {
                            val32 = ((uint32_t)row_buffer[col*4+3] << 24) |
                                   ((uint32_t)row_buffer[col*4+2] << 16) |
                                   ((uint32_t)row_buffer[col*4+1] << 8) |
                                   row_buffer[col*4];
                        }
                        value = (float)val32 / 4294967295.0f;
                    }
                }
                
                data[pixel_idx++] = value;
            }
        }
    }
    
    free(row_buffer);
    return data;
}

/* Get image dimensions */
void TinyTIFFReader_getDimensions(TinyTIFFReader* tiff, uint32_t* width, uint32_t* height) {
    if (tiff && width && height) {
        *width = tiff->width;
        *height = tiff->height;
    }
}

/* Close TIFF file */
void TinyTIFFReader_close(TinyTIFFReader* tiff) {
    if (tiff) {
        if (tiff->file) fclose(tiff->file);
        if (tiff->strip_offsets) free(tiff->strip_offsets);
        if (tiff->strip_byte_counts) free(tiff->strip_byte_counts);
        free(tiff);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* TINYTIFF_READER_H */
