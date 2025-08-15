#!/usr/bin/env python3
"""
拼接分析功能演示脚本
展示如何使用新的拼接分析功能
"""

import sys
import os

# 添加当前目录到Python路径
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from analyze_data import load_data, create_combined_plot, print_statistics

def main():
    """
    主函数 - 演示拼接分析功能
    """
    print("=== 拼接分析功能演示 ===")
    
    # 检查是否有数据文件
    import glob
    data_files = glob.glob("robot_data_*_observation.csv")
    
    if not data_files:
        print("未找到数据文件，请先运行主程序生成数据文件")
        print("或者运行测试脚本生成示例数据")
        return
    
    # 使用最新的数据文件
    latest_file = max(data_files, key=os.path.getctime)
    base_name = latest_file.replace('_observation.csv', '')
    
    print(f"使用数据文件: {base_name}")
    
    # 加载数据
    observation_df, raw_action_df, action_df = load_data(base_name)
    
    if observation_df is None and raw_action_df is None and action_df is None:
        print("无法加载数据文件")
        return
    
    # 打印统计信息
    print_statistics(observation_df, raw_action_df, action_df)
    
    # 创建拼接图表
    print("\n正在生成拼接分析图表...")
    create_combined_plot(observation_df, raw_action_df, action_df, save_plots=True)
    
    print("\n=== 演示完成 ===")
    print("生成的图表文件: combined_analysis.png")
    print("图表包含:")
    print("  - 8个observation数据分类 (每个分类内使用不同颜色)")
    print("  - 1个动作数据图表 (按腿部使用不同颜色)")
    print("  - 原始动作数据用实线表示")
    print("  - 处理后动作数据用虚线表示")

if __name__ == "__main__":
    main() 