#!/bin/bash

echo "=== 键盘控制功能演示 ==="
echo ""
echo "这个演示将展示如何在Lite3 Motion SDK中使用键盘控制功能。"
echo ""

# 检查构建目录是否存在
if [ ! -d "build" ]; then
    echo "错误: 构建目录不存在。请先运行 ./build.sh"
    exit 1
fi

# 检查测试程序是否存在
if [ ! -f "build/test_keyboard_controller" ]; then
    echo "错误: 测试程序不存在。请先运行 ./build.sh"
    exit 1
fi

echo "1. 测试键盘控制器功能"
echo "   运行测试程序，你可以尝试以下按键："
echo "   - 按 SPACE 键测试空格键功能"
echo "   - 按 'h' 键查看帮助"
echo "   - 按 'q' 键退出程序"
echo ""
echo "   按任意键开始测试..."
read -n 1

echo ""
echo "启动键盘控制器测试程序..."
echo "=================================="
cd build && ./test_keyboard_controller
echo "=================================="
echo ""

echo "2. 主程序集成说明"
echo "   主程序 (Lite_motion) 已经集成了键盘控制功能："
echo "   - SPACE 键: 切换零动作调试模式"
echo "   - 'q' 键: 退出程序"
echo "   - 'h' 键: 显示帮助"
echo ""

echo "3. 使用建议"
echo "   - 在调试机器人动作时，可以使用SPACE键快速切换零动作模式"
echo "   - 这样可以方便地测试机器人的基本控制功能"
echo "   - 程序会在控制台显示当前的状态信息"
echo ""

echo "演示完成！" 