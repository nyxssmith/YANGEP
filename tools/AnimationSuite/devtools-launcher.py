"""Tkinter launcher listing the crab-maker dev tools with short descriptions."""

from __future__ import annotations

import subprocess
import sys
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, ttk

PROJECT_DIR = Path(__file__).resolve().parent
ANIMATION_CONFIG_DIR = PROJECT_DIR / "animation_configs"

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


def animation_config_names() -> list[str]:
	return sorted(path.stem for path in ANIMATION_CONFIG_DIR.glob("*.json"))


def open_edit_animation_picker(root: tk.Tk) -> None:
	names = animation_config_names()
	if not names:
		messagebox.showerror("No animations", f"No animation config JSON files found in {ANIMATION_CONFIG_DIR}.")
		return

	dialog = tk.Toplevel(root)
	dialog.title("Edit Animation")
	dialog.resizable(False, False)
	dialog.transient(root)
	dialog.grab_set()

	ttk.Label(dialog, text="Choose an animation to edit:").pack(anchor="w", padx=12, pady=(12, 4))
	selected_name = tk.StringVar(value=names[0])
	ttk.Combobox(dialog, textvariable=selected_name, values=names, state="readonly", width=30).pack(padx=12, pady=(0, 12), fill="x")

	def confirm() -> None:
		subprocess.Popen(
			[sys.executable, str(PROJECT_DIR / "make_animation.py"), "--edit", selected_name.get()],
			cwd=PROJECT_DIR,
		)
		dialog.destroy()

	buttons = ttk.Frame(dialog)
	buttons.pack(fill="x", padx=12, pady=(0, 12))
	ttk.Button(buttons, text="Cancel", command=dialog.destroy).pack(side="right", padx=(6, 0))
	ttk.Button(buttons, text="Edit", command=confirm).pack(side="right")


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
			button_row = ttk.Frame(row)
			button_row.pack(anchor="e")
			if title == "Make Animation":
				ttk.Button(button_row, text="Edit Existing", command=lambda: open_edit_animation_picker(root)).pack(side="left", padx=(0, 6))
			ttk.Button(button_row, text="Launch", command=lambda script_name=script_name: launch(script_name)).pack(side="left")


def main() -> None:
	root = tk.Tk()
	DevToolsLauncher(root)
	root.mainloop()


if __name__ == "__main__":
	main()
