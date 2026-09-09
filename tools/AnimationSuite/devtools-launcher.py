"""Tkinter launcher listing the crab-maker dev tools with short descriptions."""

from __future__ import annotations

import subprocess
import sys
import tkinter as tk
from pathlib import Path
from tkinter import ttk

PROJECT_DIR = Path(__file__).resolve().parent

TOOLS = (
	(
		"Map Images",
		"map_images.py",
		"Place attachment points on each raw source_imgs PNG and save it into mapped_imgs/.",
	),
	(
		"Make Crab Config",
		"make_crab_config.py",
		"Pick a style (color) per limb and body, preview up/down/right, and save a crab_configs/*.json.",
	),
	(
		"Make Animation",
		"make_animation.py",
		"Pose a crab config's limbs frame by frame and save an animation_configs/*.json.",
	),
	(
		"Output From Animation",
		"output_from_animation.py",
		"Render a saved animation config with a chosen crab config into a PNG frame strip in output_imgs/.",
	),
	(
		"Combine Animation Slices",
		"combine_animation_slices.py",
		"Stack 4 direction frame strips (up/down/left/right) from output_imgs/ into one spritesheet.",
	),
)


def launch(script_name: str) -> None:
	subprocess.Popen([sys.executable, str(PROJECT_DIR / script_name)], cwd=PROJECT_DIR)


class DevToolsLauncher:
	def __init__(self, root: tk.Tk) -> None:
		root.title("Crab Maker Dev Tools")
		root.resizable(False, False)

		container = ttk.Frame(root, padding=16)
		container.pack(fill="both", expand=True)

		ttk.Label(container, text="Crab Maker Dev Tools", font=("TkDefaultFont", 14, "bold")).pack(anchor="w", pady=(0, 12))

		for title, script_name, description in TOOLS:
			row = ttk.LabelFrame(container, text=title, padding=8)
			row.pack(fill="x", pady=(0, 8))
			ttk.Label(row, text=description, wraplength=420, justify="left").pack(anchor="w", pady=(0, 6))
			ttk.Button(row, text="Launch", command=lambda script_name=script_name: launch(script_name)).pack(anchor="e")


def main() -> None:
	root = tk.Tk()
	DevToolsLauncher(root)
	root.mainloop()


if __name__ == "__main__":
	main()
