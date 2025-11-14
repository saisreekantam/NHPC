#!/usr/bin/env python3
"""
SUARA 3D Algorithm Visualization using Manim (High-Quality Video)
==================================================================

This script creates a professional, high-quality video animation of the
SUARA 3D algorithm using Manim (Mathematical Animation Engine).

Installation:
    pip install manim

Usage:
    manim -pql suara_3d_manim_animation.py SUARA3DVisualization  # Low quality preview
    manim -pqh suara_3d_manim_animation.py SUARA3DVisualization  # High quality
    manim -pqk suara_3d_manim_animation.py SUARA3DVisualization  # 4K quality

The output video will be saved in media/videos/suara_3d_manim_animation/
"""

from manim import *
import numpy as np

# Dark theme colors
DARK_BG = "#0a0a0a"
GRID_COLOR = "#1a1a1a"
INACTIVE_COLOR = "#2a4a5a"
X_COLOR = "#ff6b6b"
Y_COLOR = "#4ecdc4"
Z_COLOR = "#ffe66d"
DATA_FLOW_COLOR = "#ff69b4"
COMPLETE_COLOR = "#95e1d3"
TEXT_COLOR = "#e0e0e0"
HIGHLIGHT_COLOR = "#ffd700"


class SUARA3DVisualization(ThreeDScene):
    """
    Main Manim scene for SUARA 3D visualization
    """

    def construct(self):
        # Set background to dark theme
        self.camera.background_color = DARK_BG

        # Grid dimensions
        self.X, self.Y, self.Z = 4, 4, 2
        self.P = self.X * self.Y * self.Z

        # Introduction
        self.show_introduction()

        # Show grid construction
        self.show_grid_construction()

        # Show all 6 phases
        self.show_phase_1_reduce_scatter_x()
        self.show_phase_2_reduce_scatter_y()
        self.show_phase_3_reduce_scatter_z()
        self.show_phase_4_allgather_z()
        self.show_phase_5_allgather_y()
        self.show_phase_6_allgather_x()

        # Show conclusion
        self.show_conclusion()

    def show_introduction(self):
        """Show introduction screen"""
        title = Text("SUARA 3D Algorithm", font_size=72, color=HIGHLIGHT_COLOR)
        subtitle = Text(
            "Scalable Universal Allreduce in 3 Dimensions",
            font_size=36,
            color=TEXT_COLOR
        )
        subtitle.next_to(title, DOWN)

        # Grid info
        grid_info = VGroup(
            Text(f"Grid: {self.X} × {self.Y} × {self.Z}", color=X_COLOR),
            Text(f"Total Processes: {self.P}", color=Y_COLOR),
            Text(f"Complexity: O(∛P) = O({self.P**(1/3):.1f})", color=Z_COLOR)
        ).arrange(DOWN, buff=0.3)
        grid_info.next_to(subtitle, DOWN, buff=0.8)

        # Animate
        self.play(Write(title), run_time=1.5)
        self.play(FadeIn(subtitle), run_time=1)
        self.play(FadeIn(grid_info, shift=UP), run_time=1)
        self.wait(2)

        # Fade out
        self.play(
            FadeOut(title),
            FadeOut(subtitle),
            FadeOut(grid_info),
            run_time=1
        )

    def create_process_cube(self, x, y, z, color=INACTIVE_COLOR, opacity=0.6):
        """Create a cube representing a process"""
        size = 0.3
        cube = Cube(
            side_length=size,
            fill_color=color,
            fill_opacity=opacity,
            stroke_color=WHITE,
            stroke_width=1
        )

        # Position in 3D space
        cube.move_to([x * 0.8, y * 0.8, z * 1.2])

        # Add label
        label = Text(
            f"({x},{y},{z})",
            font_size=12,
            color=WHITE
        )
        label.move_to(cube.get_center())

        return VGroup(cube, label)

    def show_grid_construction(self):
        """Show the 3D grid construction"""
        # Setup 3D camera
        self.set_camera_orientation(phi=70 * DEGREES, theta=-45 * DEGREES)

        # Title
        title = Text("3D Process Grid Construction", font_size=48, color=HIGHLIGHT_COLOR)
        title.to_edge(UP)
        self.add_fixed_in_frame_mobjects(title)
        self.play(Write(title))

        # Create all process cubes
        self.processes = VGroup()
        self.process_dict = {}

        for x in range(self.X):
            for y in range(self.Y):
                for z in range(self.Z):
                    proc = self.create_process_cube(x, y, z)
                    self.processes.add(proc)
                    self.process_dict[(x, y, z)] = proc

        # Animate grid construction
        self.play(
            LaggedStart(
                *[FadeIn(proc, shift=UP) for proc in self.processes],
                lag_ratio=0.05
            ),
            run_time=3
        )

        # Add dimension labels
        x_label = Text("X", font_size=36, color=X_COLOR)
        y_label = Text("Y", font_size=36, color=Y_COLOR)
        z_label = Text("Z", font_size=36, color=Z_COLOR)

        # Position labels (fixed in frame)
        self.add_fixed_in_frame_mobjects(x_label, y_label, z_label)
        x_label.to_corner(DR)
        y_label.next_to(x_label, LEFT, buff=1)
        z_label.next_to(y_label, LEFT, buff=1)

        self.play(
            FadeIn(x_label),
            FadeIn(y_label),
            FadeIn(z_label)
        )

        # Rotate camera for better view
        self.begin_ambient_camera_rotation(rate=0.1)
        self.wait(2)
        self.stop_ambient_camera_rotation()

        self.wait(1)
        self.play(FadeOut(title))

    def highlight_dimension_communication(self, dimension, color, operation):
        """Highlight communication along a specific dimension"""
        title_text = f"Phase: {operation.upper()} along {dimension}"
        title = Text(title_text, font_size=40, color=color)
        title.to_edge(UP)
        self.add_fixed_in_frame_mobjects(title)
        self.play(Write(title))

        arrows = VGroup()

        if dimension == 'X':
            # Highlight X-dimension communication
            for y in range(self.Y):
                for z in range(self.Z):
                    for x in range(self.X - 1):
                        start = self.process_dict[(x, y, z)][0].get_center()
                        end = self.process_dict[(x + 1, y, z)][0].get_center()

                        arrow = Arrow3D(
                            start=start,
                            end=end,
                            color=color,
                            thickness=0.02
                        )
                        arrows.add(arrow)

                        # Highlight processes
                        self.play(
                            self.process_dict[(x, y, z)][0].animate.set_fill(
                                color, opacity=0.9
                            ),
                            self.process_dict[(x + 1, y, z)][0].animate.set_fill(
                                color, opacity=0.9
                            ),
                            run_time=0.1
                        )

        elif dimension == 'Y':
            # Highlight Y-dimension communication
            for x in range(self.X):
                for z in range(self.Z):
                    for y in range(self.Y - 1):
                        start = self.process_dict[(x, y, z)][0].get_center()
                        end = self.process_dict[(x, y + 1, z)][0].get_center()

                        arrow = Arrow3D(
                            start=start,
                            end=end,
                            color=color,
                            thickness=0.02
                        )
                        arrows.add(arrow)

                        # Highlight processes
                        self.play(
                            self.process_dict[(x, y, z)][0].animate.set_fill(
                                color, opacity=0.9
                            ),
                            self.process_dict[(x, y + 1, z)][0].animate.set_fill(
                                color, opacity=0.9
                            ),
                            run_time=0.1
                        )

        elif dimension == 'Z':
            # Highlight Z-dimension communication
            for x in range(self.X):
                for y in range(self.Y):
                    for z in range(self.Z - 1):
                        start = self.process_dict[(x, y, z)][0].get_center()
                        end = self.process_dict[(x, y, z + 1)][0].get_center()

                        arrow = Arrow3D(
                            start=start,
                            end=end,
                            color=color,
                            thickness=0.02
                        )
                        arrows.add(arrow)

                        # Highlight processes
                        self.play(
                            self.process_dict[(x, y, z)][0].animate.set_fill(
                                color, opacity=0.9
                            ),
                            self.process_dict[(x, y, z + 1)][0].animate.set_fill(
                                color, opacity=0.9
                            ),
                            run_time=0.1
                        )

        # Show arrows
        self.play(Create(arrows), run_time=1)
        self.wait(2)

        # Reset colors
        self.play(
            *[proc[0].animate.set_fill(INACTIVE_COLOR, opacity=0.6)
              for proc in self.processes],
            FadeOut(arrows),
            FadeOut(title),
            run_time=1
        )

    def show_phase_1_reduce_scatter_x(self):
        """Phase 1: Reduce-scatter along X"""
        self.highlight_dimension_communication('X', X_COLOR, 'Reduce-Scatter')

    def show_phase_2_reduce_scatter_y(self):
        """Phase 2: Reduce-scatter along Y"""
        self.highlight_dimension_communication('Y', Y_COLOR, 'Reduce-Scatter')

    def show_phase_3_reduce_scatter_z(self):
        """Phase 3: Reduce-scatter along Z"""
        self.highlight_dimension_communication('Z', Z_COLOR, 'Reduce-Scatter')

    def show_phase_4_allgather_z(self):
        """Phase 4: Allgather along Z"""
        self.highlight_dimension_communication('Z', Z_COLOR, 'Allgather')

    def show_phase_5_allgather_y(self):
        """Phase 5: Allgather along Y"""
        self.highlight_dimension_communication('Y', Y_COLOR, 'Allgather')

    def show_phase_6_allgather_x(self):
        """Phase 6: Allgather along X"""
        self.highlight_dimension_communication('X', X_COLOR, 'Allgather')

    def show_conclusion(self):
        """Show conclusion and summary"""
        # Fade out grid
        self.play(FadeOut(self.processes), run_time=1)

        # Summary
        summary_title = Text("SUARA 3D Algorithm Summary", font_size=60, color=HIGHLIGHT_COLOR)
        summary_title.to_edge(UP)
        self.add_fixed_in_frame_mobjects(summary_title)

        summary_points = VGroup(
            Text("✓ 6 Phases: 3 Reduce-Scatter + 3 Allgather", color=TEXT_COLOR),
            Text(f"✓ Complexity: O(∛P) = O({self.P**(1/3):.1f}) latency", color=X_COLOR),
            Text(f"✓ Bandwidth: M/∛P per process", color=Y_COLOR),
            Text(f"✓ Speedup: {self.P**(1/6):.2f}× over 2D SUARA", color=Z_COLOR),
            Text("✓ Optimal for large-scale HPC systems", color=COMPLETE_COLOR)
        ).arrange(DOWN, buff=0.5, aligned_edge=LEFT)

        summary_points.move_to(ORIGIN)
        self.add_fixed_in_frame_mobjects(summary_points)

        self.play(Write(summary_title))
        self.play(
            LaggedStart(
                *[FadeIn(point, shift=RIGHT) for point in summary_points],
                lag_ratio=0.3
            ),
            run_time=3
        )

        self.wait(3)

        # Credits
        credits = Text(
            "Created for NHPC Project - CS 882",
            font_size=24,
            color=TEXT_COLOR
        )
        credits.to_edge(DOWN)
        self.add_fixed_in_frame_mobjects(credits)
        self.play(FadeIn(credits))

        self.wait(2)


class Arrow3D(ThreeDVMobject):
    """Custom 3D arrow for better visualization"""

    def __init__(self, start=ORIGIN, end=RIGHT, color=WHITE, thickness=0.02, **kwargs):
        super().__init__(**kwargs)

        # Create line
        line = Line3D(start=start, end=end, color=color, thickness=thickness)

        # Create arrowhead
        direction = end - start
        direction = direction / np.linalg.norm(direction)

        cone_height = 0.15
        cone_base = end - direction * cone_height

        arrowhead = Cone(
            direction=direction,
            base_center=cone_base,
            height=cone_height,
            base_radius=0.05,
            color=color,
            fill_opacity=1
        )

        self.add(line, arrowhead)


# Alternative: Simple phase-by-phase explanation scene
class SUARA3DSimplified(Scene):
    """Simplified 2D diagram explaining SUARA 3D phases"""

    def construct(self):
        self.camera.background_color = DARK_BG

        # Title
        title = Text("SUARA 3D: Six-Phase Algorithm", font_size=56, color=HIGHLIGHT_COLOR)
        title.to_edge(UP)
        self.play(Write(title))

        # Create phase boxes
        phases = [
            ("Phase 1", "Reduce-Scatter\nalong X", X_COLOR),
            ("Phase 2", "Reduce-Scatter\nalong Y", Y_COLOR),
            ("Phase 3", "Reduce-Scatter\nalong Z", Z_COLOR),
            ("Phase 4", "Allgather\nalong Z", Z_COLOR),
            ("Phase 5", "Allgather\nalong Y", Y_COLOR),
            ("Phase 6", "Allgather\nalong X", X_COLOR),
        ]

        phase_boxes = VGroup()
        for i, (phase_num, phase_desc, color) in enumerate(phases):
            box = VGroup(
                Rectangle(
                    width=2.5,
                    height=1.5,
                    fill_color=color,
                    fill_opacity=0.3,
                    stroke_color=color,
                    stroke_width=3
                ),
                Text(phase_num, font_size=24, color=WHITE, weight=BOLD),
                Text(phase_desc, font_size=18, color=TEXT_COLOR)
            )
            box[1].move_to(box[0].get_top() + DOWN * 0.3)
            box[2].move_to(box[0].get_center() + DOWN * 0.2)

            phase_boxes.add(box)

        # Arrange in 2 rows
        row1 = VGroup(*phase_boxes[:3]).arrange(RIGHT, buff=0.5)
        row2 = VGroup(*phase_boxes[3:]).arrange(RIGHT, buff=0.5)

        phase_grid = VGroup(row1, row2).arrange(DOWN, buff=0.8)
        phase_grid.next_to(title, DOWN, buff=1)

        # Animate phases
        self.play(
            LaggedStart(
                *[FadeIn(box, shift=UP) for box in phase_boxes],
                lag_ratio=0.2
            ),
            run_time=4
        )

        # Add arrows between phases
        arrows = VGroup()
        for i in range(5):
            if i < 2:
                start = phase_boxes[i]
                end = phase_boxes[i + 1]
            elif i == 2:
                start = phase_boxes[2]
                end = phase_boxes[3]
            else:
                start = phase_boxes[i]
                end = phase_boxes[i + 1]

            arrow = Arrow(
                start.get_right() if i < 3 else start.get_left() if i > 3 else start.get_bottom(),
                end.get_left() if i < 3 else end.get_right() if i > 3 else end.get_top(),
                color=HIGHLIGHT_COLOR,
                buff=0.1
            )
            arrows.add(arrow)

        self.play(Create(arrows), run_time=2)

        # Add complexity info
        complexity = VGroup(
            Text("Complexity Analysis:", font_size=32, color=HIGHLIGHT_COLOR),
            Text("• Latency: O(∛P) vs O(√P) in 2D", font_size=24, color=X_COLOR),
            Text("• Bandwidth: M/∛P per process", font_size=24, color=Y_COLOR),
            Text("• Best for P ≥ 1000 processes", font_size=24, color=Z_COLOR)
        ).arrange(DOWN, aligned_edge=LEFT, buff=0.3)
        complexity.to_edge(DOWN, buff=0.5)

        self.play(FadeIn(complexity, shift=UP), run_time=2)
        self.wait(3)


if __name__ == "__main__":
    """
    Run with:
        manim -pql suara_3d_manim_animation.py SUARA3DVisualization
        manim -pql suara_3d_manim_animation.py SUARA3DSimplified
    """
    pass
