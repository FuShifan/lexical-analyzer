# lexical-analyzer

大学必修课程“编译原理”课程设计：一个基于 Qt 的词法分析器教学工具。  
主要功能：从用户输入或文件读取正则定义（以“_”开头的命名正则），构建 NFA → DFA → 最小化 DFA，并支持导出表格与生成基于 DFA 的匹配代码。

主要源码（可阅读）：
- 核心 NFA 构造与解析：[`NFA::processed`](NFA.cpp) （文件：[NFA.cpp](NFA.cpp), 头文件：[NFA.h](NFA.h)）
- DFA 构造与最小化：[`DFA::minDFA`](DFA.cpp)、[`DFA::generate_switch_case_source`](DFA.cpp) （文件：[DFA.cpp](DFA.cpp), 头文件：[DFA.h](DFA.h)）
- UI 与交互：对话窗口与生成代码按钮：[`Dialog::setCurrentDFA`](dialog.cpp)、[`Dialog::on_pushButton_genCode_clicked`](dialog.cpp)（文件：[dialog.cpp](dialog.cpp), 头文件：[dialog.h](dialog.h)）
- 主界面入口：[`MainWindow::on_pushButton_clicked`](mainwindow.cpp)（文件：[mainwindow.cpp](mainwindow.cpp), 头文件：[mainwindow.h](mainwindow.h)）
- 示例/练习词法分析：`mini-c词法分析源程序.cpp`（[mini-c词法分析源程序.cpp](mini-c词法分析源程序.cpp)）和 `tiny词法分析源程序.cpp`（[tiny词法分析源程序.cpp](tiny词法分析源程序.cpp)）

依赖
- Qt 6（项目文件 [Compilers2.pro](Compilers2.pro)，使用 C++17）
- 合适的编译器（Windows 下建议 MinGW-w64，Linux/macOS 下使用系统 gcc/clang）

编译方法（命令行）
1. 使用 Qt Creator：直接打开工程文件 [Compilers2.pro](Compilers2.pro)，选择合适的 Kit，构建并运行。
2. 命令行（Windows MinGW 示例）：
    ```bash
    mkdir build
    cd build
    qmake ..\Compilers2.pro      # 或 qmake6 ..\Compilers2.pro 视 Qt 安装而定
    mingw32-make                 # 或 jom / make，视环境而定
    ```
   命令行（Linux/macOS）：
    ```bash
    mkdir build
    cd build
    qmake ../Compilers2.pro
    make
    ```
3. 可执行文件位置：构建后在 IDE 指定的构建目录中（示例：build/Desktop_Qt_6_7_3_MinGW_64_bit-Debug/ 或 debug/ 下）。

运行与使用
- 启动程序，使用“打开文件”或在主界面粘贴正则定义；点击“开始转换/重新转换”将触发 [NFA::processed](http://_vscodecontentref_/0) 的处理流程，生成 NFA/DFA/MinDFA 并在对话窗口展示。
- 在对话窗口可导出表格或点击“生成代码”按钮（触发 [Dialog::on_pushButton_genCode_clicked](http://_vscodecontentref_/1)），查看或复制根据 DFA 自动生成的匹配源码（由 [DFA::generate_switch_case_source](http://_vscodecontentref_/2) 产生）。

说明与注意
- 输入格式要求：以“_”开头的命名正则（例如 `_ID100 = [a-z]+`），程序会解析编号后缀作为 token 编码基并记录在映射中（详见 [NFA.cpp](http://_vscodecontentref_/3)）。
- 若要扩展词法规则，请参阅 [NFA.cpp](http://_vscodecontentref_/4) 中的正则预处理与后缀化实现，以及 [DFA.cpp](http://_vscodecontentref_/5) 中的生成与最小化逻辑。

许可证与作者
- 用于课程作业，资料与实现仅供学习