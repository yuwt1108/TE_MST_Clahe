# TE-MST C++ 

这是一个面向实时应用的红外与可见光图像融合增强 C++ 示例。程序将同尺寸、已配准的红外图像和可见光图像融合为一幅图像，再通过 Smooth CLAHE 增强局部对比度。

**单次处理约 10 ms**，包含一对图像的融合与增强，不含文件读写。实际耗时取决于图像分辨率和运行硬件。

运行一级拉普拉斯金字塔融合和 Smooth CLAHE。低频等权，高频采用同向细节增强。CLAHE 使用 `clipLimit=4.0`、64×64 像素分块、LUT 平滑和双线性插值。

## 运行

打开 `Demo.sln`，选择 **Release | x64**，按 **Ctrl+F5** 运行视频示例。也可以用 CMake 构建 `demo_photo` 和 `demo_video` 两个独立目标。

```powershell
cmake -S . -B build/demo -A x64 -DOpenCV_DIR="<OpenCV 安装路径>"
cmake --build build/demo --config Release
.\build\demo\Release\demo_photo.exe
.\build\demo\Release\demo_video.exe
```

从仓库根目录运行。OpenCV 的 DLL 目录需要在 `PATH` 中。照片示例默认使用 `sample/Visible/1824.bmp` 与 `sample/Infrared/1824.bmp`，输出 `result_manual/fused_enhanced.png`。视频示例默认使用 `sample/video/VOT_RGBT2019_crouch` 中的两段 MP4，输出 `result_manual/fused_enhanced_crouch.mp4`。同名输出文件会被覆盖，视频预览中可按 Esc 停止。

也可以依次传入可见光输入、红外输入和输出位置。照片模式的输出位置为目录，视频模式为 MP4 路径。

融合与增强均在 `te_mst.h/.cpp`。工程使用 Visual Studio 2019 和 OpenCV 4.12。

输入为同尺寸、已配准的 8 位图像，彩色输入会转为灰度。程序分别打印融合和增强耗时，计时排除文件读写。

如需切换 OpenCV 安装位置，设置环境变量 `TE_MST_OPENCV_DIR` 和 `TE_MST_OPENCV_BIN`。默认使用本机 `D:\Ywt\openCV-4_12_0`。

小样图像来源：[Bristol Queen’s Road 测试对](https://github.com/uzeful/Infrared-and-Visual-Image-Fusion-via-Infrared-Feature-Extraction-and-Visual-Information-Preservation/tree/master/Dataset)。
