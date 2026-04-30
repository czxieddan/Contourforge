"""
生成测试用的高度图
需要安装: pip install numpy pillow
"""

import numpy as np
from PIL import Image
import os

def create_gradient_simple(width=256, height=256):
    """创建简单的线性渐变"""
    gradient = np.linspace(0, 255, height, dtype=np.uint8)
    gradient = np.tile(gradient.reshape(-1, 1), (1, width))
    return gradient

def create_gradient_radial(width=256, height=256):
    """创建径向渐变（圆锥形）"""
    x = np.linspace(-1, 1, width)
    y = np.linspace(-1, 1, height)
    X, Y = np.meshgrid(x, y)
    R = np.sqrt(X**2 + Y**2)
    R = np.clip(R, 0, 1)
    radial = ((1 - R) * 255).astype(np.uint8)
    return radial

def create_terrain_peaks(width=256, height=256, num_peaks=5):
    """创建多峰地形"""
    np.random.seed(42)  # 固定随机种子以便重现
    peaks = np.zeros((height, width), dtype=np.float32)
    
    for _ in range(num_peaks):
        cx = np.random.randint(width // 4, 3 * width // 4)
        cy = np.random.randint(height // 4, 3 * height // 4)
        amplitude = np.random.uniform(0.5, 1.0)
        sigma = np.random.uniform(20, 40)
        
        for i in range(height):
            for j in range(width):
                dist = np.sqrt((i - cy)**2 + (j - cx)**2)
                peaks[i, j] += amplitude * np.exp(-dist**2 / (2 * sigma**2))
    
    # 归一化到0-255
    peaks = (peaks / peaks.max() * 255).astype(np.uint8)
    return peaks

def create_wave_pattern(width=256, height=256):
    """创建波浪图案"""
    x = np.linspace(0, 4 * np.pi, width)
    y = np.linspace(0, 4 * np.pi, height)
    X, Y = np.meshgrid(x, y)
    
    Z = np.sin(X) * np.cos(Y)
    Z = ((Z + 1) / 2 * 255).astype(np.uint8)
    return Z

def main():
    output_dir = os.path.dirname(os.path.abspath(__file__))
    
    print("生成测试高度图...")
    
    # 1. 简单渐变
    print("  - gradient_simple.png")
    img = create_gradient_simple()
    Image.fromarray(img, mode='L').save(os.path.join(output_dir, 'gradient_simple.png'))
    
    # 2. 径向渐变
    print("  - gradient_radial.png")
    img = create_gradient_radial()
    Image.fromarray(img, mode='L').save(os.path.join(output_dir, 'gradient_radial.png'))
    
    # 3. 多峰地形
    print("  - terrain_peaks.png")
    img = create_terrain_peaks()
    Image.fromarray(img, mode='L').save(os.path.join(output_dir, 'terrain_peaks.png'))
    
    # 4. 波浪图案
    print("  - wave_pattern.png")
    img = create_wave_pattern()
    Image.fromarray(img, mode='L').save(os.path.join(output_dir, 'wave_pattern.png'))
    
    # 5. 更大的测试图（性能测试）
    print("  - terrain_large.png (512x512)")
    img = create_terrain_peaks(512, 512, num_peaks=10)
    Image.fromarray(img, mode='L').save(os.path.join(output_dir, 'terrain_large.png'))
    
    print("完成！生成了5个测试高度图。")

if __name__ == '__main__':
    main()
