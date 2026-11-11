# TE-MST C++ 

运行一级拉普拉斯金字塔融合和 Smooth CLAHE。低频等权，高频采用同向细节增强。CLAHE 使用 `clipLimit=4.0`、64×64 像素分块、LUT 平滑和双线性插值。

## 运行

打开 `Demo.sln`，选择 **Release | x64**，按 **Ctrl+F5**。入口在 `main.cpp` 的 `wmain`。在 `wmain` 开头填写 `visibleInput`、`infraredInput` 和 `outputDirectory`。默认路径指向附带样图，输出 `fused.png` 和 `enhanced.png`。同名输出文件会被覆盖。

融合与增强均在 `te_mst.h/.cpp`。工程使用 Visual Studio 2019 和 OpenCV 4.12。

输入为同尺寸、已配准的 8 位图像，彩色输入会转为灰度。程序分别打印融合和增强耗时，计时排除文件读写。

如需切换 OpenCV 安装位置，设置环境变量 `TE_MST_OPENCV_DIR` 和 `TE_MST_OPENCV_BIN`。默认使用本机 `D:\Ywt\openCV-4_12_0`。

小样图像来源：[Bristol Queen’s Road 测试对](https://github.com/uzeful/Infrared-and-Visual-Image-Fusion-via-Infrared-Feature-Extraction-and-Visual-Information-Preservation/tree/master/Dataset)。
