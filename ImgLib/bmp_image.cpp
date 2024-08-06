#include "bmp_image.h"

#include <array>
#include <fstream>
#include <string_view>

#include "pack_defines.h"

using namespace std;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        // поля заголовка Bitmap File Header
        uint8_t signature[2]{'B', 'M'};  // symbols BM
        uint32_t size = 0;               // step (GetBMPStride) * height
        uint32_t reserve = 0;            // zeros
        uint32_t step = 54;              // BitmapFileHeader + BitmapInfoHeader
    }
        PACKED_STRUCT_END

        PACKED_STRUCT_BEGIN BitmapInfoHeader{
        // поля заголовка Bitmap Info Header
        uint32_t size = 40;
        int32_t width = 0;
        int32_t height = 0;
        uint16_t plate = 1;
        uint16_t bitPerPixel = 24;
        uint32_t compression = 0;
        uint32_t bytes = 0;
        int32_t hRes = 11811;
        int32_t vRes = 11811;
        int32_t colors = 0;
        int32_t valuableColors = 0x1000000;
    }
        PACKED_STRUCT_END

        // функция вычисления отступа по ширине
        static int GetBMPStride(int w) { return 4 * ((w * 3 + 3) / 4); }

    bool SaveBMP(const Path& file, const Image& image) {
        ofstream out(file, ios::binary);
        if (!out) {
            return false;
        }

        const int width = image.GetWidth();
        const int height = image.GetHeight();
        const int step = GetBMPStride(width);
        const int pixelDataSize = step * height;
        const int fileSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + pixelDataSize;

        BitmapFileHeader fileHeader;
        fileHeader.size = fileSize;
        fileHeader.reserve = 0;
        fileHeader.step = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

        BitmapInfoHeader infoHeader;
        infoHeader.size = sizeof(BitmapInfoHeader);
        infoHeader.width = width;
        infoHeader.height = height;
        infoHeader.bytes = pixelDataSize;

        out.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BitmapFileHeader));
        out.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BitmapInfoHeader));

        vector<char> buf(step);
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                const auto& pixel = image.GetPixel(x, y);
                buf[x * 3] = static_cast<char>(pixel.b);
                buf[x * 3 + 1] = static_cast<char>(pixel.g);
                buf[x * 3 + 2] = static_cast<char>(pixel.r);
            }
            out.write(buf.data(), step);
        }

        return out.good();
    }

    bool IsValidBitmapFileHeader(const BitmapFileHeader& header) {
        if (header.signature[0] != 'B' || header.signature[1] != 'M') {
            return false;
        }
        if (header.reserve != 0 || header.step != 54) {
            return false;
        }
        return true;
    }

    bool IsValidBitmapInfoHeader(const BitmapInfoHeader& header) {
        if (header.size != 40 ||
            header.plate != 1 ||
            header.bitPerPixel != 24 ||
            header.compression != 0 ||
            header.hRes != 11811 || header.vRes != 11811 ||
            header.colors != 0 ||
            header.valuableColors != 0x1000000) {
            return false;
        }
        return true;
    }

    // напишите эту функцию
    Image LoadBMP(const Path& file) {
        BitmapFileHeader fileHeader;
        BitmapInfoHeader infoHeader;
        ifstream ifs(file, ios::binary);
        if (!ifs.good()) {
            return {};
        }
        ifs.read(reinterpret_cast<char*>(&fileHeader), sizeof(BitmapFileHeader));
        if (!IsValidBitmapFileHeader(fileHeader)) {
            return {};
        }

        ifs.read(reinterpret_cast<char*>(&infoHeader), sizeof(BitmapInfoHeader));
        if (!IsValidBitmapInfoHeader(infoHeader)) {
            return {};
        }

        const int width = infoHeader.width;
        const int height = infoHeader.height;
        const int step = GetBMPStride(width);

        Image image(width, height, Color::Black());
        vector<char> buf(step);

        for (int y = height - 1; y >= 0; --y) {
            ifs.read(buf.data(), step);
            for (int x = 0; x < width; ++x) {
                auto& pixel = image.GetPixel(x, y);
                pixel.b = static_cast<byte>(buf[x * 3]);
                pixel.g = static_cast<byte>(buf[x * 3 + 1]);
                pixel.r = static_cast<byte>(buf[x * 3 + 2]);
            }
        }
        return image;
    }

}  // namespace img_lib