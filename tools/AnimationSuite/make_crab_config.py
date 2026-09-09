"""Tkinter GUI for building crab configs from per-limb style selections."""

from __future__ import annotations

import json
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, simpledialog, ttk
from typing import Any

from PIL import Image, ImageOps, ImageTk

PROJECT_DIR = Path(__file__).resolve().parent
CRAB_CONFIG_DIR = PROJECT_DIR / "crab_configs"
STYLE_DIR = PROJECT_DIR / "style_configs"
MAPPED_DIR = PROJECT_DIR / "mapped_imgs"

DIRECTIONS = ("up", "down", "right")
SIDES = ("left", "right")
LEG_POSITIONS = ("front", "middle", "rear")
MAX_ZOOM = 128
VIEWPORT_SIZE = 220
# mapped_imgs only has front/middle/rear leg shape variants for the right-facing pose
LEG_POSITION_SUFFIX = {"front": "a", "middle": "b", "rear": "c"}
PREVIEW_SIZE = 160


def style_names() -> list[str]:
	return sorted(path.stem for path in STYLE_DIR.glob("*.json"))


def crab_config_names() -> list[str]:
	return sorted(path.stem for path in CRAB_CONFIG_DIR.glob("*.json"))


def leg_config_key(side: str, position: str) -> str:
	return f"{position}_{side}_leg"


def leg_attachment_key(side: str, position: str) -> str:
	return f"{side}_{position}_leg"


def load_style(style_name: str) -> dict[str, str]:
	with (STYLE_DIR / f"{style_name}.json").open(encoding="utf-8") as style_file:
		return json.load(style_file)


def load_mapped_config(stem: str) -> dict[str, Any]:
	with (MAPPED_DIR / f"{stem}.json").open(encoding="utf-8") as mapped_file:
		return json.load(mapped_file)


def load_mapped_image(stem: str) -> Image.Image:
	config = load_mapped_config(stem)
	return Image.open(PROJECT_DIR / config["image_path"]).convert("RGBA")


def resolve_mapped_stem(style_name: str, generic_key: str, fallback_key: str | None = None) -> str:
	style = load_style(style_name)
	if generic_key in style:
		return style[generic_key]
	if fallback_key and fallback_key in style:
		return style[fallback_key]
	if (MAPPED_DIR / f"{generic_key}.json").exists():
		return generic_key
	if fallback_key and (MAPPED_DIR / f"{fallback_key}.json").exists():
		return fallback_key
	raise ValueError(f"Style '{style_name}' has no mapping for '{generic_key}' and no matching default asset exists.")


def resolve_body_stem(style_name: str, direction: str) -> str:
	# there is no dedicated left-facing body asset; "left" is a mirrored "right"
	facing = "right" if direction == "left" else direction
	return resolve_mapped_stem(style_name, f"body_facing_{facing}")


def resolve_leg_stem(style_name: str, direction: str, position: str) -> str:
	if direction in ("right", "left"):
		suffix = LEG_POSITION_SUFFIX[position]
		return resolve_mapped_stem(style_name, f"leg_facing_right_{suffix}", "leg_facing_right")
	return resolve_mapped_stem(style_name, "leg_facing_up")


def resolve_claw_stem(style_name: str, direction: str) -> str:
	claw_key = "claw_facing_right" if direction in ("right", "left") else f"claw_facing_{direction}"
	return resolve_mapped_stem(style_name, claw_key)


def transformed(image: Image.Image, anchor_x: int, anchor_y: int, mirror: bool) -> tuple[Image.Image, int, int]:
	if mirror:
		image = ImageOps.mirror(image)
		anchor_x = image.width - 1 - anchor_x
	return image, anchor_x, anchor_y


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
	output = Image.new("RGBA", (size, size))
	if bounds is None:
		return output
	content = image.crop(bounds)
	x = (size - content.width) // 2
	y = (size - content.height) // 2
	output.alpha_composite(content, (x, y))
	return output


def render_crab(crab_config: dict[str, str], direction: str, size: int = PREVIEW_SIZE) -> Image.Image:
	body_stem = resolve_body_stem(crab_config["body"], direction)
	body_image = load_mapped_image(body_stem)
	attachment_points = load_mapped_config(body_stem)["attachment_points"]

	def leg_image_and_anchor(side: str, position: str) -> tuple[Image.Image, int, int]:
		style = crab_config[leg_config_key(side, position)]
		stem = resolve_leg_stem(style, direction, position)
		config = load_mapped_config(stem)
		image = load_mapped_image(stem)
		return image, config["attachment_point_x"], config["attachment_point_y"]

	def claw_image_and_anchor(side: str) -> tuple[Image.Image, int, int]:
		style = crab_config[f"{side}_claw"]
		stem = resolve_claw_stem(style, direction)
		config = load_mapped_config(stem)
		image = load_mapped_image(stem)
		return image, config["attachment_point_x"], config["attachment_point_y"]

	if direction == "right":
		leg_sides = ("right",)
	else:
		leg_sides = SIDES
	leg_images = [leg_image_and_anchor(side, position)[0] for side in leg_sides for position in LEG_POSITIONS]
	claw_images = [claw_image_and_anchor(side)[0] for side in SIDES]
	max_part_size = max(max(image.size) for image in (*leg_images, *claw_images))
	canvas_size = body_image.width + 2 * max_part_size
	body_origin = (max_part_size, max_part_size)
	canvas = Image.new("RGBA", (canvas_size, canvas_size))

	if direction in ("up", "down"):
		mirror_prefix = "left_" if direction == "up" else "right_"
		for side in SIDES:
			for position in LEG_POSITIONS:
				image, anchor_x, anchor_y = leg_image_and_anchor(side, position)
				attach_key = leg_attachment_key(side, position)
				image, anchor_x, anchor_y = transformed(image, anchor_x, anchor_y, attach_key.startswith(mirror_prefix))
				paste_at_attachment(canvas, image, (anchor_x, anchor_y), attachment_points[attach_key], body_origin)
		for side in SIDES:
			image, anchor_x, anchor_y = claw_image_and_anchor(side)
			claw_key = f"{side}_claw"
			# claws are drawn for the right side; mirror only the left claw, regardless of facing direction
			image, anchor_x, anchor_y = transformed(image, anchor_x, anchor_y, side == "left")
			paste_at_attachment(canvas, image, (anchor_x, anchor_y), attachment_points[claw_key], body_origin)
		canvas.alpha_composite(body_image, body_origin)
	else:
		image, anchor_x, anchor_y = claw_image_and_anchor("left")
		paste_at_attachment(canvas, image, (anchor_x, anchor_y), attachment_points["left_claw"], body_origin)

		canvas.alpha_composite(body_image, body_origin)

		for position in LEG_POSITIONS:
			image, anchor_x, anchor_y = leg_image_and_anchor("right", position)
			attach_key = leg_attachment_key("right", position)
			paste_at_attachment(canvas, image, (anchor_x, anchor_y), attachment_points[attach_key], body_origin)

		image, anchor_x, anchor_y = claw_image_and_anchor("right")
		paste_at_attachment(canvas, image, (anchor_x, anchor_y), attachment_points["right_claw"], body_origin)

	return centered_image(canvas, size)


class CrabConfigBuilder:
	"""Assemble a crab config by picking a style for each limb and the body."""

	def __init__(self, root: tk.Tk) -> None:
		self.root = root
		root.title("Crab Config Builder")
		root.resizable(True, True)

		styles = style_names()
		if not styles:
			raise RuntimeError(f"No style configs found in {STYLE_DIR}.")
		self.default_style = styles[0]

		self.name_var = tk.StringVar(value="new_crab")
		self.body_var = tk.StringVar(value=self.default_style)
		self.limb_vars: dict[str, tk.StringVar] = {
			"left_claw": tk.StringVar(value=self.default_style),
			"right_claw": tk.StringVar(value=self.default_style),
		}
		for side in SIDES:
			for position in LEG_POSITIONS:
				self.limb_vars[leg_config_key(side, position)] = tk.StringVar(value=self.default_style)

		top = ttk.Frame(root, padding=12)
		top.pack(fill="x")
		ttk.Label(top, text="Name:").pack(side="left")
		ttk.Entry(top, textvariable=self.name_var, width=20).pack(side="left", padx=(4, 12))
		ttk.Button(top, text="New", command=self.new_config).pack(side="left", padx=(0, 4))
		ttk.Button(top, text="Save", command=self.save_config).pack(side="left", padx=(0, 12))
		ttk.Label(top, text="Load:").pack(side="left")
		self.load_var = tk.StringVar()
		self.load_selector = ttk.Combobox(
			top, textvariable=self.load_var, values=crab_config_names(), state="readonly", width=20
		)
		self.load_selector.pack(side="left", padx=(4, 0))
		self.load_selector.bind("<<ComboboxSelected>>", self.load_config)

		previews = ttk.Frame(root, padding=(12, 0, 12, 12))
		previews.pack(fill="both", expand=True)
		for index in range(len(DIRECTIONS)):
			previews.columnconfigure(index, weight=1)
		previews.rowconfigure(1, weight=1)
		self.preview_canvases: dict[str, tk.Canvas] = {}
		self.preview_photos: dict[str, ImageTk.PhotoImage] = {}
		self.preview_images: dict[str, Image.Image] = {}
		self.preview_zoom: dict[str, int] = {}
		for index, direction in enumerate(DIRECTIONS):
			ttk.Label(previews, text=direction.title(), font=("TkDefaultFont", 11, "bold")).grid(row=0, column=index)
			canvas = tk.Canvas(previews, width=VIEWPORT_SIZE, height=VIEWPORT_SIZE, background="#303030", highlightthickness=0)
			canvas.grid(row=1, column=index, sticky="nsew", padx=8)
			canvas.bind("<Button-3>", self.start_pan)
			canvas.bind("<B3-Motion>", self.pan)
			canvas.bind("<MouseWheel>", lambda event, direction=direction: self.change_zoom(event, direction))
			canvas.bind("<Button-4>", lambda event, direction=direction: self.change_zoom(event, direction))
			canvas.bind("<Button-5>", lambda event, direction=direction: self.change_zoom(event, direction))
			self.preview_canvases[direction] = canvas
			self.preview_zoom[direction] = 2

		body_frame = ttk.LabelFrame(root, text="Body", padding=8)
		body_frame.pack(fill="x", padx=12, pady=(0, 8))
		self.add_selector(body_frame, "Body", self.body_var)

		columns = ttk.Frame(root, padding=(12, 0, 12, 12))
		columns.pack(fill="x")
		side_labels = {"left": "Left Side", "right": "Right Side"}
		for side in SIDES:
			column_frame = ttk.LabelFrame(columns, text=side_labels[side], padding=8)
			column_frame.pack(side="left", fill="both", expand=True, padx=8)
			self.add_selector(column_frame, "Claw", self.limb_vars[f"{side}_claw"])
			for position in LEG_POSITIONS:
				self.add_selector(column_frame, f"{position.title()} Leg", self.limb_vars[leg_config_key(side, position)])

		self.update_previews()

	def add_selector(self, parent: ttk.Frame, label: str, variable: tk.StringVar) -> None:
		row = ttk.Frame(parent)
		row.pack(fill="x", pady=2)
		ttk.Label(row, text=label, width=12).pack(side="left")
		selector = ttk.Combobox(row, textvariable=variable, values=style_names(), state="readonly", width=14)
		selector.pack(side="left", fill="x", expand=True)
		selector.bind("<<ComboboxSelected>>", lambda event: self.update_previews())

	def current_config(self) -> dict[str, str]:
		config = {"name": self.name_var.get(), "body": self.body_var.get()}
		for key, variable in self.limb_vars.items():
			config[key] = variable.get()
		return config

	def update_previews(self) -> None:
		config = self.current_config()
		for direction in DIRECTIONS:
			try:
				image = render_crab(config, direction)
			except (ValueError, FileNotFoundError) as error:
				messagebox.showerror("Render error", str(error))
				return
			self.preview_images[direction] = image
			self.redraw_preview(direction)

	def redraw_preview(self, direction: str) -> None:
		image = self.preview_images[direction]
		zoom = self.preview_zoom[direction]
		width, height = image.width * zoom, image.height * zoom
		display = image.resize((width, height), Image.NEAREST)
		photo = ImageTk.PhotoImage(display)
		canvas = self.preview_canvases[direction]
		canvas.delete("all")
		canvas.configure(scrollregion=(0, 0, width, height))
		canvas.create_image(0, 0, anchor="nw", image=photo)
		self.preview_photos[direction] = photo  # keep a reference so Tk doesn't garbage-collect it

	def start_pan(self, event: tk.Event[tk.Misc]) -> None:
		event.widget.scan_mark(event.x, event.y)

	def pan(self, event: tk.Event[tk.Misc]) -> None:
		event.widget.scan_dragto(event.x, event.y, gain=1)

	def change_zoom(self, event: tk.Event[tk.Misc], direction: str) -> None:
		zoom_in = getattr(event, "delta", 0) > 0 or getattr(event, "num", 0) == 4
		zoom = self.preview_zoom[direction]
		new_zoom = min(MAX_ZOOM, zoom + 1) if zoom_in else max(1, zoom - 1)
		if new_zoom == zoom:
			return

		canvas = self.preview_canvases[direction]
		image = self.preview_images[direction]
		mouse_x = canvas.canvasx(event.x)
		mouse_y = canvas.canvasy(event.y)
		image_x = mouse_x / zoom
		image_y = mouse_y / zoom
		self.preview_zoom[direction] = new_zoom
		self.redraw_preview(direction)
		canvas.xview_moveto(max(0, (image_x * new_zoom - event.x) / (image.width * new_zoom)))
		canvas.yview_moveto(max(0, (image_y * new_zoom - event.y) / (image.height * new_zoom)))

	def new_config(self) -> None:
		self.name_var.set("new_crab")
		self.body_var.set(self.default_style)
		for variable in self.limb_vars.values():
			variable.set(self.default_style)
		self.update_previews()

	def save_config(self) -> None:
		name = self.name_var.get().strip()
		if not name:
			messagebox.showwarning("Missing name", "Enter a name before saving.")
			return
		CRAB_CONFIG_DIR.mkdir(exist_ok=True)
		config_path = CRAB_CONFIG_DIR / f"{name}.json"
		with config_path.open("w", encoding="utf-8") as config_file:
			json.dump(self.current_config(), config_file, indent=2)
			config_file.write("\n")
		self.load_selector.configure(values=crab_config_names())
		messagebox.showinfo("Saved", f"Saved {config_path.relative_to(PROJECT_DIR)}")

	def load_config(self, event: tk.Event[tk.Misc] | None = None) -> None:
		name = self.load_var.get()
		if not name:
			return
		config_path = CRAB_CONFIG_DIR / f"{name}.json"
		with config_path.open(encoding="utf-8") as config_file:
			config = json.load(config_file)
		self.name_var.set(config.get("name", name))
		self.body_var.set(config.get("body", self.default_style))
		for key, variable in self.limb_vars.items():
			variable.set(config.get(key, self.default_style))
		self.update_previews()


def main() -> None:
	root = tk.Tk()
	CrabConfigBuilder(root)
	root.mainloop()


if __name__ == "__main__":
	main()
