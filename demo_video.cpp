#include "te_mst.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

namespace fs = std::filesystem;

namespace {
    cv::Mat toGray8(const cv::Mat& src) {
        if (src.empty())
            throw std::runtime_error("Empty frame.");
        cv::Mat img;
        if (src.depth() != CV_8U)
            src.convertTo(img, CV_8U);
        else
            img = src.clone();

        if (img.channels() == 3)
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        if (img.channels() == 4)
            cv::cvtColor(img, img, cv::COLOR_BGRA2GRAY);
        return img;
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
    // ========== 默认路径：可见光视频、红外视频、输出视频文件 ==========
    const fs::path visibleVideoPath = LR"(sample/video/VOT_RGBT2019_crouch/Visible.mp4)";
    const fs::path infraredVideoPath = LR"(sample/video/VOT_RGBT2019_crouch/Infrared.mp4)";
    const fs::path outputVideoPath = LR"(result_manual/fused_enhanced_crouch.mp4)";

    // 用法：te_mst_video_demo <可见光视频> <红外视频> <输出视频路径>
    if (argc != 1 && argc != 4) {
        std::cerr << "Usage: te_mst_video_demo <visible-video> <infrared-video> <output-video>\n";
        std::cerr << "ESC key to exit during processing\n";
        return 2;
    }

    try {
        const fs::path visPath = argc == 4 ? fs::path(argv[1]) : visibleVideoPath;
        const fs::path irPath = argc == 4 ? fs::path(argv[2]) : infraredVideoPath;
        const fs::path outPath = argc == 4 ? fs::path(argv[3]) : outputVideoPath;

        // 【关键】输出文件存在则删除，支持覆盖
        if (fs::exists(outPath)) {
            if (fs::equivalent(outPath, visPath) || fs::equivalent(outPath, irPath))
                throw std::runtime_error("Output must not overwrite an input video.");
            fs::remove(outPath);
            std::cout << "Old output file exists, removed for overwrite: " << outPath << "\n";
        }
        // 确保输出所在文件夹存在
        if (outPath.has_parent_path())
            fs::create_directories(outPath.parent_path());

        cv::VideoCapture capVis(visPath.string());
        cv::VideoCapture capIr(irPath.string());
        if (!capVis.isOpened()) throw std::runtime_error("Cannot open visible video.");
        if (!capIr.isOpened())  throw std::runtime_error("Cannot open infrared video.");

        int w = static_cast<int>(capVis.get(cv::CAP_PROP_FRAME_WIDTH));
        int h = static_cast<int>(capVis.get(cv::CAP_PROP_FRAME_HEIGHT));
        double fps = capVis.get(cv::CAP_PROP_FPS);
        if (fps <= 0) fps = 25;

        cv::VideoWriter writer;
        cv::Size outSize(w, h);
        writer.open(outPath.string(),
            cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
            fps, outSize, false); // false 灰度

        if (!writer.isOpened())
            throw std::runtime_error("Cannot open output video writer.");

        SmoothCLAHE clahe(4.0, cv::Size(64, 64), true, true);

        cv::Mat frameVis, frameIr;
        int frameIdx = 0;

        while (true) {
            bool okVis = capVis.read(frameVis);
            bool okIr = capIr.read(frameIr);
            if (!okVis || !okIr) {
                std::cout << "Reach end of video stream.\n";
                break;
            }

            auto start = std::chrono::steady_clock::now();

            cv::Mat grayVis = toGray8(frameVis);
            cv::Mat grayIr = toGray8(frameIr);

            if (grayIr.size() != grayVis.size()) {
                cv::resize(grayIr, grayIr, grayVis.size(), 0, 0, cv::INTER_LINEAR);
            }

            cv::Mat fused = fuseTeMst(grayVis, grayIr);

            auto fusionEnd = std::chrono::steady_clock::now();

            cv::Mat enhanced;
            clahe.apply(fused, enhanced);

            auto enhancementEnd = std::chrono::steady_clock::now();

            writer.write(enhanced);

            using Milliseconds = std::chrono::duration<double, std::milli>;
            double tFuse = Milliseconds(fusionEnd - start).count();
            double tClahe = Milliseconds(enhancementEnd - fusionEnd).count();

            std::cout << "Frame " << frameIdx
                << " | Fusion: " << tFuse
                << " ms | CLAHE: " << tClahe
                << " ms\n";

            cv::imshow("Fused Enhanced", enhanced);
            int key = cv::waitKey(1);
            if (key == 27) {
                std::cout << "User pressed ESC, stop.\n";
                break;
            }

            frameIdx++;
        }

        capVis.release();
        capIr.release();
        writer.release();
        cv::destroyAllWindows();
        std::cout << "Video saved to: " << outPath << "\n";

    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
