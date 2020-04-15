# Sprite Format

* [Introduction](#introduction)
* [File Layout](#file-layout)
* [Data Representation](#data-representation)
* [Chunk Layout](#chunk-layout)
* [Standard Chunks](#standard-chunks)
  * [AHDR (Animation Header)](#ahdr-animation-header)
  * [PLTE (Palette)](#plte-palette)
  * [LHDR (Layer Header)](#plte-palette)
  * [CHDR (Cell Header)](#chdr-cell-header)
  * [CDAT (Cell Data)](#cdat-cell-data)
  * [AEND (Animation End)](#aend-animation-end)

## Introduction

The sprite format (`.animera`) is heavily inspired by the PNG format. This is 
because the PNG format is very resistant to error (as it was originally designed
to be sent over a network). If the file is corrupted somehow, the signature
might not match up, or the chunk names might be invalid, or the CRC check won't
pass. If a new chunk is added in the future, old decoders will warn about it. It
is extremely unlikely that a corrupted file will be read without emitting an 
error.

## File Layout

The file starts with an 8 byte signature.

| Byte | ASCII |
|------|-------|
| 65   | A     |
| 110  | n     |
| 105  | i     |
| 109  | m     |
| 101  | e     |
| 114  | r     |
| 97   | a     |
| 0    | \0    |

Following the signature are some number of chunks. The order of the chunks is
defined by the following pseudo-code.

```
AHDR
PLTE
for each layer:
  LHDR
  for each span in layer:
    CHDR
    if span is not null:
      CDAT
AEND
```

## Data Representation

To avoid repetition, I will define some standard data types that are used in the
definitions of standard chunks.

| Type   | Description                                      |
|--------|--------------------------------------------------|
| Byte   | 1 byte, unsigned                                 |
| Int    | 4 bytes, big-endian, signed (2s-complement)      |
| Uint   | 4 bytes, big-endian, unsigned                    |
| String | Sequence of printable ASCII characters [32, 126] |

See also: [PNG integer byte order](http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html#DR.Integers-and-byte-order)

## Chunk Layout

| Part   | Type               | Description                                                                                                      |
|--------|--------------------|------------------------------------------------------------------------------------------------------------------|
| Length | Uint               | The size of the "Data" part in bytes. This is not limited to 2^31 (as it is by PNG).                             |
| Name   | 4 ASCII characters | All standard chunk names are upper case. There is no meaning in the case of the characters (as there is in PNG). |
| Data   | "Length" bytes     | The contents of the chunk is defined by the particular chunk.                                                    |
| CRC    | Uint               | A CRC (Cyclic Redundancy Check) of the "Name" and "Data" parts as defined by zlib's `crc32` function.            |

See also: [PNG chunk layout](http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#Chunk-layout)

## Standard Chunks

### AHDR (Animation Header)

| Type | Description                               |
|------|-------------------------------------------|
| Int  | Width of the canvas in pixels [1, 32768]  |
| Int  | Height of the canvas in pixels [1, 32768] |
| Int  | Number of layers [1, 2147483647]          |
| Int  | Number of frames [1, 2147483647]          |
| Int  | Animation delay in milliseconds [1, 999]  |
| Byte | Pixel format (explained below)            |

The number of layers corresponds to the number of LHDR chunks that follow this
chunk. There are three valid values for the pixel format byte. These values
happen to be the byte depth of the pixel format.

| Value | Description |
|-------|-------------|
| 1     | Indexed     |
| 2     | Gray-alpha  |
| 4     | RGBA        |

### PLTE (Palette)

For indexed and RGBA sprites, the palette consists of RGBA entries. Each entry
is four bytes. Each of the four bytes correspond to the red, green, blue and
alpha channels respectively.

For gray-alpha sprites, the palette consists of gray-alpha entries. Each entry
is two bytes. Each of the two bytes correspond to the gray and alpha channels
respectively.

There can be no more than 256 palette entries. Trailing all-zero entries are
assumed if they're not present. In other words, if the last few entries are
all-zeros, they don't need to be written to the file. This is similar to the way
PNG optimizes its PLTE and tRNS chunks.

### LHDR (Layer Header)

| Type   | Description                                               |
|--------|-----------------------------------------------------------|
| Uint   | Number of spans in this layer [1, 4294967295]             |
| Byte   | Visibility of this layer (0 for invisible, 1 for visible) |
| String | Name of this layer [0, 256]                               |

Each layer contains a number of spans. The number of spans corresponds to the
number of CHDR chunks that follow this chunk.

### CHDR (Cell Header)

The format of the CHDR chunk depends on whether the span is null or not. 

If the span is non-null, the cell rectangle is stored. This is a rectangle on
the canvas where the cell is placed. All pixels outside of the rectangle are
considered zero.

| Type | Description                                  |
|------|----------------------------------------------|
| Int  | Number of cells in this span [1, 2147483647] |
| Int  | Cell X coordinate [-2147483648, 2147483647]  |
| Int  | Cell Y coordinate [-2147483648, 2147483647]  |
| Int  | Cell width [1, 1073741823]                   |
| Int  | Cell height [1, 1073741823]                  |

If the span is null, the cell rectangle is not stored (because there is no cell
to have a rectangle).

| Type | Description                                  |
|------|----------------------------------------------|
| Int  | Number of cells in this span [1, 2147483647] |

If the span is non-null, the following chunk will be a CDAT chunk.

### CDAT (Cell Data)

Cell data is compressed using zlib's `deflate` function at the default
compression level. Before being compressed, the pixels are written in the order
you would expect. That is, pixels are written from left to right into scanlines,
and scanlines are written from top to bottom. Scanlines are not filtered as they
are in PNG so scanlines do not begin with a filter-type byte.

The pixel format of the cell data is defined by the pixel format in the AHDR
chunk.

* Indexed

   Each pixel is a single byte that is an index into the palette.

* Gray-alpha

   Each pixel is two bytes that correspond to the gray and alpha channels
   respectively.

* RGBA

   Each pixel is four bytes that correspond to the red, green, blue and alpha
   channels respectively.

See also: [PNG IDAT chunk](http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IDAT)

### AEND (Animation End)

This chunk contains no data. It corresponds to the IEND chunk of PNG. It marks
the end of the file to ensure that the file hasn't been truncated.

See also: [PNG IEND chunk](http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.IEND)
