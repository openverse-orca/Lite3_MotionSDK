#!/bin/bash

# Lite3 Motion SDK 运行脚本
# 解决 GLIBCXX 版本兼容性问题

echo "=== Lite3 Motion SDK 运行脚本 ==="

# 检查可执行文件是否存在
if [ ! -f "./build/Lite_motion" ]; then
    echo "错误: 找不到可执行文件 ./build/Lite_motion"
    echo "请先运行 ./build.sh 构建项目"
    exit 1
fi

# 设置库路径，优先使用系统库而不是 miniconda 的库
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/usr/local/lib:/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"

# 临时禁用 miniconda 的库路径
if [ -n "$CONDA_PREFIX" ]; then
    echo "检测到 conda 环境，临时调整库路径..."
    # 从 LD_LIBRARY_PATH 中移除 conda 路径
    export LD_LIBRARY_PATH=$(echo $LD_LIBRARY_PATH | sed 's|'$CONDA_PREFIX'/lib:||g')
    export LD_LIBRARY_PATH=$(echo $LD_LIBRARY_PATH | sed 's|:'$CONDA_PREFIX'/lib||g')
fi

# 显示当前库路径
echo "当前库路径: $LD_LIBRARY_PATH"

# 检查 GLIBCXX 版本
echo "检查 GLIBCXX 版本..."
strings /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep GLIBCXX | tail -5

# 运行程序
echo "启动 Lite_motion..."
cd build
./Lite_motion

# 检查程序退出状态
if [ $? -eq 0 ]; then
    echo "✓ 程序运行成功"
else
    echo "✗ 程序运行失败"
    echo ""
    echo "如果仍然遇到 GLIBCXX 错误，请尝试以下解决方案："
    echo "1. 更新系统的 libstdc++: sudo apt update && sudo apt install libstdc++6"
    echo "2. 或者使用 conda 安装更新版本的 libstdcxx-ng: conda install libstdcxx-ng"
    echo "3. 或者临时禁用 conda 环境: conda deactivate"
fi 