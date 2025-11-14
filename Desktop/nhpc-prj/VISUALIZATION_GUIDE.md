# SUARA 3D Algorithm Visualization Guide

## 📊 Overview

This project includes **three different visualization methods** for the SUARA 3D algorithm, each with unique advantages:

1. **Matplotlib Interactive Animation** - Best for quick exploration and customization
2. **Manim Video Animation** - Best for high-quality presentations and videos
3. **Web-Based Visualization** - Best for accessibility and sharing

All visualizations feature a **dark theme** for better visual clarity and modern aesthetics.

---

## 🎨 Visualization 1: Matplotlib Interactive Animation

**File:** `suara_3d_animation.py`

### Features
- ✨ Real-time interactive 3D visualization
- 🎮 Play/pause controls and speed adjustment
- 🔄 Phase-by-phase navigation (jump to any phase)
- 📊 Live information panel with algorithm details
- 🌙 Dark theme with color-coded dimensions
- 🎯 Process highlighting and communication flow arrows

### Installation
```bash
pip install numpy matplotlib
```

### Usage

**Basic usage (default 4×4×2 grid):**
```bash
python suara_3d_animation.py
```

**Custom grid dimensions:**
```bash
python suara_3d_animation.py 2 2 2    # 2×2×2 grid (8 processes)
python suara_3d_animation.py 4 4 4    # 4×4×4 grid (64 processes)
python suara_3d_animation.py 8 4 2    # 8×4×2 grid (64 processes)
```

### Interactive Controls
- **Play/Pause Button** - Toggle animation
- **Speed Slider** - Adjust animation speed (0.1× to 3.0×)
- **P1-P6 Buttons** - Jump directly to any of the 6 phases
- **Mouse Controls:**
  - Left-click + drag: Rotate 3D view
  - Right-click + drag: Pan view
  - Scroll wheel: Zoom in/out

### Color Coding
- 🟥 **Red (X-dimension)** - Phases 1 and 6
- 🟦 **Cyan (Y-dimension)** - Phases 2 and 5
- 🟨 **Yellow (Z-dimension)** - Phases 3 and 4
- ⬜ **Gray** - Inactive processes

### Output
Real-time interactive window with:
- Left panel: 3D visualization with rotating grid
- Right panel: Phase information, controls, and statistics

### Best For
- Quick exploration of different grid configurations
- Understanding algorithm flow interactively
- Educational demonstrations
- Debugging and analysis

---

## 🎬 Visualization 2: Manim High-Quality Video

**File:** `suara_3d_manim_animation.py`

### Features
- 🎥 Professional, publication-quality video output
- 📹 Smooth camera movements and transitions
- 🎨 Cinematic dark theme with gradient effects
- 📊 Automatic phase progression with explanations
- 💫 Beautiful intro and conclusion sequences
- 🎓 Perfect for presentations and reports

### Installation
```bash
# Install Manim
pip install manim

# Install FFmpeg (required for video rendering)
# Ubuntu/Debian:
sudo apt-get install ffmpeg

# macOS:
brew install ffmpeg

# Windows: Download from https://ffmpeg.org/
```

### Usage

**Preview (low quality, fast render):**
```bash
manim -pql suara_3d_manim_animation.py SUARA3DVisualization
```

**High quality (1080p):**
```bash
manim -pqh suara_3d_manim_animation.py SUARA3DVisualization
```

**4K quality (for presentations):**
```bash
manim -pqk suara_3d_manim_animation.py SUARA3DVisualization
```

**Simplified 2D explanation:**
```bash
manim -pql suara_3d_manim_animation.py SUARA3DSimplified
```

### Command Options
- `-p` - Preview the video after rendering
- `-q` - Quality setting:
  - `l` - Low (480p, fast)
  - `m` - Medium (720p)
  - `h` - High (1080p)
  - `k` - 4K (2160p)

### Output
Video files saved to: `media/videos/suara_3d_manim_animation/[quality]/`

**Two scenes available:**
1. **SUARA3DVisualization** - Full 3D animation (~60 seconds)
2. **SUARA3DSimplified** - 2D phase diagram (~30 seconds)

### Video Structure
1. Introduction (5s)
2. Grid construction (8s)
3. Phase 1: Reduce-Scatter X (8s)
4. Phase 2: Reduce-Scatter Y (8s)
5. Phase 3: Reduce-Scatter Z (8s)
6. Phase 4: Allgather Z (8s)
7. Phase 5: Allgather Y (8s)
8. Phase 6: Allgather X (8s)
9. Conclusion and summary (7s)

### Best For
- Academic presentations
- Conference talks
- YouTube videos
- Project demonstrations
- High-quality documentation

---

## 🌐 Visualization 3: Web-Based Interactive (Three.js)

**File:** `suara_3d_web_visualization.html`

### Features
- 🌍 Runs in any modern web browser
- 📱 No installation required
- 🎮 Full interactive controls
- 🎨 Stunning 3D graphics with WebGL
- 📊 Real-time statistics and phase info
- 🔄 Auto-rotate camera option
- 💡 Responsive design with info panel

### Installation
**No installation needed!** Just open the HTML file in a browser.

### Usage

**Method 1: Direct file opening**
```bash
# Linux/macOS:
open suara_3d_web_visualization.html

# Or simply double-click the file
```

**Method 2: Simple web server (recommended)**
```bash
# Python 3:
python -m http.server 8000

# Then open browser to: http://localhost:8000/suara_3d_web_visualization.html
```

**Method 3: Live Server (VS Code)**
- Install "Live Server" extension in VS Code
- Right-click the HTML file
- Select "Open with Live Server"

### Interactive Controls
- **Play/Pause** - Control animation
- **Reset** - Return to Phase 1
- **Next Phase** - Skip to next phase
- **Speed Slider** - Adjust animation speed
- **Phase Buttons (P1-P6)** - Jump to specific phase
- **Auto-rotate Camera** - Toggle automatic rotation
- **Show Communication Arrows** - Toggle arrow visibility

### Mouse Controls
- **Left-click + drag** - Rotate view
- **Right-click + drag** - Pan view
- **Scroll wheel** - Zoom in/out
- **Hover** - Process details

### Display Panels
- **Left Side:** 3D visualization with real-time rendering
- **Right Side:**
  - Statistics (Grid size, processes, complexity, speedup)
  - Current phase information with progress bar
  - Animation controls
  - Phase selection buttons
  - Color legend
  - Algorithm phase list
  - Interactive help

### Browser Requirements
- Modern browser with WebGL support
- Recommended: Chrome, Firefox, Safari, Edge
- Minimum screen resolution: 1280×720

### Best For
- Easy sharing (just send the HTML file)
- Online documentation
- Remote presentations
- Student demonstrations
- No-installation-required scenarios

---

## 📋 Comparison Table

| Feature | Matplotlib | Manim | Web (Three.js) |
|---------|-----------|-------|----------------|
| **Installation** | Simple | Complex | None |
| **Quality** | Good | Excellent | Very Good |
| **Interactivity** | High | None | Very High |
| **Video Export** | No | Yes | No |
| **Sharing** | Screenshot | Video file | HTML file |
| **Customization** | Easy | Medium | Medium |
| **Performance** | Good | N/A | Excellent |
| **Best Use** | Exploration | Presentations | Sharing |

---

## 🎯 Algorithm Phases Visualized

All three visualizations show the same 6 phases:

### Reduce-Scatter Phases (1-3)
**Phase 1: Reduce-Scatter along X**
- Each YZ-plane performs reduce-scatter along X dimension
- Reduces data from X processes to X chunks
- Color: Red

**Phase 2: Reduce-Scatter along Y**
- Each XZ-plane performs reduce-scatter along Y dimension
- Reduces data from Y processes to Y chunks
- Color: Cyan

**Phase 3: Reduce-Scatter along Z**
- Each XY-plane performs reduce-scatter along Z dimension
- Reduces data from Z processes to Z chunks
- Color: Yellow

### Allgather Phases (4-6)
**Phase 4: Allgather along Z**
- Each XY-plane gathers data along Z dimension
- Reconstructs data across Z dimension
- Color: Yellow

**Phase 5: Allgather along Y**
- Each XZ-plane gathers data along Y dimension
- Reconstructs data across Y dimension
- Color: Cyan

**Phase 6: Allgather along X**
- Each YZ-plane gathers data along X dimension
- Completes the allreduce operation
- Color: Red

---

## 🚀 Quick Start Guide

### For Quick Exploration
```bash
# Use Matplotlib animation (easiest)
python suara_3d_animation.py
```

### For Presentations
```bash
# Create high-quality video
manim -pqh suara_3d_manim_animation.py SUARA3DVisualization
```

### For Sharing
```bash
# Open web visualization
python -m http.server 8000
# Then open: http://localhost:8000/suara_3d_web_visualization.html
```

---

## 🎨 Dark Theme Design

All visualizations use a consistent dark theme:

### Color Palette
- **Background:** `#0a0a0a` (Deep black)
- **Grid:** `#1a1a1a` (Dark gray)
- **Inactive:** `#2a4a5a` (Blue-gray)
- **X-Dimension:** `#ff6b6b` (Coral red)
- **Y-Dimension:** `#4ecdc4` (Turquoise)
- **Z-Dimension:** `#ffe66d` (Golden yellow)
- **Highlight:** `#ffd700` (Gold)
- **Text:** `#e0e0e0` (Light gray)

### Design Principles
- High contrast for visibility
- Color-coded dimensions for clarity
- Smooth animations for better understanding
- Minimal but informative UI
- Professional aesthetic

---

## 📊 Performance Tips

### Matplotlib Animation
- Use smaller grid sizes for smoother animation (4×4×2 recommended)
- Reduce animation speed for complex grids
- Close other applications for better performance

### Manim Video
- Use `-ql` for quick previews during development
- Use `-qh` or `-qk` only for final renders
- Rendering time: ~30-60 seconds for low quality, 3-5 minutes for 4K

### Web Visualization
- Works best on dedicated GPU
- Disable auto-rotate for manual control
- Use Chrome or Firefox for best performance
- Close unnecessary browser tabs

---

## 🛠️ Customization

### Changing Grid Dimensions

**Matplotlib:**
```bash
python suara_3d_animation.py 8 4 2  # X=8, Y=4, Z=2
```

**Manim:**
Edit the `construct` method in the Python file:
```python
self.X, self.Y, self.Z = 8, 4, 2  # Change these values
```

**Web:**
Edit the HTML file, find:
```javascript
let X = 8, Y = 4, Z = 2;  // Change these values
```

### Changing Colors

All visualizations use the same color constants. Find and modify:
- Matplotlib: `COLORS` dictionary
- Manim: Color constants at top of file
- Web: `phases` array in JavaScript

### Changing Animation Speed

- **Matplotlib:** Use the speed slider (0.1× to 3.0×)
- **Manim:** Edit `run_time` parameters in animations
- **Web:** Use the speed slider or edit `animationSpeed`

---

## 📚 Educational Use

### For Students
1. Start with **Web Visualization** (no installation)
2. Explore different phases interactively
3. Read the phase descriptions in the info panel
4. Try different grid configurations

### For Instructors
1. Use **Manim** to create lecture videos
2. Use **Matplotlib** for live demonstrations
3. Share **Web version** with students
4. Assign exploration exercises

### For Researchers
1. Use **Matplotlib** for algorithm analysis
2. Customize visualizations for your needs
3. Export frames for publications
4. Create comparison videos with Manim

---

## 🐛 Troubleshooting

### Matplotlib Issues
**Problem:** "No module named 'matplotlib'"
```bash
pip install matplotlib
```

**Problem:** Animation window doesn't appear
- Check if running in SSH session (requires X11 forwarding)
- Try: `export DISPLAY=:0` (Linux)

### Manim Issues
**Problem:** "LaTeX not found"
- Manim works without LaTeX for most scenes
- Install only if you see LaTeX errors

**Problem:** "FFmpeg not found"
```bash
# Install FFmpeg first, then retry
sudo apt-get install ffmpeg  # Ubuntu/Debian
brew install ffmpeg          # macOS
```

### Web Visualization Issues
**Problem:** Black screen in browser
- Check browser console for errors (F12)
- Ensure WebGL is enabled
- Try a different browser

**Problem:** Slow performance
- Reduce grid size in code
- Disable auto-rotate
- Close other browser tabs

---

## 📖 Additional Resources

### Algorithm Documentation
- `README.md` - Complete SUARA algorithm explanation
- `README_2D_SUARA.md` - 2D implementation details
- Source code comments in C files

### Visualization Files
- `suara_3d_animation.py` - Matplotlib interactive
- `suara_3d_manim_animation.py` - Manim video
- `suara_3d_web_visualization.html` - Web visualization
- `requirements_visualization.txt` - Python dependencies
- `VISUALIZATION_GUIDE.md` - This file

### External Links
- [Matplotlib Documentation](https://matplotlib.org/)
- [Manim Documentation](https://docs.manim.community/)
- [Three.js Documentation](https://threejs.org/docs/)
- [SUARA Original Paper](https://scholar.google.com/scholar?q=suara+allreduce)

---

## 🎓 Learning Objectives

After using these visualizations, you should understand:

1. ✅ How SUARA decomposes processes into a 3D grid
2. ✅ The six phases of the 3D algorithm
3. ✅ Why reduce-scatter phases come before allgather
4. ✅ How communication happens along each dimension
5. ✅ The O(∛P) complexity advantage
6. ✅ The difference between 2D and 3D SUARA

---

## 💡 Tips for Best Results

1. **Start Simple:** Begin with default 4×4×2 grid
2. **Understand Phases:** Watch full cycle before customizing
3. **Read Descriptions:** Pay attention to phase descriptions
4. **Experiment:** Try different grid configurations
5. **Compare:** Use all three visualizations for complete understanding
6. **Share:** Use web version to share with others
7. **Present:** Use Manim for professional presentations

---

## 📝 Credits

**Author:** NHPC Project Team
**Course:** CS 882 - Network-Based Computing for HPC
**Date:** November 2025
**License:** MIT (for visualization code)

**Technologies Used:**
- Python 3.8+
- Matplotlib 3.5+
- Manim Community Edition 0.17+
- Three.js r128
- WebGL 2.0

---

## 🤝 Contributing

To improve these visualizations:

1. Fork the repository
2. Create a feature branch
3. Make your improvements
4. Test with different grid sizes
5. Submit a pull request

**Ideas for improvements:**
- Add more algorithm variants (Rabenseifner, Ring)
- Performance comparison charts
- Network topology visualization
- Real-time performance metrics
- Mobile-responsive web version
- VR/AR visualization

---

## 📧 Support

For questions or issues:
- Check the Troubleshooting section above
- Review the source code comments
- Open an issue on GitHub
- Contact the course instructor

---

**Enjoy visualizing SUARA 3D! 🚀**
