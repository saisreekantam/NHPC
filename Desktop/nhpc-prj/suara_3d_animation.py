#!/usr/bin/env python3
"""
SUARA 3D Algorithm Visualization with Dark Theme
=================================================

This script creates an interactive 3D animation showing the 6 phases of the SUARA algorithm:
1. Reduce-scatter along X dimension
2. Reduce-scatter along Y dimension
3. Reduce-scatter along Z dimension
4. Allgather along Z dimension
5. Allgather along Y dimension
6. Allgather along X dimension

Features:
- Dark theme for better visualization
- Interactive controls (play/pause, speed control, phase selection)
- Process highlighting during communication
- Data flow visualization
- Phase-by-phase breakdown with explanations
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.patches as mpatches
from matplotlib.widgets import Button, Slider
import sys

# Dark theme configuration
plt.style.use('dark_background')
COLORS = {
    'background': '#0a0a0a',
    'grid': '#1a1a1a',
    'inactive': '#2a4a5a',
    'active_x': '#ff6b6b',
    'active_y': '#4ecdc4',
    'active_z': '#ffe66d',
    'data_flow': '#ff69b4',
    'complete': '#95e1d3',
    'text': '#e0e0e0',
    'highlight': '#ffd700'
}

class SUARA3DAnimation:
    def __init__(self, X=4, Y=4, Z=2):
        """
        Initialize SUARA 3D animation

        Parameters:
        -----------
        X, Y, Z : int
            Grid dimensions (X × Y × Z = total processes)
        """
        self.X, self.Y, self.Z = X, Y, Z
        self.P = X * Y * Z

        # Animation state
        self.current_phase = 0
        self.current_step = 0
        self.is_playing = True
        self.speed = 1.0

        # Phase definitions
        self.phases = [
            {
                'name': 'Phase 1: Reduce-Scatter along X',
                'dimension': 'X',
                'operation': 'reduce-scatter',
                'color': COLORS['active_x'],
                'description': 'Each YZ-plane reduces and scatters data along X dimension'
            },
            {
                'name': 'Phase 2: Reduce-Scatter along Y',
                'dimension': 'Y',
                'operation': 'reduce-scatter',
                'color': COLORS['active_y'],
                'description': 'Each XZ-plane reduces and scatters data along Y dimension'
            },
            {
                'name': 'Phase 3: Reduce-Scatter along Z',
                'dimension': 'Z',
                'operation': 'reduce-scatter',
                'color': COLORS['active_z'],
                'description': 'Each XY-plane reduces and scatters data along Z dimension'
            },
            {
                'name': 'Phase 4: Allgather along Z',
                'dimension': 'Z',
                'operation': 'allgather',
                'color': COLORS['active_z'],
                'description': 'Each XY-plane gathers complete data along Z dimension'
            },
            {
                'name': 'Phase 5: Allgather along Y',
                'dimension': 'Y',
                'operation': 'allgather',
                'color': COLORS['active_y'],
                'description': 'Each XZ-plane gathers complete data along Y dimension'
            },
            {
                'name': 'Phase 6: Allgather along X',
                'dimension': 'X',
                'operation': 'allgather',
                'color': COLORS['active_x'],
                'description': 'Each YZ-plane gathers complete data along X dimension'
            }
        ]

        # Create process grid coordinates
        self.process_coords = []
        for x in range(X):
            for y in range(Y):
                for z in range(Z):
                    self.process_coords.append((x, y, z))

        # Setup figure
        self.setup_figure()

    def setup_figure(self):
        """Setup the matplotlib figure with dark theme"""
        self.fig = plt.figure(figsize=(16, 10), facecolor=COLORS['background'])

        # Main 3D plot
        self.ax = self.fig.add_subplot(121, projection='3d', facecolor=COLORS['background'])

        # Info panel
        self.info_ax = self.fig.add_subplot(122, facecolor=COLORS['background'])
        self.info_ax.axis('off')

        # Control buttons
        self.setup_controls()

        # Configure 3D plot
        self.ax.set_xlabel('X Dimension', color=COLORS['text'], fontsize=12, weight='bold')
        self.ax.set_ylabel('Y Dimension', color=COLORS['text'], fontsize=12, weight='bold')
        self.ax.set_zlabel('Z Dimension', color=COLORS['text'], fontsize=12, weight='bold')
        self.ax.set_xlim(-0.5, self.X - 0.5)
        self.ax.set_ylim(-0.5, self.Y - 0.5)
        self.ax.set_zlim(-0.5, self.Z - 0.5)

        # Set dark theme for 3D plot
        self.ax.xaxis.pane.fill = False
        self.ax.yaxis.pane.fill = False
        self.ax.zaxis.pane.fill = False
        self.ax.xaxis.pane.set_edgecolor(COLORS['grid'])
        self.ax.yaxis.pane.set_edgecolor(COLORS['grid'])
        self.ax.zaxis.pane.set_edgecolor(COLORS['grid'])
        self.ax.grid(True, alpha=0.2, color=COLORS['grid'])
        self.ax.tick_params(colors=COLORS['text'])

        # Title
        self.title = self.fig.suptitle('SUARA 3D Algorithm Visualization',
                                       fontsize=18, weight='bold',
                                       color=COLORS['highlight'])

    def setup_controls(self):
        """Setup interactive controls"""
        # Play/Pause button
        self.play_ax = plt.axes([0.02, 0.02, 0.08, 0.04])
        self.play_button = Button(self.play_ax, 'Pause',
                                  color=COLORS['grid'],
                                  hovercolor=COLORS['inactive'])
        self.play_button.on_clicked(self.toggle_play)

        # Speed slider
        self.speed_ax = plt.axes([0.15, 0.02, 0.15, 0.03])
        self.speed_slider = Slider(self.speed_ax, 'Speed', 0.1, 3.0,
                                   valinit=1.0, color=COLORS['active_x'])
        self.speed_slider.on_changed(self.update_speed)

        # Phase buttons
        button_width = 0.08
        button_height = 0.03
        button_spacing = 0.002
        start_x = 0.35
        start_y = 0.02

        self.phase_buttons = []
        for i in range(6):
            bax = plt.axes([start_x + i * (button_width + button_spacing),
                           start_y, button_width, button_height])
            btn = Button(bax, f'P{i+1}',
                        color=COLORS['grid'],
                        hovercolor=self.phases[i]['color'])
            btn.on_clicked(lambda event, phase=i: self.jump_to_phase(phase))
            self.phase_buttons.append(btn)

    def toggle_play(self, event):
        """Toggle play/pause"""
        self.is_playing = not self.is_playing
        self.play_button.label.set_text('Play' if not self.is_playing else 'Pause')

    def update_speed(self, val):
        """Update animation speed"""
        self.speed = val

    def jump_to_phase(self, phase):
        """Jump to specific phase"""
        self.current_phase = phase
        self.current_step = 0

    def draw_process(self, x, y, z, color, alpha=0.6, size=0.3):
        """Draw a single process as a cube"""
        # Define cube vertices
        r = size / 2
        vertices = np.array([
            [x-r, y-r, z-r], [x+r, y-r, z-r], [x+r, y+r, z-r], [x-r, y+r, z-r],
            [x-r, y-r, z+r], [x+r, y-r, z+r], [x+r, y+r, z+r], [x-r, y+r, z+r]
        ])

        # Define the 6 faces
        faces = [
            [vertices[0], vertices[1], vertices[5], vertices[4]],
            [vertices[7], vertices[6], vertices[2], vertices[3]],
            [vertices[0], vertices[3], vertices[7], vertices[4]],
            [vertices[1], vertices[2], vertices[6], vertices[5]],
            [vertices[0], vertices[1], vertices[2], vertices[3]],
            [vertices[4], vertices[5], vertices[6], vertices[7]]
        ]

        # Create 3D polygon
        poly = Poly3DCollection(faces, facecolors=color,
                               linewidths=1, edgecolors='white',
                               alpha=alpha)
        self.ax.add_collection3d(poly)

        # Add process label
        self.ax.text(x, y, z, f'P{x},{y},{z}',
                    color='white', fontsize=6,
                    ha='center', va='center',
                    weight='bold')

    def draw_communication_line(self, start, end, color, alpha=0.8):
        """Draw a line showing communication between processes"""
        self.ax.plot3D([start[0], end[0]],
                      [start[1], end[1]],
                      [start[2], end[2]],
                      color=color, linewidth=3,
                      alpha=alpha, linestyle='--')

        # Add arrow
        direction = np.array(end) - np.array(start)
        mid_point = np.array(start) + direction * 0.7
        self.ax.scatter(*mid_point, color=color, s=100,
                       marker='>', alpha=alpha)

    def get_active_processes(self, phase_idx, step):
        """Get processes that are active in current phase and step"""
        phase = self.phases[phase_idx]
        dimension = phase['dimension']

        active = []

        if dimension == 'X':
            # Processes in same YZ plane communicate
            for y in range(self.Y):
                for z in range(self.Z):
                    plane_procs = [(x, y, z) for x in range(self.X)]
                    if step < len(plane_procs) - 1:
                        active.append(plane_procs)

        elif dimension == 'Y':
            # Processes in same XZ plane communicate
            for x in range(self.X):
                for z in range(self.Z):
                    plane_procs = [(x, y, z) for y in range(self.Y)]
                    if step < len(plane_procs) - 1:
                        active.append(plane_procs)

        elif dimension == 'Z':
            # Processes in same XY plane communicate
            for x in range(self.X):
                for y in range(self.Y):
                    plane_procs = [(x, y, z) for z in range(self.Z)]
                    if step < len(plane_procs) - 1:
                        active.append(plane_procs)

        return active

    def draw_frame(self, frame):
        """Draw a single frame of the animation"""
        self.ax.clear()

        # Update configuration
        self.ax.set_xlabel('X Dimension', color=COLORS['text'],
                          fontsize=12, weight='bold')
        self.ax.set_ylabel('Y Dimension', color=COLORS['text'],
                          fontsize=12, weight='bold')
        self.ax.set_zlabel('Z Dimension', color=COLORS['text'],
                          fontsize=12, weight='bold')
        self.ax.set_xlim(-0.5, self.X - 0.5)
        self.ax.set_ylim(-0.5, self.Y - 0.5)
        self.ax.set_zlim(-0.5, self.Z - 0.5)
        self.ax.grid(True, alpha=0.2, color=COLORS['grid'])

        # Get current phase info
        phase = self.phases[self.current_phase]

        # Update title
        self.title.set_text(f"{phase['name']}\n{phase['description']}")

        # Draw all processes
        active_procs = self.get_active_processes(self.current_phase, self.current_step)

        for x, y, z in self.process_coords:
            # Check if this process is active
            is_active = False
            for group in active_procs:
                if (x, y, z) in group:
                    is_active = True
                    break

            if is_active:
                self.draw_process(x, y, z, phase['color'], alpha=0.9, size=0.35)
            else:
                self.draw_process(x, y, z, COLORS['inactive'], alpha=0.3, size=0.3)

        # Draw communication lines
        for group in active_procs:
            for i in range(len(group) - 1):
                # Animate communication pattern
                if self.current_step % 2 == 0:
                    self.draw_communication_line(group[i], group[i+1],
                                                phase['color'])
                else:
                    self.draw_communication_line(group[i+1], group[i],
                                                phase['color'])

        # Update info panel
        self.update_info_panel()

        # Update animation state
        if self.is_playing:
            self.current_step += 1
            max_steps = max(self.X, self.Y, self.Z) * 2
            if self.current_step >= max_steps:
                self.current_step = 0
                self.current_phase = (self.current_phase + 1) % 6

    def update_info_panel(self):
        """Update the information panel"""
        self.info_ax.clear()
        self.info_ax.axis('off')

        phase = self.phases[self.current_phase]

        # Info text
        info_text = f"""
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃  SUARA 3D ALGORITHM INFORMATION  ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

Grid Configuration:
  • Dimensions: {self.X} × {self.Y} × {self.Z}
  • Total Processes: {self.P}

Current Phase: {self.current_phase + 1}/6
  • {phase['name']}
  • Operation: {phase['operation'].upper()}
  • Active Dimension: {phase['dimension']}

Algorithm Details:
  • Complexity: O(∛P) latency
  • Bandwidth: M/∛P per process
  • Speedup: {self.P**(1/6):.2f}× over 2D

Phase Sequence:
  {'✓' if self.current_phase >= 0 else '○'} Phase 1: Reduce-Scatter X
  {'✓' if self.current_phase >= 1 else '○'} Phase 2: Reduce-Scatter Y
  {'✓' if self.current_phase >= 2 else '○'} Phase 3: Reduce-Scatter Z
  {'✓' if self.current_phase >= 3 else '○'} Phase 4: Allgather Z
  {'✓' if self.current_phase >= 4 else '○'} Phase 5: Allgather Y
  {'✓' if self.current_phase >= 5 else '○'} Phase 6: Allgather X

Legend:
  🟥 X-dimension communication
  🟦 Y-dimension communication
  🟨 Z-dimension communication
  ⬜ Inactive processes

Controls:
  • Play/Pause: Toggle animation
  • Speed: Adjust animation speed
  • P1-P6: Jump to specific phase
"""

        self.info_ax.text(0.05, 0.95, info_text,
                         transform=self.info_ax.transAxes,
                         fontsize=10, verticalalignment='top',
                         fontfamily='monospace',
                         color=COLORS['text'])

        # Add color legend
        legend_elements = [
            mpatches.Patch(color=COLORS['active_x'], label='X Communication'),
            mpatches.Patch(color=COLORS['active_y'], label='Y Communication'),
            mpatches.Patch(color=COLORS['active_z'], label='Z Communication'),
            mpatches.Patch(color=COLORS['inactive'], label='Inactive')
        ]

        self.info_ax.legend(handles=legend_elements, loc='lower left',
                          framealpha=0.9, facecolor=COLORS['grid'],
                          edgecolor=COLORS['text'])

    def animate(self):
        """Create and display the animation"""
        anim = FuncAnimation(self.fig, self.draw_frame,
                           frames=None, interval=500/self.speed,
                           blit=False, repeat=True)

        plt.tight_layout()
        plt.show()

        return anim


def main():
    """Main function to run the animation"""
    print("=" * 60)
    print("SUARA 3D Algorithm Visualization")
    print("=" * 60)
    print("\nThis animation demonstrates the 6 phases of the SUARA 3D")
    print("allreduce algorithm with a dark theme for better visualization.")
    print("\nControls:")
    print("  • Play/Pause: Toggle animation")
    print("  • Speed slider: Adjust animation speed")
    print("  • P1-P6 buttons: Jump to specific phase")
    print("\nGrid dimensions can be customized (default: 4×4×2)")
    print("=" * 60)

    # Check for custom dimensions
    if len(sys.argv) == 4:
        try:
            X, Y, Z = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
            print(f"\nUsing custom grid: {X}×{Y}×{Z}")
        except:
            X, Y, Z = 4, 4, 2
            print(f"\nUsing default grid: {X}×{Y}×{Z}")
    else:
        X, Y, Z = 4, 4, 2
        print(f"\nUsing default grid: {X}×{Y}×{Z}")

    # Create and run animation
    viz = SUARA3DAnimation(X, Y, Z)
    viz.animate()


if __name__ == '__main__':
    main()
