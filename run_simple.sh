#!/bin/bash

# 简单的运行脚本 - 临时禁用 conda 环境

echo "=== 简单运行脚本 ==="

# 检查可执行文件
if [ ! -f "./build/Lite_motion" ]; then
    echo "错误: 找不到可执行文件 ./build/Lite_motion"
    echo "请先运行 ./build.sh 构建项目"
    exit 1
fi

# 临时禁用 conda 环境
if [ -n "$CONDA_PREFIX" ]; then
    echo "临时禁用 conda 环境..."
    # 保存原始环境变量
    export ORIGINAL_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
    export ORIGINAL_PATH="$PATH"
    
    # 移除 conda 相关的环境变量
    unset CONDA_PREFIX
    unset CONDA_DEFAULT_ENV
    unset CONDA_PROMPT_MODIFIER
    
    # 设置系统库路径
    export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/usr/local/lib:/lib/x86_64-linux-gnu"
    export PATH="/usr/local/bin:/usr/bin:/bin"
fi

echo "运行 Lite_motion..."
cd build
./Lite_motion

# 恢复环境变量
if [ -n "$ORIGINAL_LD_LIBRARY_PATH" ]; then
    export LD_LIBRARY_PATH="$ORIGINAL_LD_LIBRARY_PATH"
    export PATH="$ORIGINAL_PATH"
    echo "已恢复原始环境变量"
fi 