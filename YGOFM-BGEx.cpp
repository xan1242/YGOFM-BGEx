// Yu-Gi-Oh! Forbidden Memories background image extractor
// by Xan / Tenjoin

// TODO - add support for type 1 and 2
// TODO - add other versions
// TODO - maybe add support for LBA calculation of other content
// TODO - maybe add support for repacking

#include <iostream>
#include <string>
#include "DDS.h"

//
// LBAs START
//

// based on SLUS-01411, change accordingly for your game
// these were ripped from the function 0x8002DF2C in the exe of SLUS-01411
constexpr unsigned int backgroundsLBA_allstart = 0x21D5;

constexpr unsigned int backgroundsLBA_start_0 = 0;
constexpr unsigned int backgroundsLBA_size_0 = 0x21;

constexpr unsigned int backgroundsLBA_start_1 = 0x672;
constexpr unsigned int backgroundsLBA_size_1 = 0x51;

constexpr unsigned int backgroundsLBA_start_2 = 0x13BC;
constexpr unsigned int backgroundsLBA_size_2 = 0x71;


unsigned long calclba(int BGindex, unsigned long &out_size, int &out_type)
{
    int v2;
    int v3;
    int sizeLBA = backgroundsLBA_size_0;
    int startLBA = backgroundsLBA_start_0;

    v2 = 10 * ((BGindex >> 4) & 0xF) + (BGindex & 0xF);
    v3 = BGindex >> 8;

    switch (v3)
    {
    case 2:
        sizeLBA = backgroundsLBA_size_2;
        startLBA = backgroundsLBA_start_2;
        break;
    case 1:
        sizeLBA = backgroundsLBA_size_1;
        startLBA = backgroundsLBA_start_1;
        break;
    }
    out_size = sizeLBA;
    out_type = v3;

    return startLBA + v2 * sizeLBA + backgroundsLBA_allstart;
}

//
// LBAs END
//

void DecodePS1ImageP8(uint8_t* inIndicies, uint16_t* inPalette, uint32_t* outARGB, size_t size, bool bForceBlackTransparency)
{
    for (int i = 0; i < size; i++)
    {
        uint16_t color = inPalette[inIndicies[i]];
        uint32_t rgba = 0;

        if ((color & 0x7FFF) == 0)
        {
            if ((color & 0x8000) || bForceBlackTransparency)
                rgba = 0;
            else
                rgba = 0xFF000000;
        }
        else
        {
            uint8_t r = color & 0x1F;
            uint8_t g = (color & 0x3E0) >> 5;
            uint8_t b = (color & 0x7C00) >> 10;


            float fB = (float)b / 31.0f;
            float fG = (float)g / 31.0f;
            float fR = (float)r / 31.0f;

            uint8_t b8 = (uint8_t)(fB * 255.0f);
            uint8_t g8 = (uint8_t)(fG * 255.0f);
            uint8_t r8 = (uint8_t)(fR * 255.0f);
            uint8_t a8 = 0xFF;

            if ((color & 0x8000))
            {
                a8 = (r8 + g8 + b8) / 3;
            }

            rgba |= b8 | (g8 << 8) | (r8 << 16) | (a8 << 24);
        }

        outARGB[i] = rgba;
    }
}

void DecodePS1ImageP4(uint8_t* inIndicies, uint16_t* inPalette, uint32_t* outARGB, size_t size, bool bForceBlackTransparency)
{
    int j = 0;

    for (int i = 0; i < size; i++)
    {
        uint8_t index = inIndicies[j];
        // every other time get other 4 bits and increment
        if (i & 1)
        {
            index = (index & 0xF0) >> 4;
            j++;
        }
        else
            index &= 0xF;

        uint16_t color = inPalette[index];
        uint32_t rgba = 0;

        if ((color & 0x7FFF) == 0)
        {
            if ((color & 0x8000) || bForceBlackTransparency)
                rgba = 0;
            else
                rgba = 0xFF000000;
        }
        else
        {
            uint8_t r = color & 0x1F;
            uint8_t g = (color & 0x3E0) >> 5;
            uint8_t b = (color & 0x7C00) >> 10;


            float fB = (float)b / 31.0f;
            float fG = (float)g / 31.0f;
            float fR = (float)r / 31.0f;

            uint8_t b8 = (uint8_t)(fB * 255.0f);
            uint8_t g8 = (uint8_t)(fG * 255.0f);
            uint8_t r8 = (uint8_t)(fR * 255.0f);
            uint8_t a8 = 0xFF;

            if ((color & 0x8000))
            {
                a8 = (r8 + g8 + b8) / 3;
            }

            rgba |= b8 | (g8 << 8) | (r8 << 16) | (a8 << 24);
        }

        outARGB[i] = rgba;
    }
}

// Untiles a 128x512 buffer to a 320x160 image
void* UntileImage(void* inBuffer, unsigned int imgUntileWidth, unsigned int imgUntileHeight, size_t imgSize)
{
    // parts in order of visual presentation in the tiled version
            // 1 - 128x160
            // 5 - 128x96 -- ignored
            // 2 - 128x160
            // 3 - 64x80, 4 - 64x80
            // 6 - 128x16 -- ignored

    uintptr_t cursor = 0;
    uint32_t* output_colors = (uint32_t*)(inBuffer);

    size_t imgUntileSize = imgUntileWidth * imgUntileHeight;

    constexpr unsigned int part1width = 128;
    constexpr unsigned int part1height = 160;
    constexpr unsigned int part1size = (part1width * part1height);
    void* part1 = malloc(part1size * 4);
    memcpy(part1, inBuffer, part1size * 4);
    uint32_t* rgba_part1 = (uint32_t*)(part1);

    constexpr unsigned int part2width = 128;
    constexpr unsigned int part2height = 160;
    constexpr unsigned int part2size = (part2width * part2height);
    uintptr_t part2start = imgSize / 2;
    void* part2 = malloc(part2size * 4);
    memcpy(part2, &(output_colors[part2start]), part2size * 4);
    uint32_t* rgba_part2 = (uint32_t*)(part2);

    constexpr unsigned int part3width = 64;
    constexpr unsigned int part3height = 80;
    constexpr unsigned int part3size = (part3width * part3height);
    uintptr_t part3start = part2start + part2size;
    void* part3 = malloc(part3size * 4);
    uint32_t* rgba_part3 = (uint32_t*)(part3);

    constexpr unsigned int part4width = 64;
    constexpr unsigned int part4height = 80;
    constexpr unsigned int part4size = (part4width * part4height);
    uintptr_t part4start = part3start + (part3width);
    void* part4 = malloc(part4size * 4);
    uint32_t* rgba_part4 = (uint32_t*)(part4);

    // copy half width images
    cursor = part3start;
    for (int i = 0; i < part3size; i++)
    {
        rgba_part3[i] = output_colors[(cursor)];

        cursor++;
        if (!(cursor % part3width))
            cursor += part4width;
    }

    cursor = part4start;
    for (int i = 0; i < part4size; i++)
    {
        rgba_part4[i] = output_colors[(cursor)];

        cursor++;
        if (!(cursor % (part3width + part4width)))
            cursor += part3width;
    }

    // stitch image together

    void* new_output_buffer = malloc(imgUntileSize * 4);
    uintptr_t new_output_buffer_ptr = (uintptr_t)new_output_buffer;
    uint32_t* rgba_new = (uint32_t*)(new_output_buffer);

    unsigned int part3heightsize = part3height * imgUntileWidth;

    int p1 = 0;
    int p2 = 0;
    int p3 = 0;
    int p4 = 0;

    memset(new_output_buffer, 0, imgUntileSize * 4);
    for (int i = 0; i < imgUntileSize; i++)
    {
        int pos = i % imgUntileWidth;

        if (pos < part1width)
        {
            rgba_new[i] = rgba_part1[p1];
            p1++;
        }

        if ((pos >= (part1width)) && (pos < (part1width + part2width)))
        {
            rgba_new[i] = rgba_part2[p2];
            p2++;
        }

        if (i < part3heightsize)
        {
            if ((pos >= (part1width + part2width)) && (pos < (part1width + part2width + part3width)))
            {
                rgba_new[i] = rgba_part3[p3];
                p3++;
            }
        }
        else
        {
            if ((pos >= (part1width + part2width)) && (pos < (part1width + part2width + part4width)))
            {
                rgba_new[i] = rgba_part4[p4];
                p4++;
            }
        }
    }

    free(part4);
    free(part3);
    free(part2);
    free(part1);

    free(inBuffer);
    return new_output_buffer;
}

// Untiles a 256x256 buffer to a 320x160 image
void* UntileImage256(void* inBuffer, unsigned int imgUntileWidth, unsigned int imgUntileHeight, size_t imgSize)
{
    // parts in order of visual presentation in the tiled version (from left to right)
            // 1 - 256x160
            // 4 - 128x96 -- ignored, 2 - 64x80, 3 - 64x80
            // 5 - 128x16 -- ignored

    uintptr_t cursor = 0;
    uint32_t* output_colors = (uint32_t*)(inBuffer);

    size_t imgUntileSize = imgUntileWidth * imgUntileHeight;

    constexpr unsigned int part1width = 256;
    constexpr unsigned int part1height = 160;
    constexpr unsigned int part1size = (part1width * part1height);
    void* part1 = malloc(part1size * 4);
    memcpy(part1, inBuffer, part1size * 4);
    uint32_t* rgba_part1 = (uint32_t*)(part1);

    constexpr unsigned int part4width = 128;

    constexpr unsigned int part2width = 64;
    constexpr unsigned int part2height = 80;
    constexpr unsigned int part2size = (part2width * part2height);
    uintptr_t part2start = (part1height * part1width) + part4width;
    void* part2 = malloc(part2size * 4);
    uint32_t* rgba_part2 = (uint32_t*)(part2);

    constexpr unsigned int part3width = 64;
    constexpr unsigned int part3height = 80;
    constexpr unsigned int part3size = (part3width * part3height);
    uintptr_t part3start = part2start + part2width;
    void* part3 = malloc(part3size * 4);
    uint32_t* rgba_part3 = (uint32_t*)(part3);

    cursor = part2start;

    for (int i = 0; i < part2size; i++)
    {
        rgba_part2[i] = output_colors[(cursor)];

        cursor++;
        if (!(cursor % part2width))
            cursor += part3width + part4width;
    }

    cursor = part3start;
    for (int i = 0; i < part3size; i++)
    {
        rgba_part3[i] = output_colors[(cursor)];

        cursor++;
        if (!(cursor % part3width))
            cursor += part4width + part2width;
    }

    // stitch image together

    void* new_output_buffer = malloc(imgUntileSize * 4);
    uintptr_t new_output_buffer_ptr = (uintptr_t)new_output_buffer;
    uint32_t* rgba_new = (uint32_t*)(new_output_buffer);

    unsigned int part2heightsize = part2height * imgUntileWidth;

    int p1 = 0;
    int p2 = 0;
    int p3 = 0;

    memset(new_output_buffer, 0, imgUntileSize * 4);
    for (int i = 0; i < imgUntileSize; i++)
    {
        int pos = i % imgUntileWidth;

        if (pos < part1width)
        {
            rgba_new[i] = rgba_part1[p1];
            p1++;
        }

        if (i < part2heightsize)
        {
            if ((pos >= (part1width)) && (pos < (part1width + part2width)))
            {
                rgba_new[i] = rgba_part2[p2];
                p2++;
            }
        }
        else
        {
            if ((pos >= (part1width)) && (pos < (part1width + part3width)))
            {
                rgba_new[i] = rgba_part3[p3];
                p3++;
            }
        }
    }

    free(part3);
    free(part2);
    free(part1);

    free(inBuffer);
    return new_output_buffer;
}

// Untiles a 128x512 buffer to a 256x256 image
void* UntileImage256x256(void* inBuffer, unsigned int imgUntileWidth, unsigned int imgUntileHeight, size_t imgSize)
{
    // parts in order of visual presentation in the tiled version
            // 1 - 128x256
            // 2 - 128x256

    uintptr_t cursor = 0;
    uint32_t* output_colors = (uint32_t*)(inBuffer);

    size_t imgUntileSize = imgUntileWidth * imgUntileHeight;

    constexpr unsigned int part1width = 128;
    constexpr unsigned int part1height = 256;
    constexpr unsigned int part1size = (part1width * part1height);
    void* part1 = malloc(part1size * 4);
    memcpy(part1, inBuffer, part1size * 4);
    uint32_t* rgba_part1 = (uint32_t*)(part1);

    constexpr unsigned int part2width = 128;
    constexpr unsigned int part2height = 256;
    constexpr unsigned int part2size = (part2width * part2height);
    uintptr_t part2start = imgSize / 2;
    void* part2 = malloc(part2size * 4);
    memcpy(part2, &(output_colors[part2start]), part2size * 4);
    uint32_t* rgba_part2 = (uint32_t*)(part2);

    // stitch image together

    void* new_output_buffer = malloc(imgUntileSize * 4);
    uintptr_t new_output_buffer_ptr = (uintptr_t)new_output_buffer;
    uint32_t* rgba_new = (uint32_t*)(new_output_buffer);

    int p1 = 0;
    int p2 = 0;

    memset(new_output_buffer, 0, imgUntileSize * 4);
    for (int i = 0; i < imgUntileSize; i++)
    {
        int pos = i % imgUntileWidth;

        if (pos < part1width)
        {
            rgba_new[i] = rgba_part1[p1];
            p1++;
        }

        if ((pos >= (part1width)) && (pos < (part1width + part2width)))
        {
            rgba_new[i] = rgba_part2[p2];
            p2++;
        }
    }

    free(part2);
    free(part1);

    free(inBuffer);
    return new_output_buffer;
}

void WriteImage(std::string filename, int index, int subindex, void* buffer, unsigned int width, unsigned int height)
{
    // create dds
    struct DirectX::DDS_HEADER ddshs = { 0 };
    struct DirectX::DDS_PIXELFORMAT ddspfs = { 0 };
    ddshs.dwSize = 124;
    ddshs.dwFlags = 0x21007;

    ddshs.dwWidth = width;
    ddshs.dwHeight = height;

    ddshs.dwMipMapCount = 0;
    ddspfs.dwSize = 32;
    ddspfs.dwFlags = 0x41;
    ddspfs.dwRGBBitCount = 0x20;
    ddspfs.dwRBitMask = 0xFF0000;
    ddspfs.dwGBitMask = 0xFF00;
    ddspfs.dwBBitMask = 0xFF;
    ddspfs.dwABitMask = 0xFF000000;
    ddshs.dwCaps = 0x40100A;
    ddshs.ddspf = ddspfs;
    constexpr unsigned int DDSMagic = 0x20534444;

    size_t writeSize = (width * height) * sizeof(uint32_t);


    std::string outFilename = filename + "_" + std::to_string(index) + "_" + std::to_string(subindex) + ".out.dds";
    std::cout << "Writing: " << outFilename << '\n';

    FILE* fout = fopen(outFilename.c_str(), "wb");
    if (!fout)
    {
        std::cout << "ERROR: couldn't open file for writing: " << outFilename << '\n';
        perror("ERROR");
        return;
    }

    fwrite(&DDSMagic, 4, 1, fout);
    fwrite(&ddshs, sizeof(ddshs), 1, fout);
    fwrite(buffer, sizeof(uint8_t), writeSize, fout);

    fclose(fout);
}

// Starts the extraction process of background images
// filename - filename of WA_MRG.MRG
// index - image index
// bUntile - untile flag
// TransparencyMode - 0 = keep original data, 1 = black is transparent for subimages 0 on type 1 and 2 on type 2, 2 = black is always transparent
void ExtractBGToDDS(std::string filename, int index, bool bUntile = true, uint32_t TransparencyMode = 1)
{
    std::cout << "Opening: " << filename << '\n';

    FILE* fin = fopen(filename.c_str(), "rb");
    if (!fin)
    {
        std::cout << "ERROR: couldn't open file for reading: " << filename << '\n';
        perror("ERROR");
        return;
    }
    
    unsigned long lbasize = 0;
    unsigned long size = 0;
    int type = 0;
    unsigned long lba = calclba(index, lbasize, type);

    unsigned int imgWidth = 0;
    unsigned int imgHeight = 0;
    unsigned int imgSize = 0;
    unsigned int imgWriteSize = 0;

    constexpr unsigned int imgUntileWidth = 320;
    constexpr unsigned int imgUntileHeight = 160;
    constexpr unsigned int imgUntileSize = imgUntileWidth * imgUntileHeight;

    constexpr unsigned int imgUntileWidth2 = 256;
    constexpr unsigned int imgUntileHeight2 = 256;
    constexpr unsigned int imgUntileSize2 = imgUntileWidth2 * imgUntileHeight2;
    constexpr unsigned int imgWriteSize2 = imgUntileSize2 * 4;

    constexpr unsigned int simgWidth = 256;
    constexpr unsigned int simgHeight = 256;
    constexpr unsigned int simgSize = simgWidth * simgHeight;
    constexpr unsigned int simgHalfSize = simgSize / 2;

    constexpr unsigned int simgUntileWidth = 320;
    constexpr unsigned int simgUntileHeight = 160;
    constexpr unsigned int simgUntileSize = simgUntileWidth * simgUntileHeight;
    unsigned int simgWriteSize = 0;

    unsigned int imgCount = 1;
    unsigned int simgCount = 0;
    constexpr unsigned int palleteSize256 = 256 * sizeof(uint16_t);
    constexpr unsigned int palleteSize16 = 16 * sizeof(uint16_t);

    switch (type)
    {
    case 2:
        imgCount = 3;
        simgCount = 1;
        simgWriteSize = simgSize * 4;
        if (bUntile)
            simgWriteSize = simgUntileSize * 4;

        imgWidth = 128;
        imgHeight = 512;
        break;
    case 1:
        imgCount = 2;
        simgCount = 1;
        simgWriteSize = simgSize * 4;
        if (bUntile)
            simgWriteSize = simgUntileSize * 4;

        imgWidth = 128;
        imgHeight = 512;
        break;
    default:
        imgWidth = 128;
        imgHeight = 512;
        break;
    }

    imgSize = imgWidth * imgHeight;
    imgWriteSize = imgSize * 4;
    if (bUntile) imgWriteSize = imgUntileSize * 4;
    size = lbasize * 0x800;

    std::cout <<
    "LBA: 0x" << std::uppercase << std::hex << lba << "\n"
    "SizeLBA: 0x" << std::uppercase << std::hex << lbasize << "\n"
    "Offset: 0x" << std::uppercase << std::hex << lba * 0x800 << "\n"
    "Size: 0x" << std::uppercase << std::hex << size << "\n";

    void* input_buffer = malloc(size);

    fseek(fin, lba * 0x800, SEEK_SET);
    fread(input_buffer, size, 1, fin);
    fclose(fin);

    // extract 320x160 images
    for (int c = 0; c < imgCount; c++)
    {
        uint16_t* palette = nullptr;
        uint8_t* indicies = (uint8_t*)((uintptr_t)input_buffer + (imgSize * c));

        if (type == 1)
        {
            palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * simgCount) + (palleteSize256 * c));
        }
        else if (type == 2)
        {
            if (c == 2) // 3rd image has its own palette
                palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * simgCount) + (palleteSize256 * (c - 1)));
            else
                palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * simgCount));
        }
        else
        {
            palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (palleteSize256 * c));
        }

        void* output_buffer = malloc(imgSize * 4);
        bool bTransparencyFlag = false;

        if (TransparencyMode == 1)
            bTransparencyFlag = ((c == 0) && ((type == 1)) || (c == 2) && ((type == 2)));
        if (TransparencyMode == 2)
            bTransparencyFlag = true;

        DecodePS1ImageP8(indicies, palette, (uint32_t*)(output_buffer), imgSize, bTransparencyFlag);

        if (bUntile)
        {
            if (type == 2)
            {
                switch (c)
                {
                case 2:
                    output_buffer = UntileImage(output_buffer, imgUntileWidth, imgUntileHeight, imgSize);
                    WriteImage(filename, index, c, output_buffer, imgUntileWidth, imgUntileHeight);
                    break;
                case 1:
                    output_buffer = UntileImage256x256(output_buffer, imgUntileWidth2, imgUntileHeight2, imgSize);
                    WriteImage(filename, index, c, output_buffer, imgUntileWidth2, imgUntileHeight2);
                    break;
                case 0:
                    output_buffer = UntileImage256x256(output_buffer, imgUntileWidth2, imgUntileHeight2, imgSize);
                    WriteImage(filename, index, c, output_buffer, imgUntileWidth2, imgUntileHeight2);
                    break;
                default:
                    break;
                }
            }
            else
            {
                output_buffer = UntileImage(output_buffer, imgUntileWidth, imgUntileHeight, imgSize);
                WriteImage(filename, index, c, output_buffer, imgUntileWidth, imgUntileHeight);
            }
            
        }
        else
            WriteImage(filename, index, c, output_buffer, imgWidth, imgHeight);
            
        free(output_buffer);
    }

    // extract secondary images
    if (type)
    {
        for (int c = 0; c < simgCount; c++)
        {
            uint16_t* palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * simgCount) + (palleteSize256 * imgCount) + (palleteSize16 * c));

            if (type == 1)
            {
                palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * simgCount) + (palleteSize256 * imgCount) + (palleteSize16 * c));
            }
            else if (type == 2)
            {
                palette = (uint16_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * simgCount) + (palleteSize16 * c));
            }

            uint8_t* indicies = (uint8_t*)((uintptr_t)input_buffer + (imgSize * imgCount) + (simgHalfSize * c));
            void* output_buffer = malloc(simgSize * 4);
            bool bTransparencyFlag = false;
            if (TransparencyMode)
                bTransparencyFlag = true;


            DecodePS1ImageP4(indicies, palette, (uint32_t*)(output_buffer), simgSize, true);

            if (bUntile)
            {
                output_buffer = UntileImage256(output_buffer, simgUntileWidth, simgUntileHeight, simgSize);
                WriteImage(filename, index, c + imgCount, output_buffer, simgUntileWidth, simgUntileHeight);
            }
            else
                WriteImage(filename, index, c + imgCount, output_buffer, simgWidth, simgHeight);

            free(output_buffer);
        }
    }

    free(input_buffer);
}


int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout
            << "USAGE: " << argv[0] << " WA_MRG_path index [untile [transparency]]\n"
            << "index = image index\n"
            << "untile = enable/disable untiling (0/1) (1 = default)\n"
            << "transparency = transparency mode (0 = keep original data, black is transparent for subimages 0 on type 1 and 2 on type 2 (default), 2 = black is always transparent)\n";
        return -1;
    }

    bool bUntile = true;
    uint32_t TransparencyMode = 1;

    if (argc >= 4)
    {
        bUntile = std::stoi(argv[3]) != 0;
    }

    if (argc >= 5)
    {
        TransparencyMode = std::stoi(argv[argc-1]);
    }

    if (bUntile)
        std::cout << "Untiling enabled!\n";
    else
        std::cout << "Untiling disabled!\n";


    //for (int i = 0; i < 71; i++)
    //{
    //    ExtractBGToDDS(argv[1], i, true);
    //}

    for (int i = 512; i < 576; i++)
    {
        ExtractBGToDDS(argv[1], i, true);
    }

    //ExtractBGToDDS(argv[1], std::stoi(argv[2]), bUntile, TransparencyMode);


    return 0;
}