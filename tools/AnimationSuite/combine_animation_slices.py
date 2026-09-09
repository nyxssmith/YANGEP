"""Combine 4 direction frame-strip PNGs (up/down/left/right) into one stacked spritesheet."""

from __future__ import annotations

import argparse
import sys
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

from PIL import Image, ImageTk

PROJECT_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = PROJECT_DIR / "output_imgs"
DIRECTIONS = ("up", "left", "down", "right")


def combine_slices(paths: dict[str, Path]) -> Image.Image:
	missing = [direction for direction in DIRECTIONS if direction not in paths]
	if missing:
		raise ValueError(f"Missing slice(s) for direction(s): {', '.join(missing)}")

	images = {direction: Image.open(paths[direction]).convert("RGBA") for direction in DIRECTIONS}
	sizes = {images[direction].size for direction in DIRECTIONS}
	if len(sizes) != 1:
		details = ", ".join(f"{direction}={images[direction].size}" for direction in DIRECTIONS)
		raise ValueError(f"All 4 slices must have the same dimensions, got: {details}")

	width, height = next(iter(sizes))
	sheet = Image.new("RGBA", (width, height * len(DIRECTIONS)))
	for row, direction in enumerate(DIRECTIONS):
		sheet.alpha_composite(images[direction], (0, row * height))
	return sheet


def default_output_path() -> Path:
	OUTPUT_DIR.mkdir(exist_ok=True)
	candidate = OUTPUT_DIR / "combined_spritesheet.png"
	index = 1
	while candidate.exists():
		candidate = OUTPUT_DIR / f"combined_spritesheet_{index}.png"
		index += 1
	return candidate


def resolve_output_path(filename: str) -> Path:
	filename = filename.strip()
	if not filename:
		return default_output_path()
	path = Path(filename)
	if path.suffix.lower() != ".png":
		path = path.with_suffix(".png")
	if not path.is_absolute():
		path = OUTPUT_DIR / path
	path.parent.mkdir(exist_ok=True, parents=True)
	return path


class CombineSlicesApp:
	"""Pick 4 images from output_imgs/, assign them to a direction, and stack them into a spritesheet."""

	def __init__(self, root: tk.Tk) -> None:
		self.root = root
		root.title("Combine Animation Slices")
		root.resizable(True, True)
		root.minsize(420, 320)

		self.available_files = sorted(str(path.name) for path in OUTPUT_DIR.glob("*.png"))
		self.path_vars: dict[str, tk.StringVar] = {}
		self.thumbnails: dict[str, ImageTk.PhotoImage] = {}

		form = ttk.Frame(root, padding=12)
		form.pack(fill="both", expand=True)

		for direction in DIRECTIONS:
			row = ttk.LabelFrame(form, text=direction.title(), padding=8)
			row.pack(fill="x", pady=(0, 8))

			var = tk.StringVar()
			self.path_vars[direction] = var
			combo = ttk.Combobox(row, textvariable=var, values=self.available_files, width=32)
			combo.pack(side="left", padx=(0, 8))
			combo.bind("<<ComboboxSelected>>", lambda event, direction=direction: self.update_preview(direction))
			ttk.Button(row, text="Browse...", command=lambda direction=direction: self.browse(direction)).pack(side="left")

			preview = ttk.Label(row)
			preview.pack(side="left", padx=(8, 0))
			setattr(self, f"preview_{direction}", preview)

		filename_row = ttk.Frame(form)
		filename_row.pack(fill="x", pady=(0, 8))
		ttk.Label(filename_row, text="Output filename:").pack(side="left", padx=(0, 8))
		self.filename_var = tk.StringVar()
		ttk.Entry(filename_row, textvariable=self.filename_var, width=32).pack(side="left")
		ttk.Label(filename_row, text="(blank = auto-named, saved in output_imgs/)").pack(side="left", padx=(8, 0))

		self.status_label = ttk.Label(form, padding=(0, 8, 0, 0))
		self.status_label.pack(anchor="w")

		ttk.Button(form, text="Combine & Save", command=self.combine_and_save).pack(anchor="e", pady=(8, 0))

	def browse(self, direction: str) -> None:
		path = filedialog.askopenfilename(initialdir=OUTPUT_DIR, filetypes=[("PNG images", "*.png")])
		if path:
			self.path_vars[direction].set(path)
			self.update_preview(direction)

	def resolve_path(self, direction: str) -> Path | None:
		value = self.path_vars[direction].get().strip()
		if not value:
			return None
		path = Path(value)
		if not path.is_absolute():
			path = OUTPUT_DIR / path
		return path

	def update_preview(self, direction: str) -> None:
		path = self.resolve_path(direction)
		preview: ttk.Label = getattr(self, f"preview_{direction}")
		if path is None or not path.exists():
			preview.configure(image="", text="")
			return
		image = Image.open(path).convert("RGBA")
		image.thumbnail((96, 32))
		photo = ImageTk.PhotoImage(image)
		self.thumbnails[direction] = photo
		preview.configure(image=photo, text="")

	def combine_and_save(self) -> None:
		try:
			paths = {}
			for direction in DIRECTIONS:
				path = self.resolve_path(direction)
				if path is None or not path.exists():
					raise ValueError(f"Choose an image for '{direction}'.")
				paths[direction] = path
			sheet = combine_slices(paths)
		except ValueError as error:
			messagebox.showerror("Combine error", str(error))
			return

		output_path = resolve_output_path(self.filename_var.get())
		sheet.save(output_path)
		self.status_label.configure(text=f"Saved {output_path.relative_to(PROJECT_DIR)}")


def main() -> None:
	parser = argparse.ArgumentParser(description="Stack 4 direction frame-strip PNGs into one spritesheet (rows: up, left, down, right).")
	parser.add_argument("--up", type=Path, help="Path to the up-facing frame strip PNG.")
	parser.add_argument("--down", type=Path, help="Path to the down-facing frame strip PNG.")
	parser.add_argument("--left", type=Path, help="Path to the left-facing frame strip PNG.")
	parser.add_argument("--right", type=Path, help="Path to the right-facing frame strip PNG.")
	parser.add_argument("--output", help="Output PNG filename or path (default: output_imgs/combined_spritesheet.png, auto-numbered if it exists).")
	args = parser.parse_args()

	given = {direction: getattr(args, direction) for direction in DIRECTIONS if getattr(args, direction) is not None}

	if not given:
		root = tk.Tk()
		CombineSlicesApp(root)
		root.mainloop()
		return

	if len(given) != len(DIRECTIONS):
		missing = [direction for direction in DIRECTIONS if direction not in given]
		raise SystemExit(f"All 4 directions must be given via CLI, or none (to launch the GUI). Missing: {', '.join(missing)}")

	sheet = combine_slices(given)
	output_path = resolve_output_path(args.output) if args.output else default_output_path()
	sheet.save(output_path)
	print(f"Saved {output_path}")


if __name__ == "__main__":
	main()
