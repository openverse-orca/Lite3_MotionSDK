#!/usr/bin/env python3
"""
数据分析脚本，用于分析机器人数据记录器保存的CSV文件
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import sys
from pathlib import Path

def load_data(base_filename):
    """
    加载数据文件
    
    Args:
        base_filename: 基础文件名（不包含扩展名）
    
    Returns:
        tuple: (observation_df, raw_action_df, action_df)
    """
    # 查找最新的数据文件
    data_dir = Path(".")
    files = list(data_dir.glob(f"{base_filename}_*"))
    
    if not files:
        print(f"未找到以 {base_filename}_ 开头的文件")
        return None, None, None
    
    # 按修改时间排序，取最新的
    latest_file = max(files, key=lambda x: x.stat().st_mtime)
    # 往回找到倒数第一个数字的位置，截取到该数字（含）前的部分作为base_name
    import re
    stem = latest_file.stem
    match = list(re.finditer(r'\d+', stem))
    if match:
        last_num = match[-1]
        base_name = stem[:last_num.end()]
    else:
        base_name = stem
    
    observation_file = f"{base_name}_observation.csv"
    raw_action_file = f"{base_name}_raw_action.csv"
    action_file = f"{base_name}_action.csv"
    
    print(f"加载数据文件:")
    print(f"  Observation: {observation_file}")
    print(f"  Raw Action: {raw_action_file}")
    print(f"  Action: {action_file}")
    
    try:
        observation_df = pd.read_csv(observation_file)
        raw_action_df = pd.read_csv(raw_action_file)
        action_df = pd.read_csv(action_file)
        
        print(f"数据加载成功:")
        print(f"  Observation: {len(observation_df)} 行, {len(observation_df.columns)-1} 列数据")
        print(f"  Raw Action: {len(raw_action_df)} 行, {len(raw_action_df.columns)-1} 列数据")
        print(f"  Action: {len(action_df)} 行, {len(action_df.columns)-1} 列数据")
        
        return observation_df, raw_action_df, action_df
        
    except FileNotFoundError as e:
        print(f"文件未找到: {e}")
        return None, None, None
    except Exception as e:
        print(f"加载数据时出错: {e}")
        return None, None, None

def plot_observation_data(observation_df, save_plots=True):
    """
    绘制观察数据，按分类分组显示
    
    Args:
        observation_df: 观察数据DataFrame
        save_plots: 是否保存图片
    """
    if observation_df is None or len(observation_df) == 0:
        print("没有观察数据可绘制")
        return
    
    # 定义observation数据的分类结构
    obs_categories = {
        'Body Linear Acceleration': {
            'indices': [0, 1, 2],
            'names': ['acc_x', 'acc_y', 'acc_z'],
            'colors': ['blue', 'red', 'green']
        },
        'Body Angular Velocity': {
            'indices': [3, 4, 5],
            'names': ['roll_vel', 'pitch_vel', 'yaw_vel'],
            'colors': ['blue', 'red', 'green']
        },
        'Body Orientation': {
            'indices': [6, 7, 8],
            'names': ['roll', 'pitch', 'yaw'],
            'colors': ['blue', 'red', 'green']
        },
        'Command Values': {
            'indices': [9, 10, 11, 12],
            'names': ['cmd_vx', 'cmd_vy', 'cmd_vz', 'cmd_yaw'],
            'colors': ['blue', 'red', 'green', 'orange']
        },
        'Joint Position Offsets': {
            'indices': list(range(13, 25)),
            'names': [f'joint_pos_{i}' for i in range(12)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow']
        },
        'Joint Velocities': {
            'indices': list(range(25, 37)),
            'names': [f'joint_vel_{i}' for i in range(12)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow']
        },
        'Previous Actions': {
            'indices': list(range(37, 49)),
            'names': [f'prev_action_{i}' for i in range(12)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow']
        },
        'Height Measurements': {
            'indices': list(range(49, 65)),
            'names': [f'height_{i}' for i in range(16)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow', 'navy', 'maroon', 'teal', 'lime']
        }
    }
    
    # 获取数据列（排除timestamp列）
    data_cols = [col for col in observation_df.columns if col != 'timestamp']
    
    # 创建子图
    fig, axes = plt.subplots(4, 2, figsize=(20, 16))
    fig.suptitle('Observation Data Analysis by Category', fontsize=16)
    
    # 将axes转换为一维数组以便索引
    axes_flat = axes.flatten()
    
    # 为每个分类绘制图表
    for i, (category_name, category_info) in enumerate(obs_categories.items()):
        if i >= len(axes_flat):
            break
            
        ax = axes_flat[i]
        
        # 绘制该分类下的所有数据
        for j, idx in enumerate(category_info['indices']):
            if idx < len(data_cols):
                col_name = data_cols[idx]
                # 使用循环的颜色，确保每个数据都有不同的颜色
                color_idx = j % len(category_info['colors'])
                ax.plot(observation_df['timestamp'], observation_df[col_name], 
                       label=category_info['names'][j], 
                       color=category_info['colors'][color_idx], 
                       alpha=0.8,
                       linewidth=1.5)
        
        ax.set_title(f'{category_name} ({len(category_info["indices"])} values)')
        ax.set_xlabel('Timestamp')
        ax.set_ylabel('Value')
        ax.grid(True, alpha=0.3)
        ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.tight_layout()
    
    if save_plots:
        plt.savefig('observation_analysis.png', dpi=300, bbox_inches='tight')
        print("观察数据图已保存为 observation_analysis.png")
    
    plt.show()

def plot_action_data(raw_action_df, action_df, save_plots=True):
    """
    绘制动作数据，所有数据在一张图表上
    
    Args:
        raw_action_df: 原始动作数据DataFrame
        action_df: 处理后动作数据DataFrame
        save_plots: 是否保存图片
    """
    if raw_action_df is None or action_df is None:
        print("没有动作数据可绘制")
        return
    
    # 获取数据列（排除timestamp列）
    raw_cols = [col for col in raw_action_df.columns if col != 'timestamp']
    action_cols = [col for col in action_df.columns if col != 'timestamp']
    
    # 定义动作数据的分类结构（按腿部）
    action_categories = {
        'FL Leg': {
            'indices': [0, 1, 2],
            'names': ['FL_HipX', 'FL_HipY', 'FL_Knee'],
            'colors': ['blue', 'red', 'green']
        },
        'FR Leg': {
            'indices': [3, 4, 5],
            'names': ['FR_HipX', 'FR_HipY', 'FR_Knee'],
            'colors': ['orange', 'purple', 'brown']
        },
        'HL Leg': {
            'indices': [6, 7, 8],
            'names': ['HL_HipX', 'HL_HipY', 'HL_Knee'],
            'colors': ['pink', 'gray', 'olive']
        },
        'HR Leg': {
            'indices': [9, 10, 11],
            'names': ['HR_HipX', 'HR_HipY', 'HR_Knee'],
            'colors': ['cyan', 'magenta', 'yellow']
        }
    }
    
    # 创建单个图表
    fig, ax = plt.subplots(1, 1, figsize=(16, 8))
    
    # 绘制原始动作数据
    for leg_name, leg_info in action_categories.items():
        for j, idx in enumerate(leg_info['indices']):
            if idx < len(raw_cols):
                col_name = raw_cols[idx]
                color_idx = j % len(leg_info['colors'])
                ax.plot(raw_action_df['timestamp'], raw_action_df[col_name], 
                       label=f'Raw {leg_info["names"][j]}', 
                       color=leg_info['colors'][color_idx], 
                       alpha=0.8,
                       linewidth=2,
                       linestyle='-')
    
    # 绘制处理后动作数据
    for leg_name, leg_info in action_categories.items():
        for j, idx in enumerate(leg_info['indices']):
            if idx < len(action_cols):
                col_name = action_cols[idx]
                color_idx = j % len(leg_info['colors'])
                ax.plot(action_df['timestamp'], action_df[col_name], 
                       label=f'Processed {leg_info["names"][j]}', 
                       color=leg_info['colors'][color_idx], 
                       alpha=0.6,
                       linewidth=1.5,
                       linestyle='--')
    
    ax.set_title('Action Data Analysis - All Joints', fontsize=16)
    ax.set_xlabel('Timestamp')
    ax.set_ylabel('Action Value')
    ax.grid(True, alpha=0.3)
    ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.tight_layout()
    
    if save_plots:
        plt.savefig('action_analysis.png', dpi=300, bbox_inches='tight')
        print("动作数据图已保存为 action_analysis.png")
    
    plt.show()

def create_combined_plot(observation_df, raw_action_df, action_df, save_plots=True):
    """
    创建拼接的图表，包含observation数据和动作数据
    
    Args:
        observation_df: 观察数据DataFrame
        raw_action_df: 原始动作数据DataFrame
        action_df: 处理后动作数据DataFrame
        save_plots: 是否保存图片
    """
    if observation_df is None:
        print("没有观察数据可绘制")
        return
    
    # 定义observation数据的分类结构
    obs_categories = {
        'Body Linear Acceleration': {
            'indices': [0, 1, 2],
            'names': ['acc_x', 'acc_y', 'acc_z'],
            'colors': ['blue', 'red', 'green']
        },
        'Body Angular Velocity': {
            'indices': [3, 4, 5],
            'names': ['roll_vel', 'pitch_vel', 'yaw_vel'],
            'colors': ['blue', 'red', 'green']
        },
        'Body Orientation': {
            'indices': [6, 7, 8],
            'names': ['roll', 'pitch', 'yaw'],
            'colors': ['blue', 'red', 'green']
        },
        'Command Values': {
            'indices': [9, 10, 11, 12],
            'names': ['cmd_vx', 'cmd_vy', 'cmd_vz', 'cmd_yaw'],
            'colors': ['blue', 'red', 'green', 'orange']
        },
        'Joint Position Offsets': {
            'indices': list(range(13, 25)),
            'names': [f'joint_pos_{i}' for i in range(12)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow']
        },
        'Joint Velocities': {
            'indices': list(range(25, 37)),
            'names': [f'joint_vel_{i}' for i in range(12)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow']
        },
        'Previous Actions': {
            'indices': list(range(37, 49)),
            'names': [f'prev_action_{i}' for i in range(12)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow']
        },
        'Height Measurements': {
            'indices': list(range(49, 65)),
            'names': [f'height_{i}' for i in range(16)],
            'colors': ['blue', 'red', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan', 'magenta', 'yellow', 'navy', 'maroon', 'teal', 'lime']
        }
    }
    
    # 定义动作数据的分类结构
    action_categories = {
        'FL Leg': {
            'indices': [0, 1, 2],
            'names': ['FL_HipX', 'FL_HipY', 'FL_Knee'],
            'colors': ['blue', 'red', 'green']
        },
        'FR Leg': {
            'indices': [3, 4, 5],
            'names': ['FR_HipX', 'FR_HipY', 'FR_Knee'],
            'colors': ['orange', 'purple', 'brown']
        },
        'HL Leg': {
            'indices': [6, 7, 8],
            'names': ['HL_HipX', 'HL_HipY', 'HL_Knee'],
            'colors': ['pink', 'gray', 'olive']
        },
        'HR Leg': {
            'indices': [9, 10, 11],
            'names': ['HR_HipX', 'HR_HipY', 'HR_Knee'],
            'colors': ['cyan', 'magenta', 'yellow']
        }
    }
    
    # 获取数据列
    obs_cols = [col for col in observation_df.columns if col != 'timestamp']
    raw_cols = [col for col in raw_action_df.columns if col != 'timestamp'] if raw_action_df is not None else []
    action_cols = [col for col in action_df.columns if col != 'timestamp'] if action_df is not None else []
    
    # 创建拼接的图表：上半部分为observation数据，下半部分为动作数据
    fig = plt.figure(figsize=(24, 20))
    fig.suptitle('Robot Data Analysis - Combined View', fontsize=20)
    
    # 上半部分：observation数据 (4x2 布局)
    for i, (category_name, category_info) in enumerate(obs_categories.items()):
        ax = plt.subplot(4, 3, i + 1)
        
        # 绘制该分类下的所有数据
        for j, idx in enumerate(category_info['indices']):
            if idx < len(obs_cols):
                col_name = obs_cols[idx]
                color_idx = j % len(category_info['colors'])
                ax.plot(observation_df['timestamp'], observation_df[col_name], 
                       label=category_info['names'][j], 
                       color=category_info['colors'][color_idx], 
                       alpha=0.8,
                       linewidth=1.5)
        
        ax.set_title(f'{category_name} ({len(category_info["indices"])} values)', fontsize=12)
        ax.set_xlabel('Timestamp')
        ax.set_ylabel('Value')
        ax.grid(True, alpha=0.3)
        ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8)
    
    # 下半部分：动作数据 (第9-12个子图位置)
    if raw_action_df is not None and action_df is not None:
        ax_action = plt.subplot(4, 3, 9)
        
        # 绘制原始动作数据
        for leg_name, leg_info in action_categories.items():
            for j, idx in enumerate(leg_info['indices']):
                if idx < len(raw_cols):
                    col_name = raw_cols[idx]
                    color_idx = j % len(leg_info['colors'])
                    ax_action.plot(raw_action_df['timestamp'], raw_action_df[col_name], 
                                   label=f'Raw {leg_info["names"][j]}', 
                                   color=leg_info['colors'][color_idx], 
                                   alpha=0.8,
                                   linewidth=2,
                                   linestyle='-')
        
        # 绘制处理后动作数据
        for leg_name, leg_info in action_categories.items():
            for j, idx in enumerate(leg_info['indices']):
                if idx < len(action_cols):
                    col_name = action_cols[idx]
                    color_idx = j % len(leg_info['colors'])
                    ax_action.plot(action_df['timestamp'], action_df[col_name], 
                                   label=f'Processed {leg_info["names"][j]}', 
                                   color=leg_info['colors'][color_idx], 
                                   alpha=0.6,
                                   linewidth=1.5,
                                   linestyle='--')
        
        ax_action.set_title('Action Data - All Joints', fontsize=12)
        ax_action.set_xlabel('Timestamp')
        ax_action.set_ylabel('Action Value')
        ax_action.grid(True, alpha=0.3)
        ax_action.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8)
    
    plt.tight_layout()
    
    if save_plots:
        plt.savefig('combined_analysis.png', dpi=300, bbox_inches='tight')
        print("拼接分析图已保存为 combined_analysis.png")
    
    plt.show()

def print_statistics(observation_df, raw_action_df, action_df):
    """
    打印数据统计信息
    
    Args:
        observation_df: 观察数据DataFrame
        raw_action_df: 原始动作数据DataFrame
        action_df: 处理后动作数据DataFrame
    """
    print("\n=== 数据统计信息 ===")
    
    if observation_df is not None:
        print(f"\n观察数据统计:")
        print(f"  数据点数量: {len(observation_df)}")
        print(f"  时间范围: {observation_df['timestamp'].min()} - {observation_df['timestamp'].max()}")
        print(f"  数据列数: {len(observation_df.columns) - 1}")
        
        # 计算一些基本统计
        data_cols = [col for col in observation_df.columns if col != 'timestamp']
        print(f"  数值范围:")
        for col in data_cols[:5]:  # 只显示前5列
            print(f"    {col}: {observation_df[col].min():.6f} - {observation_df[col].max():.6f}")
    
    if raw_action_df is not None:
        print(f"\n原始动作数据统计:")
        print(f"  数据点数量: {len(raw_action_df)}")
        print(f"  时间范围: {raw_action_df['timestamp'].min()} - {raw_action_df['timestamp'].max()}")
        
        data_cols = [col for col in raw_action_df.columns if col != 'timestamp']
        print(f"  数值范围:")
        for col in data_cols:
            print(f"    {col}: {raw_action_df[col].min():.6f} - {raw_action_df[col].max():.6f}")
    
    if action_df is not None:
        print(f"\n处理后动作数据统计:")
        print(f"  数据点数量: {len(action_df)}")
        print(f"  时间范围: {action_df['timestamp'].min()} - {action_df['timestamp'].max()}")
        
        data_cols = [col for col in action_df.columns if col != 'timestamp']
        print(f"  数值范围:")
        for col in data_cols:
            print(f"    {col}: {action_df[col].min():.6f} - {action_df[col].max():.6f}")

def main():
    """
    主函数
    """
    if len(sys.argv) > 1:
        base_filename = sys.argv[1]
    else:
        base_filename = "robot_data"
    
    print(f"分析数据文件: {base_filename}")
    
    # 加载数据
    observation_df, raw_action_df, action_df = load_data(base_filename)
    
    if observation_df is None and raw_action_df is None and action_df is None:
        print("无法加载任何数据文件")
        return
    
    # 打印统计信息
    print_statistics(observation_df, raw_action_df, action_df)
    
    # 绘制图表并拼接
    try:
        # 创建拼接的图表
        create_combined_plot(observation_df, raw_action_df, action_df)
    except Exception as e:
        print(f"绘制图表时出错: {e}")
        print("请确保已安装 matplotlib 库: pip install matplotlib")

if __name__ == "__main__":
    main() 