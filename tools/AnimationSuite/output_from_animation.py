"""Render a crab animation config as a horizontal frame strip, styled by a crab config."""

from __future__ import annotations

import argparse
import json
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, ttk
from typing import Any

from PIL import Image, ImageOps, ImageTk

from make_crab_config import (
	CRAB_CONFIG_DIR,
	crab_config_names,
	leg_config_key,
	load_mapped_config,
	load_mapped_image,
	resolve_body_stem,
	resolve_claw_stem,
	resolve_leg_stem,
)


PROJECT_DIR = Path(__file__).resolve().parent
ANIMATION_DIR = PROJECT_DIR / "animation_configs"
OUTPUT_DIR = PROJECT_DIR / "output_imgs"
MAX_ZOOM = 128

LEFT_LEG_ATTACHMENTS = (
	"left_front_leg",
	"left_middle_leg",
	"left_rear_leg",
)
RIGHT_LEG_ATTACHMENTS = (
	"right_front_leg",
	"right_middle_leg",
	"right_rear_leg",
)
LEG_ATTACHMENTS = LEFT_LEG_ATTACHMENTS + RIGHT_LEG_ATTACHMENTS
CLAW_ATTACHMENTS = ("left_claw", "right_claw")


def load_config(config_path: Path) -> dict[str, Any]:
	with config_path.open(encoding="utf-8") as config_file:
		return json.load(config_file)


def load_crab_config(identifier: str) -> dict[str, Any]:
	path = Path(identifier)
	if path.suffix != ".json":
		path = CRAB_CONFIG_DIR / f"{identifier}.json"
	elif not path.is_absolute():
		path = CRAB_CONFIG_DIR / path
	return load_config(path)


def stem_for_attachment(crab_config: dict[str, str], name: str, direction: str) -> str:
	if name in CLAW_ATTACHMENTS:
		return resolve_claw_stem(crab_config[name], direction)
	side, position, _ = name.split("_")
	return resolve_leg_stem(crab_config[leg_config_key(side, position)], direction, position)


def transformed_part(image: Image.Image, config: dict[str, Any], mirror: bool, rotation: float) -> tuple[Image.Image, tuple[int, int]]:
	anchor_x = config["attachment_point_x"]
	anchor_y = config["attachment_point_y"]
	if mirror:
		image = ImageOps.mirror(image)
		anchor_x = image.width - 1 - anchor_x
	radius = max(anchor_x, anchor_y, image.width - 1 - anchor_x, image.height - 1 - anchor_y)
	size = 2 * radius + 1
	padded_part = Image.new("RGBA", (size, size))
	padded_part.alpha_composite(image, (radius - anchor_x, radius - anchor_y))
	rotated_part = padded_part.rotate(rotation, resample=Image.Resampling.NEAREST, fillcolor=(0, 0, 0, 0))
	return rotated_part, (radius, radius)


def paste_at_attachment(
	canvas: Image.Image,
	part: Image.Image,
	part_anchor: tuple[int, int],
	body_anchor: dict[str, int],
	body_origin: tuple[int, int],
) -> None:
	x = body_origin[0] + body_anchor["x"] - part_anchor[0]
	y = body_origin[1] + body_anchor["y"] - part_anchor[1]
	canvas.alpha_composite(part, (x, y))


def centered_image(image: Image.Image, size: int) -> Image.Image:
	bounds = image.getbbox()
	if bounds is None:
		raise ValueError("The composed crab contains no visible pixels.")

	content = image.crop(bounds)
	output = Image.new("RGBA", (size, size))
	x = (size - content.width) // 2
	y = (size - content.height) // 2
	output.alpha_composite(content, (x, y))
	return output


def compose_frame(crab_config: dict[str, str], direction: str, body_anchors: dict[str, dict[str, int]], output_size: int) -> Image.Image:
	body_stem = resolve_body_stem(crab_config["body"], direction)
	body_config = load_mapped_config(body_stem)
	body = load_mapped_image(body_stem)
	if direction == "left":
		body = ImageOps.mirror(body)

	active_names = (
		RIGHT_LEG_ATTACHMENTS + CLAW_ATTACHMENTS if direction == "right"
		else LEFT_LEG_ATTACHMENTS + CLAW_ATTACHMENTS if direction == "left"
		else LEG_ATTACHMENTS + CLAW_ATTACHMENTS
	)
	stems = {name: stem_for_attachment(crab_config, name, direction) for name in active_names}
	part_configs = {name: load_mapped_config(stem) for name, stem in stems.items()}
	part_images = {name: load_mapped_image(stem) for name, stem in stems.items()}

	max_part_size = max(max(image.width, image.height) for image in part_images.values())
	canvas_size = body.width + 2 * max_part_size
	body_origin = (max_part_size, max_part_size)
	canvas = Image.new("RGBA", (canvas_size, canvas_size))

	if direction in ("up", "down"):
		for name in LEG_ATTACHMENTS:
			mirror = name.startswith("left_") if direction == "up" else name.startswith("right_")
			part, anchor = transformed_part(part_images[name], part_configs[name], mirror, float(body_anchors[name].get("rotation", 0.0)))
			paste_at_attachment(canvas, part, anchor, body_anchors[name], body_origin)

		for name in CLAW_ATTACHMENTS:
			part, anchor = transformed_part(part_images[name], part_configs[name], name.startswith("left_"), float(body_anchors[name].get("rotation", 0.0)))
			paste_at_attachment(canvas, part, anchor, body_anchors[name], body_origin)

		canvas.alpha_composite(body, body_origin)

	elif direction == "right":
		# Order: left claw -> body -> right legs -> right claw
		if "left_claw" in part_images and "left_claw" in body_anchors:
			part, anchor = transformed_part(part_images["left_claw"], part_configs["left_claw"], False, float(body_anchors["left_claw"].get("rotation", 0.0)))
			paste_at_attachment(canvas, part, anchor, body_anchors["left_claw"], body_origin)

		canvas.alpha_composite(body, body_origin)

		for name in RIGHT_LEG_ATTACHMENTS:
			if name in part_images and name in body_anchors:
				part, anchor = transformed_part(part_images[name], part_configs[name], False, float(body_anchors[name].get("rotation", 0.0)))
				paste_at_attachment(canvas, part, anchor, body_anchors[name], body_origin)

		if "right_claw" in part_images and "right_claw" in body_anchors:
			part, anchor = transformed_part(part_images["right_claw"], part_configs["right_claw"], False, float(body_anchors["right_claw"].get("rotation", 0.0)))
			paste_at_attachment(canvas, part, anchor, body_anchors["right_claw"], body_origin)

	elif direction == "left":
		# Order: right claw -> body -> left legs -> left claw
		if "right_claw" in part_images and "right_claw" in body_anchors:
			part, anchor = transformed_part(part_images["right_claw"], part_configs["right_claw"], True, float(body_anchors["right_claw"].get("rotation", 0.0)))
			paste_at_attachment(canvas, part, anchor, body_anchors["right_claw"], body_origin)

		canvas.alpha_composite(body, body_origin)

		for name in LEFT_LEG_ATTACHMENTS:
			if name in part_images and name in body_anchors:
				part, anchor = transformed_part(part_images[name], part_configs[name], True, float(body_anchors[name].get("rotation", 0.0)))
				paste_at_attachment(canvas, part, anchor, body_anchors[name], body_origin)

		if "left_claw" in part_images and "left_claw" in body_anchors:
			part, anchor = transformed_part(part_images["left_claw"], part_configs["left_claw"], True, float(body_anchors["left_claw"].get("rotation", 0.0)))
			paste_at_attachment(canvas, part, anchor, body_anchors["left_claw"], body_origin)

	return centered_image(canvas, output_size)


def animation_files() -> list[Path]:
	return sorted(ANIMATION_DIR.glob("*.json"))


def select_animation() -> Path:
	configs = animation_files()
	if not configs:
		raise FileNotFoundError(f"No animation JSON files found in {ANIMATION_DIR}")

	print("Available animations:")
	for index, config_path in enumerate(configs, start=1):
		print(f"  {index}. {config_path.stem}")

	while True:
		choice = input("Choose an animation number: ").strip()
		try:
			return configs[int(choice) - 1]
		except (IndexError, ValueError):
			print(f"Enter a number from 1 to {len(configs)}.")


def compose_animation(animation_path: Path, crab_config: dict[str, str], output_size: int) -> Path:
	if output_size <= 0:
		raise ValueError("Output size must be a positive integer.")

	animation = load_config(animation_path)
	direction = animation.get("direction")
	if direction not in {"up", "down", "right", "left"}:
		raise ValueError("The animation direction must be 'up', 'down', 'right', or 'left'.")
	frames = animation.get("frames", [])
	if not frames:
		raise ValueError("The animation contains no frames.")

	frame_images = [
		compose_frame(crab_config, direction, frame["attachment_points"], output_size)
		for frame in frames
	]
	output = Image.new("RGBA", (output_size * len(frame_images), output_size))
	for index, frame_image in enumerate(frame_images):
		output.alpha_composite(frame_image, (index * output_size, 0))

	OUTPUT_DIR.mkdir(exist_ok=True)
	output_path = OUTPUT_DIR / f"{animation['name']}_{crab_config['name']}_{direction}.png"
	output.save(output_path)
	return output_path


class OutputFromAnimationApp:
	"""Pick an animation and crab config, preview the render, and save it to output_imgs/."""

	def __init__(self, root: tk.Tk, animation_name: str | None, crab_config_name: str | None, size: int) -> None:
		self.root = root
		root.title("Output From Animation")
		root.resizable(True, True)
		root.minsize(500, 400)

		self.animation_names = [path.stem for path in animation_files()]
		self.crab_config_options = crab_config_names()
		self.zoom = 2
		self.preview_image: Image.Image | None = None
		self.photo: ImageTk.PhotoImage | None = None

		self.animation_var = tk.StringVar(value=animation_name if animation_name in self.animation_names else next(iter(self.animation_names), ""))
		self.crab_config_var = tk.StringVar(value=crab_config_name if crab_config_name in self.crab_config_options else next(iter(self.crab_config_options), ""))
		self.size_var = tk.IntVar(value=size)

		top = ttk.Frame(root, padding=12)
		top.pack(fill="x")
		ttk.Label(top, text="Animation:").pack(side="left")
		ttk.Combobox(top, textvariable=self.animation_var, values=self.animation_names, state="readonly", width=18).pack(side="left", padx=(4, 12))
		ttk.Label(top, text="Crab config:").pack(side="left")
		ttk.Combobox(top, textvariable=self.crab_config_var, values=self.crab_config_options, state="readonly", width=18).pack(side="left", padx=(4, 12))
		ttk.Label(top, text="Size:").pack(side="left")
		ttk.Spinbox(top, from_=16, to=1024, increment=16, textvariable=self.size_var, width=6).pack(side="left", padx=(4, 12))
		ttk.Button(top, text="Render", command=self.render).pack(side="left")

		self.status_label = ttk.Label(root, padding=(12, 0, 12, 8))
		self.status_label.pack(anchor="w")
		if not self.animation_names:
			self.status_label.configure(text=f"No animation JSON files found in {ANIMATION_DIR}.")
		elif not self.crab_config_options:
			self.status_label.configure(text=f"No crab config JSON files found in {CRAB_CONFIG_DIR}.")

		self.canvas = tk.Canvas(root, background="#303030", highlightthickness=0)
		self.canvas.pack(fill="both", expand=True, padx=12, pady=(0, 12))
		self.canvas.bind("<Button-3>", self.start_pan)
		self.canvas.bind("<B3-Motion>", self.pan)
		self.canvas.bind("<MouseWheel>", self.change_zoom)
		self.canvas.bind("<Button-4>", self.change_zoom)
		self.canvas.bind("<Button-5>", self.change_zoom)

	def render(self) -> None:
		if not self.animation_var.get() or not self.crab_config_var.get():
			messagebox.showwarning("Nothing to render", "Choose an animation and a crab config first.")
			return
		try:
			animation_path = ANIMATION_DIR / f"{self.animation_var.get()}.json"
			crab_config = load_crab_config(self.crab_config_var.get())
			output_path = compose_animation(animation_path, crab_config, self.size_var.get())
		except (ValueError, FileNotFoundError, KeyError) as error:
			messagebox.showerror("Render error", str(error))
			return

		self.preview_image = Image.open(output_path).convert("RGBA")
		self.redraw_preview()
		self.status_label.configure(text=f"Saved {output_path.relative_to(PROJECT_DIR)}")

	def redraw_preview(self) -> None:
		if self.preview_image is None:
			return
		width, height = self.preview_image.width * self.zoom, self.preview_image.height * self.zoom
		display = self.preview_image.resize((width, height), Image.NEAREST)
		self.photo = ImageTk.PhotoImage(display)
		self.canvas.delete("all")
		self.canvas.configure(scrollregion=(0, 0, width, height))
		self.canvas.create_image(0, 0, anchor="nw", image=self.photo)

	def start_pan(self, event: tk.Event[tk.Misc]) -> None:
		self.canvas.scan_mark(event.x, event.y)

	def pan(self, event: tk.Event[tk.Misc]) -> None:
		self.canvas.scan_dragto(event.x, event.y, gain=1)

	def change_zoom(self, event: tk.Event[tk.Misc]) -> None:
		zoom_in = getattr(event, "delta", 0) > 0 or getattr(event, "num", 0) == 4
		new_zoom = min(MAX_ZOOM, self.zoom + 1) if zoom_in else max(1, self.zoom - 1)
		if new_zoom == self.zoom:
			return
		self.zoom = new_zoom
		self.redraw_preview()


def main() -> None:
	parser = argparse.ArgumentParser(description="Render a crab animation frame strip.")
	parser.add_argument("--size", type=int, default=128, help="Square output size in pixels (default: 128).")
	parser.add_argument("--crab-config", help="Crab config name (from crab_configs/) or path providing the limb styles to render.")
	parser.add_argument("--animation", help="Animation name (from animation_configs/) to render.")
	parser.add_argument("--cli-only", action="store_true", help="Render without a GUI; suitable for automation scripts.")
	args = parser.parse_args()

	if args.cli_only:
		if not args.crab_config:
			raise SystemExit("--crab-config is required when using --cli-only.")
		animation_path = ANIMATION_DIR / f"{args.animation}.json" if args.animation else select_animation()
		if not animation_path.exists():
			raise SystemExit(f"Animation config not found: {animation_path}")
		crab_config = load_crab_config(args.crab_config)
		output_path = compose_animation(animation_path, crab_config, args.size)
		print(f"Saved {output_path.relative_to(PROJECT_DIR)}")
		return

	root = tk.Tk()
	OutputFromAnimationApp(root, args.animation, args.crab_config, args.size)
	root.mainloop()


if __name__ == "__main__":
	main()
