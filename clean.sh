#!/bin/bash

# Lite3 Motion SDK 清理脚本

echo "=== Cleaning Lite3 Motion SDK ==="

# 检查是否存在build目录
if [ -d "build" ]; then
    echo "Cleaning build directory..."
    cd build
    make clean
    cd ..
    
    # 询问是否删除build目录
    read -p "Do you want to remove the build directory? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Removing build directory..."
        rm -rf build
        echo "✓ Build directory removed"
    else
        echo "Build directory kept"
    fi
else
    echo "No build directory found"
fi

echo "✓ Clean completed!" 