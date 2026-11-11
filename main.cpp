#include "te_mst.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fs = std::filesystem;

namespace {
cv::Mat readGray(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream)
        throw std::runtime_error("Cannot open an input image.");
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)), {});
    if (bytes.empty())
        throw std::runtime_error("An input image is empty.");
    cv::Mat image = cv::imdecode(bytes, cv::IMREAD_UNCHANGED);
    if (image.empty() || image.depth() != CV_8U)
        throw std::runtime_error("Inputs must be readable 8-bit images.");
    if (image.channels() == 3)
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
    if (image.channels() == 4)
        cv::cvtColor(image, image, cv::COLOR_BGRA2GRAY);
    return image;
}

void writePng(const fs::path& path, const cv::Mat& image) {
    std::vector<unsigned char> bytes;
    if (!cv::imencode(".png", image, bytes))
        throw std::runtime_error("Cannot encode output PNG.");
    std::ofstream stream(path, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    stream.close();
    if (!stream)
        throw std::runtime_error("Cannot write output PNG.");
}
}

int wmain(int argc, wchar_t* argv[]) {
    // 在这里填写可见光图、红外图和输出目录。支持中文路径。
    const fs::path visibleInput = LR"(sample\Visible\Road.png)";
    const fs::path infraredInput = LR"(sample\Infrared\Road.png)";
    const fs::path outputDirectory = LR"(result_manual)";

    if (argc != 1 && argc != 4) {
        std::cerr << "Usage: te_mst_demo <visible-image> <infrared-image> <output-directory>\n";
        return 2;
    }
    try {
        const fs::path visiblePath = argc == 4 ? fs::path(argv[1]) : visibleInput;
        const fs::path infraredPath = argc == 4 ? fs::path(argv[2]) : infraredInput;
        const fs::path output = argc == 4 ? fs::path(argv[3]) : outputDirectory;
        const cv::Mat visible = readGray(visiblePath);
        const cv::Mat infrared = readGray(infraredPath);


        SmoothCLAHE clahe(4.0, cv::Size(64, 64), true, true);

        const auto start = std::chrono::steady_clock::now();
        const cv::Mat fused = fuseTeMst(visible, infrared);       //1. 红外+强度融合增强   fused
       
        const auto fusionEnd = std::chrono::steady_clock::now();   

        cv::Mat enhanced;
        clahe.apply(fused, enhanced);    //2.  对fused  进行对比度提升   输出  enhanced

        const auto enhancementEnd = std::chrono::steady_clock::now();
        fs::create_directories(output);
        writePng(output / "fused.png", fused);
        writePng(output / "enhanced.png", enhanced);

        using Milliseconds = std::chrono::duration<double, std::milli>;
        std::cout << fused.cols << "x" << fused.rows
                  << "  Fusion: " << Milliseconds(fusionEnd - start).count()
                  << " ms  CLAHE: " << Milliseconds(enhancementEnd - fusionEnd).count()
                  << " ms\nWrote fused.png and enhanced.png\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
