#!/usr/bin/env python3
"""
数据记录器测试脚本
用于验证数据记录功能是否正常工作
"""

import pandas as pd
import numpy as np
import os
import sys
from pathlib import Path

def create_test_data():
    """
    创建测试数据文件
    """
    # 创建测试数据
    timestamps = list(range(10000, 10100, 20))  # 模拟时间戳
    
    # 创建观察数据（65列）
    observation_data = []
    for ts in timestamps:
        row = [ts] + [np.random.normal(0, 1) for _ in range(65)]
        observation_data.append(row)
    
    # 创建原始动作数据（12列）
    raw_action_data = []
    for ts in timestamps:
        row = [ts] + [np.random.normal(0, 0.5) for _ in range(12)]
        raw_action_data.append(row)
    
    # 创建处理后动作数据（12列）
    action_data = []
    for ts in timestamps:
        row = [ts] + [np.random.normal(0, 0.3) for _ in range(12)]
        action_data.append(row)
    
    # 创建列名
    obs_cols = ['timestamp'] + [f'obs_{i}' for i in range(65)]
    raw_action_cols = ['timestamp'] + [f'raw_action_{i}' for i in range(12)]
    action_cols = ['timestamp'] + [f'action_{i}' for i in range(12)]
    
    # 创建DataFrame
    obs_df = pd.DataFrame(observation_data, columns=obs_cols)
    raw_action_df = pd.DataFrame(raw_action_data, columns=raw_action_cols)
    action_df = pd.DataFrame(action_data, columns=action_cols)
    
    # 保存测试文件
    test_filename = "test_robot_data_2024-01-01_12-00-00"
    obs_df.to_csv(f"{test_filename}_observation.csv", index=False)
    raw_action_df.to_csv(f"{test_filename}_raw_action.csv", index=False)
    action_df.to_csv(f"{test_filename}_action.csv", index=False)
    
    print(f"测试数据文件已创建:")
    print(f"  {test_filename}_observation.csv")
    print(f"  {test_filename}_raw_action.csv")
    print(f"  {test_filename}_action.csv")
    
    return test_filename

def test_data_analysis():
    """
    测试数据分析功能
    """
    # 创建测试数据
    test_filename = create_test_data()
    
    # 导入分析脚本
    sys.path.append('scripts')
    try:
        from analyze_data import load_data, print_statistics, plot_observation_data, plot_action_data
        
        print("\n=== 测试数据分析功能 ===")
        
        # 加载数据
        observation_df, raw_action_df, action_df = load_data("test_robot_data")
        
        if observation_df is not None and raw_action_df is not None and action_df is not None:
            print("✓ 数据加载成功")
            
            # 打印统计信息
            print_statistics(observation_df, raw_action_df, action_df)
            
            # 测试绘图功能（不显示，只保存）
            try:
                # 导入新的拼接函数
                from analyze_data import create_combined_plot
                create_combined_plot(observation_df, raw_action_df, action_df, save_plots=True)
                print("✓ 拼接图表生成成功")
            except Exception as e:
                print(f"✗ 图表生成失败: {e}")
        else:
            print("✗ 数据加载失败")
            
    except ImportError as e:
        print(f"✗ 无法导入分析脚本: {e}")
    except Exception as e:
        print(f"✗ 测试过程中出错: {e}")
    
    # 清理测试文件
    cleanup_test_files(test_filename)

def cleanup_test_files(test_filename):
    """
    清理测试文件
    """
    files_to_remove = [
        f"{test_filename}_observation.csv",
        f"{test_filename}_raw_action.csv",
        f"{test_filename}_action.csv",
        "observation_analysis.png",
        "action_analysis.png"
    ]
    
    for file in files_to_remove:
        if os.path.exists(file):
            os.remove(file)
            print(f"已删除测试文件: {file}")

def main():
    """
    主函数
    """
    print("=== 数据记录器功能测试 ===")
    
    # 检查分析脚本是否存在
    if not os.path.exists('scripts/analyze_data.py'):
        print("✗ 分析脚本不存在: scripts/analyze_data.py")
        return
    
    # 测试数据分析功能
    test_data_analysis()
    
    print("\n=== 测试完成 ===")

if __name__ == "__main__":
    main() 